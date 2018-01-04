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

#include "osn-nodeobs.hpp"
#include "utility.hpp"
#include "shared.hpp"

#ifdef _WIN32
#include <direct.h>
#endif

// Global Storage (Should be changed in the future)
static std::string g_moduleDirectory = ""; // Module Directory

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
		NODE_SET_METHOD(exports, "OBS_API_getOBS_existingProfiles", NodeOBS::API::GetExistingOBSProfiles);
		NODE_SET_METHOD(exports, "OBS_API_getOBS_existingSceneCollections", NodeOBS::API::GetExistingOBSSceneCollections);
		NODE_SET_METHOD(exports, "OBS_API_getOBS_currentProfile", NodeOBS::API::GetCurrentOBSProfile);
		NODE_SET_METHOD(exports, "OBS_API_setOBS_currentProfile", NodeOBS::API::SetCurrentOBSProfile);
		NODE_SET_METHOD(exports, "OBS_API_getOBS_currentSceneCollection", NodeOBS::API::GetCurrentOBSSceneCollection);
		NODE_SET_METHOD(exports, "OBS_API_setOBS_currentSceneCollection", NodeOBS::API::SetCurrentOBSSceneCollection);
		NODE_SET_METHOD(exports, "OBS_API_isOBS_installed", NodeOBS::API::IsOBSInstalled);
		NODE_SET_METHOD(exports, "OBS_API_useOBS_config", NodeOBS::API::UseOBSConfiguration);

		// Audio
		NODE_SET_METHOD(exports, "OBS_audio_createFader", NodeOBS::Audio::CreateFader);
		NODE_SET_METHOD(exports, "OBS_audio_destroyFader", NodeOBS::Audio::DestroyFader);
		NODE_SET_METHOD(exports, "OBS_audio_faderAddCallback", NodeOBS::Audio::FaderAddCallback);
		NODE_SET_METHOD(exports, "OBS_audio_faderRemoveCallback", NodeOBS::Audio::FaderRemoveCallback);
		NODE_SET_METHOD(exports, "OBS_audio_faderSetDb", NodeOBS::Audio::FaderSetDb);
		NODE_SET_METHOD(exports, "OBS_audio_faderGetDb", NodeOBS::Audio::FaderGetDb);
		NODE_SET_METHOD(exports, "OBS_audio_faderSetDeflection", NodeOBS::Audio::FaderSetDeflection);
		NODE_SET_METHOD(exports, "OBS_audio_faderGetDeflection", NodeOBS::Audio::FaderGetDeflection);
		NODE_SET_METHOD(exports, "OBS_audio_faderSetMul", NodeOBS::Audio::FaderSetMul);
		NODE_SET_METHOD(exports, "OBS_audio_faderGetMul", NodeOBS::Audio::FaderGetMul);
		NODE_SET_METHOD(exports, "OBS_audio_faderAttachSource", NodeOBS::Audio::FaderAttachSource);
		NODE_SET_METHOD(exports, "OBS_audio_faderDetachSource", NodeOBS::Audio::FaderDetachSource);

		NODE_SET_METHOD(exports, "OBS_audio_createVolmeter", NodeOBS::Audio::CreateVolMeter);
		NODE_SET_METHOD(exports, "OBS_audio_destroyVolmeter", NodeOBS::Audio::DestroyVolMeter);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterAttachSource", NodeOBS::Audio::VolMeterAttachSource);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterDetachSource", NodeOBS::Audio::VolMeterDetachSource);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterSetUpdateInterval", NodeOBS::Audio::VolMeterSetUpdateInterval);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterGetUpdateInterval", NodeOBS::Audio::VolMeterGetUpdateInterval);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterSetPeakHold", NodeOBS::Audio::VolMeterSetPeakHold);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterGetPeakHold", NodeOBS::Audio::VolMeterGetPeakHold);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterAddCallback", NodeOBS::Audio::VolMeterAddCallback);
		NODE_SET_METHOD(exports, "OBS_audio_volmeterRemoveCallback", NodeOBS::Audio::VolMeterRemoveCallback);

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
	g_moduleDirectory = std::string(*param0);
	replaceAll(g_moduleDirectory, "\\", "/");
}
