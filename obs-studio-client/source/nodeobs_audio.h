#pragma once

#include <v8.h>

/* Fader API */
void OBS_audio_createFader(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_destroyFader(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderSetDb(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderGetDb(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderSetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderGetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderSetMul(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderGetMul(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_faderRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

/* Volmeter API */
void OBS_audio_createVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_destroyVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterSetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterGetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterSetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterGetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
void OBS_audio_volmeterRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);