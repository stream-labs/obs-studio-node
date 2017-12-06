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
#include "nodeobs_common.h"
#include <v8.h>

struct ServerInfo {
	std::string name;
	std::string address;
	int bitrate = 0;
	int ms = -1;

	inline ServerInfo() {}

	inline ServerInfo(const char *name_, const char *address_)
		: name(name_), address(address_) {}
};

class TestMode {
	obs_video_info ovi;
	OBSSource source[6];

	static void render_rand(void *, uint32_t cx, uint32_t cy)
	{
		gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t *randomvals[3] = {
			gs_effect_get_param_by_name(solid, "randomvals1"),
			gs_effect_get_param_by_name(solid, "randomvals2"),
			gs_effect_get_param_by_name(solid, "randomvals3")
		};

		struct vec4 r;

		for (int i = 0; i < 3; i++) {
			vec4_set(&r,
				rand_float(true) * 100.0f,
				rand_float(true) * 100.0f,
				rand_float(true) * 50000.0f + 10000.0f,
				0.0f);
			gs_effect_set_vec4(randomvals[i], &r);
		}

		while (gs_effect_loop(solid, "Random"))
			gs_draw_sprite(nullptr, 0, cx, cy);
	}

public:
	inline TestMode()
	{
		obs_get_video_info(&ovi);
		obs_add_main_render_callback(render_rand, this);

		for (uint32_t i = 0; i < 6; i++) {
			source[i] = obs_get_output_source(i);
			obs_source_release(source[i]);
			obs_set_output_source(i, nullptr);
		}
	}

	inline ~TestMode()
	{
		for (uint32_t i = 0; i < 6; i++)
			obs_set_output_source(i, source[i]);

		obs_remove_main_render_callback(render_rand, this);
		obs_reset_video(&ovi);
	}

	inline void SetVideo(int cx, int cy, int fps_num, int fps_den)
	{
		obs_video_info newOVI = ovi;

		newOVI.output_width = (uint32_t)cx;
		newOVI.output_height = (uint32_t)cy;
		newOVI.fps_num = (uint32_t)fps_num;
		newOVI.fps_den = (uint32_t)fps_den;

		obs_reset_video(&newOVI);
	}
};

enum class Type {
	Invalid,
	Streaming,
	Recording
};

enum class Service {
	Twitch,
	Hitbox,
	Beam,
	Other
};

enum class Encoder {
	x264,
	NVENC,
	QSV,
	AMD,
	Stream
};

enum class Quality {
	Stream,
	High
};

enum class FPSType : int {
	PreferHighFPS,
	PreferHighRes,
	UseCurrent,
	fps30,
	fps60
};

struct Event {
    obs::CallbackInfo *cb_info;

	std::string event;
	std::string description;
	int percentage;
};
		
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


void StartThread(obs::CallbackInfo* cb);
void StopThread(void); 

void FindIdealHardwareResolution(void);
bool TestSoftwareEncoding(void);

void TestBandwidthThread(void);
void TestStreamEncoderThread(void);
void TestRecordingEncoderThread(void);
void SaveStreamSettings(void);
void SaveSettings(void);
void CheckSettings(void);
void SetDefaultSettings(void);

void TestHardwareEncoding(void);
bool CanTestServer(const char *server);

void start_next_step(void (*task)(), std::string event, std::string description, int percentage);

void GetServers(std::vector<ServerInfo> &servers);