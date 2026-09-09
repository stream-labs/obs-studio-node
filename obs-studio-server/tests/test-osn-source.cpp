#include <catch2/catch_test_macros.hpp>
#include "nodeobs_api.h"
#include "osn-error.hpp"
#include "osn-input.hpp"
#include "osn-scene.hpp"
#include "osn-source.hpp"
#include <obs.h>
#include "shared.hpp"
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace {

// Keep the test source independent of OBS plugins so its behavior depends only
// on the source manager and libobs reference lifecycle.
constexpr char TEST_SOURCE_ID[] = "source_manager_lifetime_test_source";

const char *testSourceGetName(void *)
{
	return "Source Manager Lifetime Test Source";
}

void *testSourceCreate(obs_data_t *, obs_source_t *source)
{
	return source;
}

void testSourceDestroy(void *) {}

obs_properties_t *testSourceGetProperties(void *)
{
	obs_properties_t *properties = obs_properties_create();
	obs_properties_add_bool(properties, "enabled", "Enabled");
	return properties;
}

obs_source_info makeTestSourceInfo()
{
	obs_source_info info{};
	info.id = TEST_SOURCE_ID;
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.get_name = testSourceGetName;
	info.create = testSourceCreate;
	info.destroy = testSourceDestroy;
	info.get_properties = testSourceGetProperties;
	return info;
}

// These tests only need the OBS core and its deferred-destruction queue.
class ObsCoreSetup {
public:
	ObsCoreSetup() { REQUIRE(obs_startup("en-US", nullptr, nullptr)); }
	~ObsCoreSetup()
	{
		// Do not let a queued source destruction outlive the OBS core.
		obs_wait_for_destroy_queue();
		obs_shutdown();
	}

	ObsCoreSetup(const ObsCoreSetup &) = delete;
	ObsCoreSetup &operator=(const ObsCoreSetup &) = delete;
};

// Source destruction runs on OBS_TASK_DESTROY. Blocking that queue creates the
// precise window where the last strong reference is gone but the manager's
// destroy callback has not removed the source registration yet.
class DestroyQueueGate {
public:
	DestroyQueueGate()
	{
		obs_queue_task(OBS_TASK_DESTROY, waitForRelease, this, false);
		// Wait until the gate is running so later destruction is guaranteed to
		// queue behind it.
		std::unique_lock lock(mutex);
		condition.wait(lock, [this] { return entered; });
	}

	~DestroyQueueGate() { releaseAndWait(); }

	void releaseAndWait()
	{
		{
			std::lock_guard lock(mutex);
			released = true;
		}
		condition.notify_all();

		if (!drained) {
			// Draining also runs the source's destroy signal, which unregisters
			// it from Source::Manager.
			obs_wait_for_destroy_queue();
			drained = true;
		}
	}

	DestroyQueueGate(const DestroyQueueGate &) = delete;
	DestroyQueueGate &operator=(const DestroyQueueGate &) = delete;

private:
	static void waitForRelease(void *data)
	{
		auto *gate = static_cast<DestroyQueueGate *>(data);
		std::unique_lock lock(gate->mutex);
		gate->entered = true;
		gate->condition.notify_all();
		gate->condition.wait(lock, [gate] { return gate->released; });
	}

	std::mutex mutex;
	std::condition_variable condition;
	bool entered = false;
	bool released = false;
	bool drained = false;
};

} // namespace

TEST_CASE("Scene AddSource rejects malformed argument counts")
{
	for (const std::size_t argumentCount : {std::size_t{0}, std::size_t{1}, std::size_t{3}, std::size_t{19}, std::size_t{21}}) {
		std::vector<ipc::value> args;
		for (std::size_t i = 0; i < argumentCount; i++)
			args.emplace_back(uint64_t{0});

		std::vector<ipc::value> response;
		osn::Scene::AddSource(nullptr, 0, args, response);

		REQUIRE(response.size() >= 2);
		CHECK((ErrorCode)response[0].value_union.ui64 == ErrorCode::Error);
		CHECK(response[1].value_str == "Invalid number of arguments to add a source to a scene.");
	}
}

TEST_CASE("Source manager safely promotes references during deferred destruction")
{
	ObsCoreSetup setupOBS;
	static const obs_source_info testSourceInfo = makeTestSourceInfo();
	obs_register_source(&testSourceInfo);
	auto &manager = osn::Source::Manager::GetInstance();
	const auto sourceCount = manager.size();

	{
		INFO("The final source release wins the race");
		DestroyQueueGate destroyQueue;
		OBSSourceAutoRelease source = obs_source_create_private(TEST_SOURCE_ID, "released source", nullptr);
		REQUIRE(source != nullptr);

		// Private sources do not emit the global source-create signal, so mirror
		// production registration here. Repeated allocation must keep one ID and
		// one retained weak reference for the source.
		const uint64_t sourceId = manager.allocate(source);
		osn::Source::attach_source_signals(source);
		REQUIRE(sourceId != UINT64_MAX);
		CHECK(manager.allocate(source) == sourceId);
		CHECK(manager.size() == sourceCount + 1);

		// The gate keeps deferred destruction from unregistering the source.
		// Dropping the only strong reference therefore leaves an expired source
		// ID in the manager, reproducing the original race window.
		source = nullptr;

		CHECK(manager.size() == sourceCount + 1);
		std::vector<ipc::value> args = {ipc::value(sourceId)};
		std::vector<ipc::value> response;
		// Promotion of the retained weak reference must fail cleanly instead of
		// dereferencing the stale raw pointer stored by the old implementation.
		osn::Source::GetProperties(nullptr, 0, args, response);
		REQUIRE(!response.empty());
		CHECK((ErrorCode)response[0].value_union.ui64 == ErrorCode::InvalidReference);

		// Let OBS finish destruction and deliver the manager's destroy callback.
		destroyQueue.releaseAndWait();
		CHECK(manager.size() == sourceCount);
	}

	{
		INFO("The source lookup wins the race");
		DestroyQueueGate destroyQueue;
		OBSSourceAutoRelease source = obs_source_create_private(TEST_SOURCE_ID, "retained source", nullptr);
		REQUIRE(source != nullptr);

		const uint64_t sourceId = manager.allocate(source);
		osn::Source::attach_source_signals(source);
		REQUIRE(sourceId != UINT64_MAX);
		// This time lookup wins: promotion happens while the original strong
		// reference is still alive and must keep the source usable.
		OBSSourceAutoRelease retainedSource = manager.findAndRef(sourceId);
		REQUIRE(retainedSource != nullptr);

		source = nullptr;
		CHECK(std::string(obs_source_get_name(retainedSource)) == "retained source");
		OBSSourceAutoRelease secondReference = manager.findAndRef(sourceId);
		CHECK(secondReference != nullptr);

		// Releasing every promoted reference expires the source. Its registration
		// remains until the blocked destroy callback runs, but another promotion
		// must already report that the source is gone.
		secondReference = nullptr;
		retainedSource = nullptr;
		CHECK(manager.size() == sourceCount + 1);
		CHECK(!manager.findAndRef(sourceId));

		// Draining the queue completes destruction and removes the registration.
		destroyQueue.releaseAndWait();
		CHECK(manager.size() == sourceCount);
	}
}
