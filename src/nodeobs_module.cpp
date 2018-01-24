#include "nodeobs_module.h"
#include <node.h>
#include "nodeobs_api.h"
#include "nodeobs_service.h"
#include "nodeobs_content.h"
#include "nodeobs_settings.h"
#include "nodeobs_event.h"
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
	// Find current directory.
	{
		std::vector<char> pathCWD(65535); // Should use MAX_PATH here
		char *answer = getcwd(pathCWD.data(), pathCWD.size() - 1);
		g_moduleDirectory = std::string(pathCWD.data()) + "/node_modules/obs-studio-node/distribute";
		replaceAll(g_moduleDirectory, "\\", "/");
	}

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
    NODE_SET_METHOD(exports, "OBS_API_initOBS_API", OBS_API::OBS_API_initOBS_API);
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

    //OBS_API unit tests
    NODE_SET_METHOD(exports, "OBS_API_test_openAllModules", OBS_API::OBS_API_test_openAllModules);

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

    //OBS_service unit tests
    NODE_SET_METHOD(exports, "OBS_service_test_resetAudioContext", OBS_service::OBS_service_test_resetAudioContext);
    NODE_SET_METHOD(exports, "OBS_service_test_resetVideoContext", OBS_service::OBS_service_test_resetVideoContext);
    NODE_SET_METHOD(exports, "OBS_service_test_createAudioEncoder", OBS_service::OBS_service_test_createAudioEncoder);
    NODE_SET_METHOD(exports, "OBS_service_test_createVideoStreamingEncoder", OBS_service::OBS_service_test_createVideoStreamingEncoder);
    NODE_SET_METHOD(exports, "OBS_service_test_createVideoRecordingEncoder", OBS_service::OBS_service_test_createVideoRecordingEncoder);
    NODE_SET_METHOD(exports, "OBS_service_test_createService", OBS_service::OBS_service_test_createService);
    NODE_SET_METHOD(exports, "OBS_service_test_createRecordingSettings", OBS_service::OBS_service_test_createRecordingSettings);
    NODE_SET_METHOD(exports, "OBS_service_test_createStreamingOutput", OBS_service::OBS_service_test_createStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_test_createRecordingOutput", OBS_service::OBS_service_test_createRecordingOutput);
    NODE_SET_METHOD(exports, "OBS_service_test_startStreaming", OBS_service::OBS_service_test_startStreaming);
    NODE_SET_METHOD(exports, "OBS_service_test_startRecording", OBS_service::OBS_service_test_startRecording);
    NODE_SET_METHOD(exports, "OBS_service_test_stopStreaming", OBS_service::OBS_service_test_stopStreaming);
    NODE_SET_METHOD(exports, "OBS_service_test_stopRecording", OBS_service::OBS_service_test_stopRecording);
    NODE_SET_METHOD(exports, "OBS_service_test_associateAudioAndVideoToTheCurrentStreamingContext", OBS_service::OBS_service_test_associateAudioAndVideoToTheCurrentStreamingContext);
    NODE_SET_METHOD(exports, "OBS_service_test_associateAudioAndVideoToTheCurrentRecordingContext", OBS_service::OBS_service_test_associateAudioAndVideoToTheCurrentRecordingContext);
    NODE_SET_METHOD(exports, "OBS_service_test_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", OBS_service::OBS_service_test_associateAudioAndVideoEncodersToTheCurrentStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_test_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", OBS_service::OBS_service_test_associateAudioAndVideoEncodersToTheCurrentRecordingOutput);
    NODE_SET_METHOD(exports, "OBS_service_test_setServiceToTheStreamingOutput", OBS_service::OBS_service_test_setServiceToTheStreamingOutput);
    NODE_SET_METHOD(exports, "OBS_service_test_setRecordingSettings", OBS_service::OBS_service_test_setRecordingSettings);

    //OBS_event
    #define SET_SIGNAL_METHOD(name) \
        NODE_SET_METHOD(exports, #name , name)

    SET_SIGNAL_METHOD(OBS_signal_sourceRemoved);
    SET_SIGNAL_METHOD(OBS_signal_sourceDestroyed);
    SET_SIGNAL_METHOD(OBS_signal_sourceSaved);
    SET_SIGNAL_METHOD(OBS_signal_sourceLoaded);
    SET_SIGNAL_METHOD(OBS_signal_sourceActivated);
    SET_SIGNAL_METHOD(OBS_signal_sourceDeactivated);
    SET_SIGNAL_METHOD(OBS_signal_sourceShown);
    SET_SIGNAL_METHOD(OBS_signal_sourceHidden);
    SET_SIGNAL_METHOD(OBS_signal_sourceMuted);

    SET_SIGNAL_METHOD(OBS_signal_createdSource);
    SET_SIGNAL_METHOD(OBS_signal_removedSource);
    SET_SIGNAL_METHOD(OBS_signal_destroyedSource);
    SET_SIGNAL_METHOD(OBS_signal_savedSource);
    SET_SIGNAL_METHOD(OBS_signal_loadedSource);
    SET_SIGNAL_METHOD(OBS_signal_activatedSource);
    SET_SIGNAL_METHOD(OBS_signal_deactivatedSource);
    SET_SIGNAL_METHOD(OBS_signal_showedSource);
    SET_SIGNAL_METHOD(OBS_signal_hidSource);

    SET_SIGNAL_METHOD(OBS_signal_outputStarted);
    SET_SIGNAL_METHOD(OBS_signal_outputStopped);
    SET_SIGNAL_METHOD(OBS_signal_outputStarting);
    SET_SIGNAL_METHOD(OBS_signal_outputStopping);
    SET_SIGNAL_METHOD(OBS_signal_outputActivated);
    SET_SIGNAL_METHOD(OBS_signal_outputDeactivated);
    SET_SIGNAL_METHOD(OBS_signal_outputReconnecting);
    SET_SIGNAL_METHOD(OBS_signal_outputReconnected);

    #undef SET_SIGNAL_METHOD

    //OBS_content

    NODE_SET_METHOD(exports, "OBS_content_getSourceFilterVisibility", OBS_content::OBS_content_getSourceFilterVisibility);
    NODE_SET_METHOD(exports, "OBS_content_setSourceFilterVisibility", OBS_content::OBS_content_setSourceFilterVisibility);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFader", OBS_content::OBS_content_getSourceFader);
    NODE_SET_METHOD(exports, "OBS_content_getSourceVolmeter", OBS_content::OBS_content_getSourceVolmeter);
    NODE_SET_METHOD(exports, "OBS_content_flipHorzSceneItems", OBS_content::OBS_content_flipHorzSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_flipVertSceneItems", OBS_content::OBS_content_flipVertSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_resetSceneItems", OBS_content::OBS_content_resetSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_stretchSceneItems", OBS_content::OBS_content_stretchSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_fitSceneItems", OBS_content::OBS_content_fitSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_centerSceneItems", OBS_content::OBS_content_centerSceneItems);
    NODE_SET_METHOD(exports, "OBS_content_getSceneItemRot", OBS_content::OBS_content_getSceneItemRot);
    NODE_SET_METHOD(exports, "OBS_content_getSceneItemCrop", OBS_content::OBS_content_getSceneItemCrop);
    NODE_SET_METHOD(exports, "OBS_content_setSceneItemRot", OBS_content::OBS_content_setSceneItemRot);
    NODE_SET_METHOD(exports, "OBS_content_setSceneItemCrop", OBS_content::OBS_content_setSceneItemCrop);
    NODE_SET_METHOD(exports, "OBS_content_getListCurrentScenes", OBS_content::OBS_content_getListCurrentScenes);
    NODE_SET_METHOD(exports, "OBS_content_getListCurrentSourcesFromScene", OBS_content::OBS_content_getListCurrentSourcesFromScene);
    NODE_SET_METHOD(exports, "OBS_content_getListInputSources", OBS_content::OBS_content_getListInputSources);
    NODE_SET_METHOD(exports, "OBS_content_getListFilters", OBS_content::OBS_content_getListFilters);
    NODE_SET_METHOD(exports, "OBS_content_getListCurrentTransitions", OBS_content::OBS_content_getListCurrentTransitions);
    NODE_SET_METHOD(exports, "OBS_content_getListTransitions", OBS_content::OBS_content_getListTransitions);
    NODE_SET_METHOD(exports, "OBS_content_createScene", OBS_content::OBS_content_createScene);
    NODE_SET_METHOD(exports, "OBS_content_removeScene", OBS_content::OBS_content_removeScene);
    NODE_SET_METHOD(exports, "OBS_content_addSource", OBS_content::OBS_content_addSource);
    NODE_SET_METHOD(exports, "OBS_content_removeSource", OBS_content::OBS_content_removeSource);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFrame", OBS_content::OBS_content_getSourceFrame);
    NODE_SET_METHOD(exports, "OBS_content_getSourceProperties", OBS_content::OBS_content_getSourceProperties);
    NODE_SET_METHOD(exports, "OBS_content_getSourcePropertiesSubParameters", OBS_content::OBS_content_getSourcePropertiesSubParameters);
    NODE_SET_METHOD(exports, "OBS_content_getSourcePropertyCurrentValue", OBS_content::OBS_content_getSourcePropertyCurrentValue);
    NODE_SET_METHOD(exports, "OBS_content_setProperty", OBS_content::OBS_content_setProperty);
    NODE_SET_METHOD(exports, "OBS_content_setCurrentScene", OBS_content::OBS_content_setCurrentScene);
    NODE_SET_METHOD(exports, "OBS_content_setSourcePosition", OBS_content::OBS_content_setSourcePosition);
    NODE_SET_METHOD(exports, "OBS_content_setSourceScaling", OBS_content::OBS_content_setSourceScaling);

    NODE_SET_METHOD(exports, "OBS_content_renameTransition", OBS_content::OBS_content_renameTransition);
    NODE_SET_METHOD(exports, "OBS_content_renameSourceFilter", OBS_content::OBS_content_renameSourceFilter);
    NODE_SET_METHOD(exports, "OBS_content_renameSource", OBS_content::OBS_content_renameSource);
    NODE_SET_METHOD(exports, "OBS_content_renameScene", OBS_content::OBS_content_renameScene);

    NODE_SET_METHOD(exports, "OBS_content_getCurrentTransition", OBS_content::OBS_content_getCurrentTransition);
    NODE_SET_METHOD(exports, "OBS_content_setTransitionDuration", OBS_content::OBS_content_setTransitionDuration);
    NODE_SET_METHOD(exports, "OBS_content_getTransitionDuration", OBS_content::OBS_content_getTransitionDuration);
    NODE_SET_METHOD(exports, "OBS_content_addTransition", OBS_content::OBS_content_addTransition);
    NODE_SET_METHOD(exports, "OBS_content_removeTransition", OBS_content::OBS_content_removeTransition);
    NODE_SET_METHOD(exports, "OBS_content_setTransition", OBS_content::OBS_content_setTransition);
    NODE_SET_METHOD(exports, "OBS_content_updateTransitionProperties", OBS_content::OBS_content_updateTransitionProperties);
    NODE_SET_METHOD(exports, "OBS_content_getTransitionProperties", OBS_content::OBS_content_getTransitionProperties);
    NODE_SET_METHOD(exports, "OBS_content_getTransitionPropertiesSubParameters", OBS_content::OBS_content_getTransitionPropertiesSubParameters);
    NODE_SET_METHOD(exports, "OBS_content_setTransitionProperty", OBS_content::OBS_content_setTransitionProperty);
    NODE_SET_METHOD(exports, "OBS_content_getTransitionPropertyCurrentValue", OBS_content::OBS_content_getTransitionPropertyCurrentValue);

    NODE_SET_METHOD(exports, "OBS_content_addSourceFilter", OBS_content::OBS_content_addSourceFilter);
    NODE_SET_METHOD(exports, "OBS_content_removeSourceFilter", OBS_content::OBS_content_removeSourceFilter);
    NODE_SET_METHOD(exports, "OBS_content_updateSourceFilterProperties", OBS_content::OBS_content_updateSourceFilterProperties);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFilterProperties", OBS_content::OBS_content_getSourceFilterProperties);
    NODE_SET_METHOD(exports, "OBS_content_getListSourceFilters", OBS_content::OBS_content_getListSourceFilters);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFilterPropertyCurrentValue", OBS_content::OBS_content_getSourceFilterPropertyCurrentValue);
    NODE_SET_METHOD(exports, "OBS_content_setSourceFilterProperty", OBS_content::OBS_content_setSourceFilterProperty);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFilterPropertiesSubParameters", OBS_content::OBS_content_getSourceFilterPropertiesSubParameters);

    NODE_SET_METHOD(exports, "OBS_content_getSourcePosition", OBS_content::OBS_content_getSourcePosition);
    NODE_SET_METHOD(exports, "OBS_content_getSourceScaling", OBS_content::OBS_content_getSourceScaling);
    NODE_SET_METHOD(exports, "OBS_content_getSourceSize", OBS_content::OBS_content_getSourceSize);

    NODE_SET_METHOD(exports, "OBS_content_setSourceOrder", OBS_content::OBS_content_setSourceOrder);
    NODE_SET_METHOD(exports, "OBS_content_updateSourceProperties", OBS_content::OBS_content_updateSourceProperties);

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
    NODE_SET_METHOD(exports, "OBS_content_loadConfigFile", OBS_content::OBS_content_loadConfigFile);
    NODE_SET_METHOD(exports, "OBS_content_saveIntoConfigFile", OBS_content::OBS_content_saveIntoConfigFile);
    NODE_SET_METHOD(exports, "OBS_content_getSourceFlags", OBS_content::OBS_content_getSourceFlags);
    NODE_SET_METHOD(exports, "OBS_content_sourceSetMuted", OBS_content::OBS_content_sourceSetMuted);
    NODE_SET_METHOD(exports, "OBS_content_isSourceMuted", OBS_content::OBS_content_isSourceMuted);
    NODE_SET_METHOD(exports, "OBS_content_getSourceVisibility", OBS_content::OBS_content_getSourceVisibility);
    NODE_SET_METHOD(exports, "OBS_content_setSourceVisibility", OBS_content::OBS_content_setSourceVisibility);
    NODE_SET_METHOD(exports, "OBS_content_fillTabScenes", OBS_content::OBS_content_fillTabScenes);
    NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", OBS_content::OBS_content_setShouldDrawUI);

    //OBS_content unit tests
    NODE_SET_METHOD(exports, "OBS_content_test_getListCurrentScenes", OBS_content::OBS_content_test_getListCurrentScenes);
    NODE_SET_METHOD(exports, "OBS_content_test_getListCurrentSourcesFromScene", OBS_content::OBS_content_test_getListCurrentSourcesFromScene);
    NODE_SET_METHOD(exports, "OBS_content_test_getListInputSources", OBS_content::OBS_content_test_getListInputSources);
    NODE_SET_METHOD(exports, "OBS_content_test_getListFilters", OBS_content::OBS_content_test_getListFilters);
    NODE_SET_METHOD(exports, "OBS_content_test_getListTransitions", OBS_content::OBS_content_test_getListTransitions);
    NODE_SET_METHOD(exports, "OBS_content_test_createScene", OBS_content::OBS_content_test_createScene);
    NODE_SET_METHOD(exports, "OBS_content_test_removeScene", OBS_content::OBS_content_test_removeScene);
    NODE_SET_METHOD(exports, "OBS_content_test_addSource", OBS_content::OBS_content_test_addSource);
    NODE_SET_METHOD(exports, "OBS_content_test_removeSource", OBS_content::OBS_content_test_removeSource);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourceProperties", OBS_content::OBS_content_test_getSourceProperties);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertiesSubParameters", OBS_content::OBS_content_test_getSourcePropertiesSubParameters);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue", OBS_content::OBS_content_test_getSourcePropertyCurrentValue);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_boolType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_boolType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_colorType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_colorType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_intType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_intType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_floatType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_floatType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_textType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_textType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_fontType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_fontType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_pathType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_pathType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_buttonType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_buttonType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_editableListType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_editableListType);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_listType_intFormat", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_intFormat);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_listType_floatFormat", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_floatFormat);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_listType_stringFormat", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_stringFormat);
    NODE_SET_METHOD(exports, "OBS_content_test_getSourcePropertyCurrentValue_frameRateType", OBS_content::OBS_content_test_getSourcePropertyCurrentValue_frameRateType);
    NODE_SET_METHOD(exports, "OBS_content_test_setProperty", OBS_content::OBS_content_test_setProperty);
    NODE_SET_METHOD(exports, "OBS_content_test_setSourcePosition", OBS_content::OBS_content_test_setSourcePosition);
    NODE_SET_METHOD(exports, "OBS_content_test_setSourceScaling", OBS_content::OBS_content_test_setSourceScaling);
    NODE_SET_METHOD(exports, "OBS_content_test_setSourceOrder", OBS_content::OBS_content_test_setSourceOrder);

    //OBS_settings
    NODE_SET_METHOD(exports, "OBS_settings_getListCategories", OBS_settings::OBS_settings_getListCategories);
    NODE_SET_METHOD(exports, "OBS_settings_getSettings", OBS_settings::OBS_settings_getSettings);
    NODE_SET_METHOD(exports, "OBS_settings_saveSettings", OBS_settings::OBS_settings_saveSettings);
}
