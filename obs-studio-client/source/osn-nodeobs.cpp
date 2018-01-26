// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "osn-NodeOBS.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "controller.hpp"

#ifdef _WIN32
#include <direct.h>
#endif

// Initializer
INITIALIZER(osn_nodeobs) {
	// Add initializer function
	initializerFunctions.push([](v8::Local<v8::Object>& exports) {
		// Find current directory.
		{
			std::vector<char> pathCWD(65535); // Should use MAX_PATH here
			char *answer = _getcwd(pathCWD.data(), pathCWD.size() - 1);
			g_moduleDirectory = std::string(pathCWD.data()) + "/node_modules/obs-studio-node/distribute";
			replaceAll(g_moduleDirectory, "\\", "/");
		}

		// EDIT: Add function to specify actual load directory.
		NODE_SET_METHOD(exports, "SetWorkingDirectory", NodeOBS::Module::SetWorkingDirectory);
		// END OF EDIT:

		// API
		NODE_SET_METHOD(exports, "OBS_API_initAPI", NodeOBS::API::InitAPI);
		NODE_SET_METHOD(exports, "OBS_API_initOBS_API", NodeOBS::API::InitOBSAPI);
		NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", NodeOBS::API::DestroyOBSAPI);
		NODE_SET_METHOD(exports, "OBS_API_openAllModules", NodeOBS::API::OpenAllModules);
		NODE_SET_METHOD(exports, "OBS_API_initAllModules", NodeOBS::API::InitAllModules);
		NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", NodeOBS::API::GetPerformanceStatistics);
		NODE_SET_METHOD(exports, "OBS_API_getPathConfigDirectory", NodeOBS::API::GetPathConfigDirectory);
		NODE_SET_METHOD(exports, "OBS_API_setPathConfigDirectory", NodeOBS::API::SetPathConfigDirectory);
		//NODE_SET_METHOD(exports, "OBS_API_getOBS_existingProfiles", NodeOBS::API::GetExistingOBSProfiles);
		//NODE_SET_METHOD(exports, "OBS_API_getOBS_existingSceneCollections", NodeOBS::API::GetExistingOBSSceneCollections);
		//NODE_SET_METHOD(exports, "OBS_API_getOBS_currentProfile", NodeOBS::API::GetCurrentOBSProfile);
		//NODE_SET_METHOD(exports, "OBS_API_setOBS_currentProfile", NodeOBS::API::SetCurrentOBSProfile);
		//NODE_SET_METHOD(exports, "OBS_API_getOBS_currentSceneCollection", NodeOBS::API::GetCurrentOBSSceneCollection);
		//NODE_SET_METHOD(exports, "OBS_API_setOBS_currentSceneCollection", NodeOBS::API::SetCurrentOBSSceneCollection);
		//NODE_SET_METHOD(exports, "OBS_API_isOBS_installed", NodeOBS::API::IsOBSInstalled);
		//NODE_SET_METHOD(exports, "OBS_API_useOBS_config", NodeOBS::API::UseOBSConfiguration);

		// Audio
		//NODE_SET_METHOD(exports, "OBS_audio_createFader", NodeOBS::Audio::CreateFader);
		//NODE_SET_METHOD(exports, "OBS_audio_destroyFader", NodeOBS::Audio::DestroyFader);
		//NODE_SET_METHOD(exports, "OBS_audio_faderAddCallback", NodeOBS::Audio::FaderAddCallback);
		//NODE_SET_METHOD(exports, "OBS_audio_faderRemoveCallback", NodeOBS::Audio::FaderRemoveCallback);
		//NODE_SET_METHOD(exports, "OBS_audio_faderSetDb", NodeOBS::Audio::FaderSetDb);
		//NODE_SET_METHOD(exports, "OBS_audio_faderGetDb", NodeOBS::Audio::FaderGetDb);
		//NODE_SET_METHOD(exports, "OBS_audio_faderSetDeflection", NodeOBS::Audio::FaderSetDeflection);
		//NODE_SET_METHOD(exports, "OBS_audio_faderGetDeflection", NodeOBS::Audio::FaderGetDeflection);
		//NODE_SET_METHOD(exports, "OBS_audio_faderSetMul", NodeOBS::Audio::FaderSetMul);
		//NODE_SET_METHOD(exports, "OBS_audio_faderGetMul", NodeOBS::Audio::FaderGetMul);
		//NODE_SET_METHOD(exports, "OBS_audio_faderAttachSource", NodeOBS::Audio::FaderAttachSource);
		//NODE_SET_METHOD(exports, "OBS_audio_faderDetachSource", NodeOBS::Audio::FaderDetachSource);

		//NODE_SET_METHOD(exports, "OBS_audio_createVolmeter", NodeOBS::Audio::CreateVolMeter);
		//NODE_SET_METHOD(exports, "OBS_audio_destroyVolmeter", NodeOBS::Audio::DestroyVolMeter);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterAttachSource", NodeOBS::Audio::VolMeterAttachSource);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterDetachSource", NodeOBS::Audio::VolMeterDetachSource);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterSetUpdateInterval", NodeOBS::Audio::VolMeterSetUpdateInterval);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterGetUpdateInterval", NodeOBS::Audio::VolMeterGetUpdateInterval);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterSetPeakHold", NodeOBS::Audio::VolMeterSetPeakHold);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterGetPeakHold", NodeOBS::Audio::VolMeterGetPeakHold);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterAddCallback", NodeOBS::Audio::VolMeterAddCallback);
		//NODE_SET_METHOD(exports, "OBS_audio_volmeterRemoveCallback", NodeOBS::Audio::VolMeterRemoveCallback);

		// AutoConfig
		NODE_SET_METHOD(exports, "GetListServer", NodeOBS::AutoConfig::GetListServer);
		NODE_SET_METHOD(exports, "InitializeAutoConfig", NodeOBS::AutoConfig::InitializeAutoConfig);
		NODE_SET_METHOD(exports, "StartBandwidthTest", NodeOBS::AutoConfig::StartBandwidthTest);
		NODE_SET_METHOD(exports, "StartStreamEncoderTest", NodeOBS::AutoConfig::StartStreamEncoderTest);
		NODE_SET_METHOD(exports, "StartRecordingEncoderTest", NodeOBS::AutoConfig::StartRecordingEncoderTest);
		NODE_SET_METHOD(exports, "StartCheckSettings", NodeOBS::AutoConfig::StartCheckSettings);
		NODE_SET_METHOD(exports, "StartSetDefaultSettings", NodeOBS::AutoConfig::StartSetDefaultSettings);
		NODE_SET_METHOD(exports, "StartSaveStreamSettings", NodeOBS::AutoConfig::StartSaveStreamSettings);
		NODE_SET_METHOD(exports, "StartSaveSettings", NodeOBS::AutoConfig::StartSaveSettings);
		NODE_SET_METHOD(exports, "TerminateAutoConfig", NodeOBS::AutoConfig::TerminateAutoConfig);

		// Content
		NODE_SET_METHOD(exports, "OBS_content_getSourceFilterVisibility", NodeOBS::Content::GetSourceFilterVisibility);
		NODE_SET_METHOD(exports, "OBS_content_setSourceFilterVisibility", NodeOBS::Content::SetSourceFilterVisibility);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFader", NodeOBS::Content::GetSourceFader);
		NODE_SET_METHOD(exports, "OBS_content_getSourceVolmeter", NodeOBS::Content::GetSourceVolmeter);
		NODE_SET_METHOD(exports, "OBS_content_flipHorzSceneItems", NodeOBS::Content::FlipHorizontalSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_flipVertSceneItems", NodeOBS::Content::FlipVerticalSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_resetSceneItems", NodeOBS::Content::ResetSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_stretchSceneItems", NodeOBS::Content::StretchSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_fitSceneItems", NodeOBS::Content::FitSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_centerSceneItems", NodeOBS::Content::CenterSceneItems);
		NODE_SET_METHOD(exports, "OBS_content_getSceneItemRot", NodeOBS::Content::GetSceneItemRotation);
		NODE_SET_METHOD(exports, "OBS_content_getSceneItemCrop", NodeOBS::Content::GetSceneItemCrop);
		NODE_SET_METHOD(exports, "OBS_content_setSceneItemRot", NodeOBS::Content::SetSceneItemRotation);
		NODE_SET_METHOD(exports, "OBS_content_setSceneItemCrop", NodeOBS::Content::SetSceneItemCrop);
		NODE_SET_METHOD(exports, "OBS_content_getListCurrentScenes", NodeOBS::Content::GetListCurrentScenes);
		NODE_SET_METHOD(exports, "OBS_content_getListCurrentSourcesFromScene", NodeOBS::Content::GetListCurrentSourcesFromScene);
		NODE_SET_METHOD(exports, "OBS_content_getListInputSources", NodeOBS::Content::GetListInputSources);
		NODE_SET_METHOD(exports, "OBS_content_getListFilters", NodeOBS::Content::GetListFilters);
		NODE_SET_METHOD(exports, "OBS_content_getListCurrentTransitions", NodeOBS::Content::GetListCurrentTransitions);
		NODE_SET_METHOD(exports, "OBS_content_getListTransitions", NodeOBS::Content::GetListTransitions);
		NODE_SET_METHOD(exports, "OBS_content_createScene", NodeOBS::Content::CreateScene);
		NODE_SET_METHOD(exports, "OBS_content_removeScene", NodeOBS::Content::RemoveScene);
		NODE_SET_METHOD(exports, "OBS_content_addSource", NodeOBS::Content::AddSource);
		NODE_SET_METHOD(exports, "OBS_content_removeSource", NodeOBS::Content::RemoveSource);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFrame", NodeOBS::Content::GetSourceFrame);
		NODE_SET_METHOD(exports, "OBS_content_getSourceProperties", NodeOBS::Content::GetSourceProperties);
		NODE_SET_METHOD(exports, "OBS_content_getSourcePropertiesSubParameters", NodeOBS::Content::GetSourcePropertiesSubParameters);
		NODE_SET_METHOD(exports, "OBS_content_getSourcePropertyCurrentValue", NodeOBS::Content::GetSourcePropertyCurrentValue);
		NODE_SET_METHOD(exports, "OBS_content_setProperty", NodeOBS::Content::SetProperty);
		NODE_SET_METHOD(exports, "OBS_content_setCurrentScene", NodeOBS::Content::SetCurrentScene);
		NODE_SET_METHOD(exports, "OBS_content_setSourcePosition", NodeOBS::Content::SetSourcePosition);
		NODE_SET_METHOD(exports, "OBS_content_setSourceScaling", NodeOBS::Content::SetSourceScaling);
		NODE_SET_METHOD(exports, "OBS_content_renameTransition", NodeOBS::Content::RenameTransition);
		NODE_SET_METHOD(exports, "OBS_content_renameSourceFilter", NodeOBS::Content::RenameSourceFilter);
		NODE_SET_METHOD(exports, "OBS_content_renameSource", NodeOBS::Content::RenameSource);
		NODE_SET_METHOD(exports, "OBS_content_renameScene", NodeOBS::Content::RenameScene);
		NODE_SET_METHOD(exports, "OBS_content_getCurrentTransition", NodeOBS::Content::GetCurrentTransition);
		NODE_SET_METHOD(exports, "OBS_content_setTransitionDuration", NodeOBS::Content::SetTransitionDuration);
		NODE_SET_METHOD(exports, "OBS_content_getTransitionDuration", NodeOBS::Content::GetTransitionDuration);
		NODE_SET_METHOD(exports, "OBS_content_addTransition", NodeOBS::Content::AddTransition);
		NODE_SET_METHOD(exports, "OBS_content_removeTransition", NodeOBS::Content::RemoveTransition);
		NODE_SET_METHOD(exports, "OBS_content_setTransition", NodeOBS::Content::SetTransition);
		NODE_SET_METHOD(exports, "OBS_content_updateTransitionProperties", NodeOBS::Content::UpdateTransitionProperties);
		NODE_SET_METHOD(exports, "OBS_content_getTransitionProperties", NodeOBS::Content::GetTransitionProperties);
		NODE_SET_METHOD(exports, "OBS_content_getTransitionPropertiesSubParameters", NodeOBS::Content::GetTransitionPropertiesSubParameters);
		NODE_SET_METHOD(exports, "OBS_content_setTransitionProperty", NodeOBS::Content::SetTransitionProperty);
		NODE_SET_METHOD(exports, "OBS_content_getTransitionPropertyCurrentValue", NodeOBS::Content::GetTransitionPropertyCurrentValue);
		NODE_SET_METHOD(exports, "OBS_content_addSourceFilter", NodeOBS::Content::AddSourceFilter);
		NODE_SET_METHOD(exports, "OBS_content_removeSourceFilter", NodeOBS::Content::RemoveSourceFilter);
		NODE_SET_METHOD(exports, "OBS_content_updateSourceFilterProperties", NodeOBS::Content::UpdateSourceFilterProperties);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFilterProperties", NodeOBS::Content::GetSourceFilterProperties);
		NODE_SET_METHOD(exports, "OBS_content_getListSourceFilters", NodeOBS::Content::GetListSourceFilters);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFilterPropertyCurrentValue", NodeOBS::Content::GetSourceFilterPropertyCurrentValue);
		NODE_SET_METHOD(exports, "OBS_content_setSourceFilterProperty", NodeOBS::Content::SetSourceFilterProperty);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFilterPropertiesSubParameters", NodeOBS::Content::GetSourceFilterPropertiesSubParameters);
		NODE_SET_METHOD(exports, "OBS_content_getSourcePosition", NodeOBS::Content::GetSourcePosition);
		NODE_SET_METHOD(exports, "OBS_content_getSourceScaling", NodeOBS::Content::GetSourceScaling);
		NODE_SET_METHOD(exports, "OBS_content_getSourceSize", NodeOBS::Content::GetSourceSize);
		NODE_SET_METHOD(exports, "OBS_content_setSourceOrder", NodeOBS::Content::SetSourceOrder);
		NODE_SET_METHOD(exports, "OBS_content_updateSourceProperties", NodeOBS::Content::UpdateSourceProperties);
		NODE_SET_METHOD(exports, "OBS_content_createDisplay", NodeOBS::Content::CreateDisplay);
		NODE_SET_METHOD(exports, "OBS_content_destroyDisplay", NodeOBS::Content::DestroyDisplay);
		NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewOffset", NodeOBS::Content::GetDisplayPreviewOffset);
		NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewSize", NodeOBS::Content::GetDisplayPreviewSize);
		NODE_SET_METHOD(exports, "OBS_content_createSourcePreviewDisplay", NodeOBS::Content::CreateSourcePreviewDisplay);
		NODE_SET_METHOD(exports, "OBS_content_resizeDisplay", NodeOBS::Content::ResizeDisplay);
		NODE_SET_METHOD(exports, "OBS_content_moveDisplay", NodeOBS::Content::MoveDisplay);
		NODE_SET_METHOD(exports, "OBS_content_setPaddingSize", NodeOBS::Content::SetPaddingSize);
		NODE_SET_METHOD(exports, "OBS_content_setPaddingColor", NodeOBS::Content::SetPaddingColor);
		NODE_SET_METHOD(exports, "OBS_content_setBackgroundColor", NodeOBS::Content::SetBackgroundColor);
		NODE_SET_METHOD(exports, "OBS_content_setOutlineColor", NodeOBS::Content::SetOutlineColor);
		NODE_SET_METHOD(exports, "OBS_content_setGuidelineColor", NodeOBS::Content::SetGuidelineColor);
		NODE_SET_METHOD(exports, "OBS_content_setResizeBoxOuterColor", NodeOBS::Content::SetResizeBoxOuterColor);
		NODE_SET_METHOD(exports, "OBS_content_setResizeBoxInnerColor", NodeOBS::Content::SetResizeBoxInnerColor);
		NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", NodeOBS::Content::SetShouldDrawUI);
		NODE_SET_METHOD(exports, "OBS_content_selectSource", NodeOBS::Content::SelectSource);
		NODE_SET_METHOD(exports, "OBS_content_selectSources", NodeOBS::Content::SelectSources);
		NODE_SET_METHOD(exports, "OBS_content_dragSelectedSource", NodeOBS::Content::DragSelectedSource);
		NODE_SET_METHOD(exports, "OBS_content_loadConfigFile", NodeOBS::Content::LoadConfigFile);
		NODE_SET_METHOD(exports, "OBS_content_saveIntoConfigFile", NodeOBS::Content::SaveIntoConfigFile);
		NODE_SET_METHOD(exports, "OBS_content_getSourceFlags", NodeOBS::Content::GetSourceFlags);
		NODE_SET_METHOD(exports, "OBS_content_sourceSetMuted", NodeOBS::Content::SourceSetMuted);
		NODE_SET_METHOD(exports, "OBS_content_isSourceMuted", NodeOBS::Content::IsSourceMuted);
		NODE_SET_METHOD(exports, "OBS_content_getSourceVisibility", NodeOBS::Content::GetSourceVisibility);
		NODE_SET_METHOD(exports, "OBS_content_setSourceVisibility", NodeOBS::Content::SetSourceVisibility);
		NODE_SET_METHOD(exports, "OBS_content_fillTabScenes", NodeOBS::Content::FillTabScenes);

		// Service
		NODE_SET_METHOD(exports, "OBS_service_resetAudioContext", NodeOBS::Service::ResetAudioContext);
		NODE_SET_METHOD(exports, "OBS_service_resetVideoContext", NodeOBS::Service::ResetVideoContext);
		NODE_SET_METHOD(exports, "OBS_service_createAudioEncoder", NodeOBS::Service::CreateAudioEncoder);
		NODE_SET_METHOD(exports, "OBS_service_createVideoStreamingEncoder", NodeOBS::Service::CreateVideoStreamingEncoder);
		NODE_SET_METHOD(exports, "OBS_service_createVideoRecordingEncoder", NodeOBS::Service::CreateVideoRecordingEncoder);
		NODE_SET_METHOD(exports, "OBS_service_createService", NodeOBS::Service::CreateService);
		NODE_SET_METHOD(exports, "OBS_service_createRecordingSettings", NodeOBS::Service::CreateRecordingSettings);
		NODE_SET_METHOD(exports, "OBS_service_createStreamingOutput", NodeOBS::Service::CreateStreamingOutput);
		NODE_SET_METHOD(exports, "OBS_service_createRecordingOutput", NodeOBS::Service::CreateRecordingOutput);
		NODE_SET_METHOD(exports, "OBS_service_startStreaming", NodeOBS::Service::StartStreaming);
		NODE_SET_METHOD(exports, "OBS_service_startRecording", NodeOBS::Service::StartRecording);
		NODE_SET_METHOD(exports, "OBS_service_stopStreaming", NodeOBS::Service::StopStreaming);
		NODE_SET_METHOD(exports, "OBS_service_stopRecording", NodeOBS::Service::StopRecording);
		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", NodeOBS::Service::AssociateAVToStreamingContext);
		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext", NodeOBS::Service::AssociateAVToRecordingContext);
		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", NodeOBS::Service::AssociateAVEncodersToStreamingOutput);
		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", NodeOBS::Service::AssociateAVEncodersToRecordingOutput);
		NODE_SET_METHOD(exports, "OBS_service_setServiceToTheStreamingOutput", NodeOBS::Service::SetServiceToStreamingOutput);
		NODE_SET_METHOD(exports, "OBS_service_setRecordingSettings", NodeOBS::Service::SetRecordingSettings);
		NODE_SET_METHOD(exports, "OBS_service_isStreamingOutputActive", NodeOBS::Service::IsStreamingOutputActive);

		// Settings
		NODE_SET_METHOD(exports, "OBS_settings_getListCategories", NodeOBS::Settings::GetListCategories);
		NODE_SET_METHOD(exports, "OBS_settings_getSettings", NodeOBS::Settings::GetSettings);
		NODE_SET_METHOD(exports, "OBS_settings_saveSettings", NodeOBS::Settings::SaveSettings);

		//OBS_event
	//#define SET_SIGNAL_METHOD(name) NODE_SET_METHOD(exports, #name , name)
	//	SET_SIGNAL_METHOD(OBS_signal_sourceRemoved);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceDestroyed);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceSaved);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceLoaded);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceActivated);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceDeactivated);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceShown);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceHidden);
	//	SET_SIGNAL_METHOD(OBS_signal_sourceMuted);

	//	SET_SIGNAL_METHOD(OBS_signal_createdSource);
	//	SET_SIGNAL_METHOD(OBS_signal_removedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_destroyedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_savedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_loadedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_activatedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_deactivatedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_showedSource);
	//	SET_SIGNAL_METHOD(OBS_signal_hidSource);

	//	SET_SIGNAL_METHOD(OBS_signal_outputStarted);
	//	SET_SIGNAL_METHOD(OBS_signal_outputStopped);
	//	SET_SIGNAL_METHOD(OBS_signal_outputStarting);
	//	SET_SIGNAL_METHOD(OBS_signal_outputStopping);
	//	SET_SIGNAL_METHOD(OBS_signal_outputActivated);
	//	SET_SIGNAL_METHOD(OBS_signal_outputDeactivated);
	//	SET_SIGNAL_METHOD(OBS_signal_outputReconnecting);
	//	SET_SIGNAL_METHOD(OBS_signal_outputReconnected);
	//#undef SET_SIGNAL_METHOD
	});
}

void NodeOBS::Module::SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	Controller::GetInstance().GetConnection()->Call("NodeOBSModule", "SetWorkingDirectory",
		std::vector<IPC::Value>{std::string(*param0)}, nullptr, nullptr);
}

void NodeOBS::API::InitAPI(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "InitAPI",
		std::vector<IPC::Value>{std::string(*param0)}, nullptr, nullptr);
}

void NodeOBS::API::InitOBSAPI(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "InitOBSAPI",
		std::vector<IPC::Value>{std::string(*param0)}, nullptr, nullptr);
}

void NodeOBS::API::DestroyOBSAPI(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "DestroyOBSAPI",
		{}, nullptr, nullptr);
}

void NodeOBS::API::OpenAllModules(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "OpenAllModules",
		{}, nullptr, nullptr);
}

void NodeOBS::API::InitAllModules(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "InitAllModules",
		{}, nullptr, nullptr);
}

void NodeOBS::API::GetPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Local<v8::Object> stats = v8::Object::New(args.GetIsolate());

	// Return from opposite will be `[key], [value], ...` in IPC format.
	// [key] (String)
	// [value] (Integer64, Double, ...)
	struct CallData {
		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		v8::Local<v8::Object>* stats;
		v8::Isolate* iso;
	} cd;
	cd.stats = &stats;
	cd.iso = args.GetIsolate();

	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "GetPerformanceStatistics",
		{}, [](const void* data, const std::vector<IPC::Value>& rval) {
		CallData* cd = const_cast<CallData*>(reinterpret_cast<const CallData*>(data));

		if ((rval.size() % 2) != 0)
			throw std::runtime_error("GetPerformanceStatistics returned invalid message.");

		for (size_t n = 0; n < rval.size(); n += 2) {
			IPC::Value key, value;
			key = rval[n];
			value = rval[n + 1];

			switch (value.type) {
				case IPC::Type::Float:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.fp32));
					break;
				case IPC::Type::Double:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.fp64));
					break;
				case IPC::Type::Int32:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.i32));
					break;
				case IPC::Type::Int64:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.i64));
					break;
				case IPC::Type::UInt32:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.ui32));
					break;
				case IPC::Type::UInt64:
					(*cd->stats)->Set(v8::String::NewFromUtf8(cd->iso, key.value_str.c_str()),
						v8::Number::New(cd->iso, value.value.ui64));
					break;
			}
		}

		cd->cv.notify_all();
	}, &cd);

	{
		std::unique_lock<std::mutex> ulock;
		cd.cv.wait(ulock, [&cd]() { return cd.finished; });
	}

	args.GetReturnValue().Set(stats);
}

void NodeOBS::API::GetPathConfigDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	struct CallData {
		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		std::string dir;
	} cd;

	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "GetPathConfigDirectory",
		{}, [](const void* data, const std::vector<IPC::Value>& rval) {
		CallData* cd = const_cast<CallData*>(reinterpret_cast<const CallData*>(data));
		cd->dir = rval[0].value_str;
		cd->cv.notify_all();
	}, &cd);

	{
		std::unique_lock<std::mutex> ulock;
		cd.cv.wait(ulock, [&cd]() { return cd.finished; });
	}

	args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(), cd.dir.c_str()));
}

void NodeOBS::API::SetPathConfigDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	Controller::GetInstance().GetConnection()->Call("NodeOBSAPI", "SetPathConfigDirectory",
		std::vector<IPC::Value>{std::string(*param0)}, nullptr, nullptr);
}

void NodeOBS::AutoConfig::GetListServer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	v8::String::Utf8Value param1(args[1]->ToString());

	// Remote returns structure in the following order:
	// - name (String)
	// - address (String)
	// - bitrate (UInt64)
	// - ping (UInt64)

	struct CallData {
		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;

		std::vector<std::string> names;
		std::vector<std::string> addresses;
	} cd;

	Controller::GetInstance().GetConnection()->Call("NodeOBSAutoConfig", "GetListServer",
		std::vector<IPC::Value>{std::string(*param0), std::string(*param1)}, 
		[](const void* data, const std::vector<IPC::Value>& rval) {
		CallData* cd = const_cast<CallData*>(reinterpret_cast<const CallData*>(data));

		if ((rval.size() % 2) != 0)
			throw std::runtime_error("GetListServer returned invalid message.");

		size_t fsz = rval.size() / 2;
		cd->names.reserve(fsz);
		cd->addresses.reserve(fsz);

		for (size_t n = 0; n < fsz; n++) {
			cd->names.push_back(rval[n * 2].value_str);
			cd->addresses.push_back(rval[n * 2 + 1].value_str);
		}

		cd->cv.notify_all();
	}, &cd);

	{
		std::unique_lock<std::mutex> ulock;
		cd.cv.wait(ulock, [&cd]() { return cd.finished; });
	}

	// Convert to JavaScript object.
	v8::Local<v8::Array> res = v8::Array::New(args.GetIsolate());
	for (size_t n = 0; n < cd.names.size(); n++) {
		v8::Local<v8::Object> obj = v8::Object::New(args.GetIsolate());
		obj->Set(v8::String::NewFromUtf8(args.GetIsolate(), "server_name"),
			v8::String::NewFromUtf8(args.GetIsolate(), cd.names[n].c_str()));
		obj->Set(v8::String::NewFromUtf8(args.GetIsolate(), "server"),
			v8::String::NewFromUtf8(args.GetIsolate(), cd.addresses[n].c_str()));
		res->Set(n, obj);
	}

	args.GetReturnValue().Set(res);
}

void NodeOBS::AutoConfig::InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Local<v8::Function> param0(args[0].As<v8::Function>());
	v8::Local<v8::Object> param1(args[1].As<v8::Object>());
	// ToDo. May require two-way calling.
}

void NodeOBS::AutoConfig::StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::AutoConfig::TerminateAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFilterVisibility(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourceFilterVisibility(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFader(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::FlipHorizontalSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::FlipVerticalSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::ResetSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::StretchSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::FitSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::CenterSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSceneItemRotation(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSceneItemCrop(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSceneItemRotation(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSceneItemCrop(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListCurrentScenes(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListCurrentSourcesFromScene(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListInputSources(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListFilters(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListCurrentTransitions(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListTransitions(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::CreateScene(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RemoveScene(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::AddSource(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RemoveSource(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFrame(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourcePropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourcePropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetCurrentScene(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourcePosition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourceScaling(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RenameTransition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RenameSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RenameSource(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RenameScene(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetCurrentTransition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetTransitionDuration(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetTransitionDuration(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::AddTransition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RemoveTransition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetTransition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::UpdateTransitionProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetTransitionProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetTransitionPropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetTransitionProperty(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetTransitionPropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::AddSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::RemoveSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::UpdateSourceFilterProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFilterProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetListSourceFilters(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFilterPropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourceFilterProperty(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFilterPropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourcePosition(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceScaling(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceSize(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourceOrder(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::UpdateSourceProperties(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::CreateDisplay(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::DestroyDisplay(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetDisplayPreviewOffset(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetDisplayPreviewSize(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::CreateSourcePreviewDisplay(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::ResizeDisplay(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::MoveDisplay(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetPaddingSize(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetPaddingColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetBackgroundColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetOutlineColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetGuidelineColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetResizeBoxOuterColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetResizeBoxInnerColor(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetShouldDrawUI(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SelectSource(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SelectSources(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::DragSelectedSource(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::LoadConfigFile(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SaveIntoConfigFile(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceFlags(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SourceSetMuted(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::IsSourceMuted(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::GetSourceVisibility(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::SetSourceVisibility(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Content::FillTabScenes(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::ResetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::ResetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateService(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::CreateRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::StartStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::StopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::StartRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::StopRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::AssociateAVToStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::AssociateAVToRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::AssociateAVEncodersToStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::AssociateAVEncodersToRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::SetServiceToStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::SetRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Service::IsStreamingOutputActive(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Settings::GetListCategories(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Settings::GetSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Settings::SaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceRemoved(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceDestroyed(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceSaved(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceLoaded(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceActivated(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

void NodeOBS::Signal::SourceDeactivated(const v8::FunctionCallbackInfo<v8::Value>& args) {

}
