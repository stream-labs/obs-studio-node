#pragma once

#include <obs.h>
#include <atomic>
#include <functional>

class obs_global_ref_counter
{
	protected:

	void increment_ref_counter()
	{
		m_TotalConstructed++;
	}

	void decrement_ref_counter()
	{
		m_TotalDestructed++;
	}

	public:

	__declspec(noinline) static int get_total_allocations()
	{
		return m_TotalConstructed;
	}

	__declspec(noinline) static int get_total_deallocations()
	{
		return m_TotalDestructed;
	}

	__declspec(noinline) static int get_total_leaks()
	{
		return m_TotalConstructed - m_TotalDestructed;
	}

	private:

	static std::atomic<int> m_TotalConstructed;
	static std::atomic<int> m_TotalDestructed;
};

template<typename ObjectType>
class obs_wrapper : protected obs_global_ref_counter
{
	public:
	obs_wrapper()
	{
		m_Object      = nullptr;
		m_RefIncrease = nullptr;
		m_RefDecrease = nullptr;
	}

	// Expects that the object will start with its reference count equal to 1
	obs_wrapper(
	    ObjectType*                      obj,
	    std::function<void(ObjectType*)> refIncrease,
	    std::function<void(ObjectType*)> refDecrease)
	{
		m_Object      = obj;
		m_RefIncrease = refIncrease;
		m_RefDecrease = refDecrease;
		if (is_valid()) {
			increment_ref_counter();
		}
	}

	// Copy constructor
	obs_wrapper(const obs_wrapper& other)
	{
		m_Object      = other.m_Object;
		m_RefIncrease = other.m_RefIncrease;
		m_RefDecrease = other.m_RefDecrease;
		if (m_RefIncrease) {
			m_RefIncrease(m_Object);
		}
		if (is_valid()) {
			increment_ref_counter();
		}
	}
	
	// Move constructor
	obs_wrapper<ObjectType> operator=(obs_wrapper<ObjectType>&& other)
	{
		reset();

		m_Object            = std::move(other.m_Object);
		m_RefIncrease       = std::move(other.m_RefIncrease);
		m_RefDecrease       = std::move(other.m_RefDecrease);
		other.m_Object      = nullptr;
		other.m_RefIncrease = nullptr;
		other.m_RefDecrease = nullptr;

		return *this;
	}

	// Assignment op
	__declspec(noinline) obs_wrapper<ObjectType>& operator=(const obs_wrapper<ObjectType>& other)
	{
		if (other.m_RefIncrease) {
			other.m_RefIncrease(other.m_Object);
		}
		if (other.is_valid()) {
			increment_ref_counter();
		}

		reset();

		m_Object      = other.m_Object;
		m_RefIncrease = other.m_RefIncrease;
		m_RefDecrease = other.m_RefDecrease;

		return *this;
	}

	__declspec(noinline) friend bool operator==(const obs_wrapper<ObjectType>& c1, const obs_wrapper<ObjectType>& c2)
	{
		return (c1.m_Object == c2.m_Object);
	}

	__declspec(noinline) friend bool operator!=(const obs_wrapper<ObjectType>& c1, const obs_wrapper<ObjectType>& c2)
	{
		return !(c1 == c2);
	}

	operator bool() const
	{
		return m_Object != nullptr;
	}

	~obs_wrapper()
	{
		if (is_valid() && !obs_initialized()) {
			// Setup leak info for sentry here (if avaliable!)
		}

		reset();
	}

	void reset()
	{
		if (is_valid()) {
			decrement_ref_counter();
		}
		if (m_RefDecrease)
			m_RefDecrease(m_Object);
		m_Object      = nullptr;
		m_RefIncrease = nullptr;
		m_RefDecrease = nullptr;
	}

	ObjectType* get()
	{
		assert(m_Object != nullptr);
		return m_Object;
	}

	bool is_valid() const
	{
		return m_Object != nullptr;
	}

	private:
	ObjectType*                      m_Object;
	std::function<void(ObjectType*)> m_RefIncrease;
	std::function<void(ObjectType*)> m_RefDecrease;
};

template<typename ObjectType>
static obs_wrapper<ObjectType> make_wrapper(
    ObjectType*                      obj,
    std::function<void(ObjectType*)> refIncrease,
    std::function<void(ObjectType*)> refDecrease)
{
	return std::move(obs_wrapper<ObjectType>(obj, refIncrease, refDecrease));
}

////////////////////////////
// WRAPPER IMPLEMENTATION //
////////////////////////////

namespace Wrapper
{
	class Encoder
	{
		public:
		enum class Type
		{
			Unknow,
			Video,
			Audio
		};

		static Encoder
		    Create(const char* id, const char* name, obs_data_t* settings, size_t mixer_idx, obs_data_t* hotkey_data);

		void Reset();
		void SetScaledSize(int cx, int cy);
		bool SetVideo(video_t* video);
		bool SetAudio(audio_t* audio);
		bool SetVideoEncoder(obs_output_t* output);
		bool SetAudioEncoder(obs_output_t* output);
		void Update(obs_data_t* data);

		private:
		// Member variables
		// ...
	};

}; // namespace Wrapper