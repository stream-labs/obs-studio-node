/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include <iostream>
#include <string>
#pragma once
#include <graphics/math-extra.h>
#include <mutex>
#include <obs.hpp>
#include <queue>
#include <thread>
#include "obs-nodeobs_api.h"
#include "obs-nodeobs_service.h"

class AutoConfigInfo
{
	public:
	AutoConfigInfo(
		std::string event, std::string description, double percentage) {
		this->m_event = event;
		this->m_description = description;
		this->m_percentage = percentage;
	};
	~AutoConfigInfo() {};

	std::string m_event;
	std::string m_description;
	double      m_percentage;
};

typedef void (*callbackAutoConfig)(AutoConfigInfo* data, void* jsThread);

extern void* g_jsAutoConfigThread;
extern callbackAutoConfig g_callback;

namespace obs
{

	namespace autoConfig
	{
		void InitializeAutoConfig(callbackAutoConfig callback, void* jsThread);
		void StartBandwidthTest();
		void StartStreamEncoderTest();
		void StartRecordingEncoderTest();
		void StartCheckSettings();
		void StartSetDefaultSettings();
		void StartSaveStreamSettings();
		void StartSaveSettings();

		void StopThread();
		void FindIdealHardwareResolution();
		bool TestSoftwareEncoding();
		void TestBandwidthThread();
		void TestStreamEncoderThread();
		void TestRecordingEncoderThread();
		void SaveStreamSettings();
		void SaveSettings();
		bool CheckSettings();
		void SetDefaultSettings();
		void TestHardwareEncoding();
		bool CanTestServer(const char* server);
		void WaitPendingTests(double timeout = 10);
	}
}