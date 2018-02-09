#include "nodeobs_module.h"
#include <node.h>
#include "nodeobs_api.h"
#include "nodeobs_service.h"
#include "nodeobs_content.h"
#include "nodeobs_settings.h"
#include "nodeobs_autoconfig.h"

std::string g_moduleDirectory = "";

using namespace v8;

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value param0(args[0]->ToString());
	g_moduleDirectory = std::string(*param0);
	replaceAll(g_moduleDirectory, "\\", "/");
}

void nodeobs_init(Local<Object> exports) {
	// EDIT: Add function to specify actual load directory.
	NODE_SET_METHOD(exports, "SetWorkingDirectory", SetWorkingDirectory);
	// END OF EDIT:

    /// Functions ///

    //OBS_AutoConfig
    NODE_SET_METHOD(exports, "GetListServer", GetListServer);

    NODE_SET_METHOD(exports, "InitializeAutoConfig", InitializeAutoConfig);

    NODE_SET_METHOD(exports, "StartBandwidthTest", StartBandwidthTest);
    NODE_SET_METHOD(exports, "StartStreamEncoderTest", StartStreamEncoderTest);
    NODE_SET_METHOD(exports, "StartRecordingEncoderTest", StartRecordingEncoderTest);

    NODE_SET_METHOD(exports, "StartCheckSettings", StartCheckSettings);
    NODE_SET_METHOD(exports, "StartSetDefaultSettings", StartSetDefaultSettings);

    NODE_SET_METHOD(exports, "StartSaveStreamSettings", StartSaveStreamSettings);
    NODE_SET_METHOD(exports, "StartSaveSettings", StartSaveSettings);

    NODE_SET_METHOD(exports, "TerminateAutoConfig", TerminateAutoConfig);

    //OBS_API
    NODE_SET_METHOD(exports, "OBS_API_initAPI", OBS_API::OBS_API_initAPI);
    NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", OBS_API::OBS_API_destroyOBS_API);
    NODE_SET_METHOD(exports, "OBS_API_openAllModules", OBS_API::OBS_API_openAllModules);
    NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", OBS_API::OBS_API_getPerformanceStatistics);
    NODE_SET_METHOD(exports, "OBS_API_getPathConfigDirectory", OBS_API::OBS_API_getPathConfigDirectory);
    NODE_SET_METHOD(exports, "OBS_API_setPathConfigDirectory", OBS_API::OBS_API_setPathConfigDirectory);
    NODE_SET_METHOD(exports, "OBS_API_getOBS_existingProfiles", OBS_API::OBS_API_getOBS_existingProfiles);
    NODE_SET_METHOD(exports, "OBS_API_getOBS_existingSceneCollections", OBS_API::OBS_API_getOBS_existingSceneCollections);
    NODE_SET_METHOD(exports, "OBS_API_getOBS_currentProfile", OBS_API::OBS_API_getOBS_currentProfile);
    NODE_SET_METHOD(exports, "OBS_API_setOBS_currentProfile", OBS_API::OBS_API_setOBS_currentProfile);
    NODE_SET_METHOD(exports, "OBS_API_getOBS_currentSceneCollection", OBS_API::OBS_API_getOBS_currentSceneCollection);
    NODE_SET_METHOD(exports, "OBS_API_setOBS_currentSceneCollection", OBS_API::OBS_API_setOBS_currentSceneCollection);
    NODE_SET_METHOD(exports, "OBS_API_isOBS_installed", OBS_API::OBS_API_isOBS_installed);
    NODE_SET_METHOD(exports, "OBS_API_useOBS_config", OBS_API::OBS_API_useOBS_config);

    //OBS_service
    NODE_SET_METHOD(exports, "OBS_service_resetAudioContext", OBS_service::OBS_service_resetAudioContext);
    NODE_SET_METHOD(exports, "OBS_service_resetVideoContext", OBS_service::OBS_service_resetVideoContext);
    NODE_SET_METHOD(exports, "OBS_service_createAudioEncoder", OBS_service::OBS_service_createAudioEncoder);
    NODE_SET_METHOD(exports, "OBS_service_createVideoStreamingEncoder", OBS_service::OBS_service_createVideoStreamingEncoder);
    NODE_SET_METHOD(exports, "OBS_service_createVideoRecordingEncoder", OBS_service::OBS_service_createVideoRecordingEncoder);
    NODE_SET_METHOD(exports, "OBS_service_createService", OBS_service::OBS_service_createService);
    NODE_SET_METHOD(exports, "OBS_service_createRecordingSettings", OBS_service::OBS_service_createRecordingSettings);
    NODE_SET_METHOD(exports, "OBS_service_createStreamingOutput", OBS_service::OBS_service_createStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_createRecordingOutput", OBS_service::OBS_service_createRecordingOutput);
    NODE_SET_METHOD(exports, "OBS_service_startStreaming", OBS_service::OBS_service_startStreaming);
    NODE_SET_METHOD(exports, "OBS_service_startRecording", OBS_service::OBS_service_startRecording);
    NODE_SET_METHOD(exports, "OBS_service_stopStreaming", OBS_service::OBS_service_stopStreaming);
    NODE_SET_METHOD(exports, "OBS_service_stopRecording", OBS_service::OBS_service_stopRecording);
    NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", OBS_service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext);
    NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext", OBS_service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext);
    NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", OBS_service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", OBS_service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput);
    NODE_SET_METHOD(exports, "OBS_service_setServiceToTheStreamingOutput", OBS_service::OBS_service_setServiceToTheStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_setRecordingSettings", OBS_service::OBS_service_setRecordingSettings);
    NODE_SET_METHOD(exports, "OBS_service_isStreamingOutputActive", OBS_service::OBS_service_isStreamingOutputActive);

    //OBS_content
    NODE_SET_METHOD(exports, "OBS_content_createDisplay", OBS_content::OBS_content_createDisplay);
    NODE_SET_METHOD(exports, "OBS_content_destroyDisplay", OBS_content::OBS_content_destroyDisplay);
    NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewOffset", OBS_content::OBS_content_getDisplayPreviewOffset);
    NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewSize", OBS_content::OBS_content_getDisplayPreviewSize);
    NODE_SET_METHOD(exports, "OBS_content_createSourcePreviewDisplay", OBS_content::OBS_content_createSourcePreviewDisplay);
    NODE_SET_METHOD(exports, "OBS_content_resizeDisplay", OBS_content::OBS_content_resizeDisplay);
    NODE_SET_METHOD(exports, "OBS_content_moveDisplay", OBS_content::OBS_content_moveDisplay);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingSize", OBS_content::OBS_content_setPaddingSize);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingColor", OBS_content::OBS_content_setPaddingColor);
	NODE_SET_METHOD(exports, "OBS_content_setBackgroundColor", OBS_content::OBS_content_setBackgroundColor);
	NODE_SET_METHOD(exports, "OBS_content_setOutlineColor", OBS_content::OBS_content_setOutlineColor);
	NODE_SET_METHOD(exports, "OBS_content_setGuidelineColor", OBS_content::OBS_content_setGuidelineColor);
	NODE_SET_METHOD(exports, "OBS_content_setResizeBoxOuterColor", OBS_content::OBS_content_setResizeBoxOuterColor);
	NODE_SET_METHOD(exports, "OBS_content_setResizeBoxInnerColor", OBS_content::OBS_content_setResizeBoxInnerColor);
	NODE_SET_METHOD(exports, "OBS_content_selectSource", OBS_content::OBS_content_selectSource);
    NODE_SET_METHOD(exports, "OBS_content_selectSources", OBS_content::OBS_content_selectSources);
    NODE_SET_METHOD(exports, "OBS_content_dragSelectedSource", OBS_content::OBS_content_dragSelectedSource);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_getDrawGuideLines", OBS_content::OBS_content_getDrawGuideLines);
	NODE_SET_METHOD(exports, "OBS_content_setDrawGuideLines", OBS_content::OBS_content_setDrawGuideLines);

    //OBS_settings
    NODE_SET_METHOD(exports, "OBS_settings_getListCategories", OBS_settings::OBS_settings_getListCategories);
    NODE_SET_METHOD(exports, "OBS_settings_getSettings", OBS_settings::OBS_settings_getSettings);
    NODE_SET_METHOD(exports, "OBS_settings_saveSettings", OBS_settings::OBS_settings_saveSettings);
}
