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
#include <ipc-value.hpp>
#include <ipc-function.hpp>
#include <ipc-server.hpp>
#include <vector>
#include <inttypes.h>

namespace NodeOBS {
	class Module {
		public:
		static void Register(IPC::Server&);

		public:
		static void SetWorkingDirectory(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class API {
		public:
		static void Register(IPC::Server&);

		public:
		static void InitAPI(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		//static void InitOBSAPI(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void DestroyOBSAPI(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void OpenAllModules(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void InitAllModules(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetPerformanceStatistics(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetPathConfigDirectory(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetPathConfigDirectory(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetExistingOBSProfiles(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetExistingOBSSceneCollections(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetCurrentOBSProfile(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetCurrentOBSProfile(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetCurrentOBSSceneCollection(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetCurrentOBSSceneCollection(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void IsOBSInstalled(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void UseOBSConfiguration(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		private:
		static void initAPI();
		static bool initOBS_API();

		static std::string GenerateTimeDateFilename(const char* ext);
		static void stdout_log_handler(int log_level, const char *format, va_list args, void *param);
		static void node_obs_log(int log_level, const char *msg, va_list args, void *param);
		static int GetConfigPath(char *path, size_t size, const char *name);
	};

	class Audio {
		public:
		static void CreateFader(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void DestroyFader(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderAddCallback(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderRemoveCallback(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderSetDb(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderGetDb(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderSetDeflection(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderGetDeflection(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderSetMul(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderGetMul(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderAttachSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FaderDetachSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		public:
		static void CreateVolMeter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void DestroyVolMeter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterAttachSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterDetachSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterSetUpdateInterval(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterGetUpdateInterval(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterSetPeakHold(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterGetPeakHold(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterAddCallback(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void VolMeterRemoveCallback(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class AutoConfig {
		public:
		static void GetListServer(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void InitializeAutoConfig(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartBandwidthTest(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartStreamEncoderTest(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartRecordingEncoderTest(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartCheckSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartSetDefaultSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartSaveStreamSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartSaveSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void TerminateAutoConfig(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class Content {
		public:
		static void GetSourceFilterVisibility(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSourceFilterVisibility(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFader(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceVolmeter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FlipHorizontalSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FlipVerticalSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void ResetSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StretchSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FitSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CenterSceneItems(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSceneItemRotation(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSceneItemCrop(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSceneItemRotation(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSceneItemCrop(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListCurrentScenes(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListCurrentSourcesFromScene(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListInputSources(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListFilters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListCurrentTransitions(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListTransitions(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateScene(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RemoveScene(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AddSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RemoveSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFrame(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourcePropertiesSubParameters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourcePropertyCurrentValue(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetProperty(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetCurrentScene(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSourcePosition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSourceScaling(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void RenameTransition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RenameSourceFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RenameSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RenameScene(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void GetCurrentTransition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetTransitionDuration(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetTransitionDuration(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AddTransition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RemoveTransition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetTransition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void UpdateTransitionProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetTransitionProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetTransitionPropertiesSubParameters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetTransitionProperty(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetTransitionPropertyCurrentValue(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void AddSourceFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RemoveSourceFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void UpdateSourceFilterProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFilterProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetListSourceFilters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFilterPropertyCurrentValue(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSourceFilterProperty(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFilterPropertiesSubParameters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void GetSourcePosition(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceScaling(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceSize(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void SetSourceOrder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void UpdateSourceProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void CreateDisplay(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void DestroyDisplay(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetDisplayPreviewOffset(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetDisplayPreviewSize(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateSourcePreviewDisplay(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void ResizeDisplay(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void MoveDisplay(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetPaddingSize(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetPaddingColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetBackgroundColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetOutlineColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetGuidelineColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetResizeBoxOuterColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetResizeBoxInnerColor(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetShouldDrawUI(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void SelectSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SelectSources(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void DragSelectedSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void LoadConfigFile(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SaveIntoConfigFile(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceSetMuted(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void IsSourceMuted(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSourceVisibility(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSourceVisibility(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FillTabScenes(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class Service {
		public:
		static void ResetAudioContext(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void ResetVideoContext(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateAudioEncoder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateVideoStreamingEncoder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateVideoRecordingEncoder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateService(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateRecordingSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateStreamingOutput(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreateRecordingOutput(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartStreaming(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StopStreaming(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StartRecording(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void StopRecording(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AssociateAVToStreamingContext(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AssociateAVToRecordingContext(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AssociateAVEncodersToStreamingOutput(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void AssociateAVEncodersToRecordingOutput(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetServiceToStreamingOutput(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetRecordingSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void IsStreamingOutputActive(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class Settings {
		public:
		static void GetListCategories(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SaveSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};

	class Signal {
		public:
		static void SourceRemoved(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceDestroyed(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceSaved(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceLoaded(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceActivated(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SourceDeactivated(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		//static void SourceRemoved(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		//static void SourceRemoved(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};
}
