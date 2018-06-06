#pragma once
#include <iostream>
#include <string>
#pragma once
#include <thread>
#include <mutex>
#include <obs.hpp>
#include <graphics/math-extra.h>
#include "nodeobs_api.h"
#include "nodeobs_service.h"

namespace autoConfig {
	void Register(ipc::server& srv);
	void GetListServer(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void InitializeAutoConfig(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartBandwidthTest(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartStreamEncoderTest(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartRecordingEncoderTest(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartCheckSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartSetDefaultSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartSaveStreamSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StartSaveSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void TerminateAutoConfig(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	void StopThread();
	void FindIdealHardwareResolution();
	bool TestSoftwareEncoding();
	void TestBandwidthThread();
	void TestStreamEncoderThread();
	void TestRecordingEncoderThread();
	void SaveStreamSettings();
	void SaveSettings();
	void CheckSettings();
	void SetDefaultSettings();
	void TestHardwareEncoding();
	bool CanTestServer(const char *server);
}