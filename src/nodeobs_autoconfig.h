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
#include <v8.h>


void GetListServer(const v8::FunctionCallbackInfo<v8::Value>& args);
void InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
void TerminateAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
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