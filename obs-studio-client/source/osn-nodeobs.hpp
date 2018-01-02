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

#pragma once
#include <node.h>

namespace NodeOBS {
	class Module {
		public:
		static void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class API {
		public:
		static void InitAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void InitOBSAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DestroyOBSAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OpenAllModules(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void InitAllModules(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetPathConfigDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetPathConfigDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetExistingOBSProfiles(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetExistingOBSSceneCollections(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetCurrentOBSProfile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetCurrentOBSProfile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetCurrentOBSSceneCollection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetCurrentOBSSceneCollection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsOBSInstalled(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void UseOBSConfiguration(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class Audio {
		public:
		static void CreateFader(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DestroyFader(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderSetDb(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderGetDb(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderSetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderGetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderSetMul(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderGetMul(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FaderDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args);

		public:
		static void CreateVolMeter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DestroyVolMeter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterSetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterGetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterSetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterGetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void VolMeterRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class AutoConfig {
		public:
		static void GetListServer(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void TerminateAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class Content {
		public:
		static void GetSourceFilterVisibility(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSourceFilterVisibility(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFader(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FlipHorizontalSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FlipVerticalSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ResetSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StretchSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FitSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CenterSceneItems(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSceneItemRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSceneItemCrop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSceneItemRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSceneItemCrop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListCurrentScenes(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListCurrentSourcesFromScene(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListInputSources(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListFilters(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListCurrentTransitions(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListTransitions(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateScene(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveScene(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFrame(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourcePropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourcePropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetCurrentScene(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSourcePosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSourceScaling(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void RenameTransition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RenameSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RenameSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RenameScene(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void GetCurrentTransition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetTransitionDuration(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetTransitionDuration(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddTransition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveTransition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetTransition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void UpdateTransitionProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetTransitionProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetTransitionPropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetTransitionProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetTransitionPropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void AddSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveSourceFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void UpdateSourceFilterProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFilterProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetListSourceFilters(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFilterPropertyCurrentValue(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSourceFilterProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFilterPropertiesSubParameters(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void GetSourcePosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceScaling(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceSize(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void SetSourceOrder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void UpdateSourceProperties(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void CreateDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DestroyDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDisplayPreviewOffset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDisplayPreviewSize(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateSourcePreviewDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ResizeDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MoveDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetPaddingSize(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetPaddingColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBackgroundColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetOutlineColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetGuidelineColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetResizeBoxOuterColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetResizeBoxInnerColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetShouldDrawUI(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void SelectSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SelectSources(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DragSelectedSource(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void LoadConfigFile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SaveIntoConfigFile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceFlags(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceSetMuted(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsSourceMuted(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSourceVisibility(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSourceVisibility(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FillTabScenes(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class Service {
		public:
		static void ResetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ResetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateService(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StartRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void StopRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AssociateAVToStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AssociateAVToRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AssociateAVEncodersToStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AssociateAVEncodersToRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetServiceToStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsStreamingOutputActive(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class Settings {
		public:
		static void GetListCategories(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

	class Signal {
		public:
		static void SourceRemoved(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceDestroyed(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceSaved(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceLoaded(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceActivated(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceDeactivated(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceRemoved(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SourceRemoved(const v8::FunctionCallbackInfo<v8::Value>& args);
	};
}
