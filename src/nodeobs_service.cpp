#include "nodeobs_service.h"

#include <windows.h>
#include <ShlObj.h>

obs_output_t* streamingOutput;
obs_output_t* recordingOutput;
obs_encoder_t* audioEncoder;
obs_encoder_t* videoStreamingEncoder;
obs_encoder_t* videoRecordingEncoder;
obs_service_t* service;

std::string aacRecEncID;
std::string aacStreamEncID;

std::string videoEncoder;
std::string videoQuality;
bool usingRecordingPreset = false;
bool recordingConfigured = false;
bool ffmpegOutput = false;
bool lowCPUx264 = false;


OBS_service::OBS_service()
{

}
OBS_service::~OBS_service()
{

}

void OBS_service::OBS_service_resetAudioContext(const FunctionCallbackInfo<Value>& args)
{
	resetAudioContext();
}

void OBS_service::OBS_service_resetVideoContext(const FunctionCallbackInfo<Value>& args)
{
	resetVideoContext(NULL);
}

void OBS_service::OBS_service_createAudioEncoder(const FunctionCallbackInfo<Value>& args)
{
	createAudioEncoder();
}

void OBS_service::OBS_service_createVideoStreamingEncoder(const FunctionCallbackInfo<Value>& args)
{
	createVideoStreamingEncoder();
}

void OBS_service::OBS_service_createVideoRecordingEncoder(const FunctionCallbackInfo<Value>& args)
{
	createVideoRecordingEncoder();
}

void OBS_service::OBS_service_createService(const FunctionCallbackInfo<Value>& args)
{
	createService();
}	

void OBS_service::OBS_service_createRecordingSettings(const FunctionCallbackInfo<Value>& args)
{
	createRecordingSettings();
}

void OBS_service::OBS_service_createStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
	createStreamingOutput();
}	

void OBS_service::OBS_service_createRecordingOutput(const FunctionCallbackInfo<Value>& args)
{
	createRecordingOutput();
}	

void OBS_service::OBS_service_startStreaming(const FunctionCallbackInfo<Value>& args)
{
	startStreaming();
}

void OBS_service::OBS_service_startRecording(const FunctionCallbackInfo<Value>& args)
{
	startRecording();
}

void OBS_service::OBS_service_stopStreaming(const FunctionCallbackInfo<Value>& args)
{
	stopStreaming();
}

void OBS_service::OBS_service_stopRecording(const FunctionCallbackInfo<Value>& args)
{
	stopRecording();
}

void OBS_service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const FunctionCallbackInfo<Value>& args)
{
    associateAudioAndVideoToTheCurrentStreamingContext();
}

void OBS_service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const FunctionCallbackInfo<Value>& args)
{
    associateAudioAndVideoToTheCurrentRecordingContext();
}

void OBS_service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
}

void OBS_service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const FunctionCallbackInfo<Value>& args)
{
    associateAudioAndVideoEncodersToTheCurrentRecordingOutput();
}

void OBS_service::OBS_service_setServiceToTheStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
    setServiceToTheStreamingOutput();
}

void OBS_service::OBS_service_setRecordingSettings(const FunctionCallbackInfo<Value>& args)
{
    setRecordingSettings();
}

void OBS_service::OBS_service_isStreamingOutputActive(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    int isActive = isStreamingOutputActive();

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, to_string(isActive).c_str()));
}

void OBS_service::OBS_service_test_resetAudioContext(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    if(resetAudioContext()) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_resetVideoContext(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    if(resetVideoContext(NULL) == OBS_VIDEO_SUCCESS) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createAudioEncoder(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    createAudioEncoder();

    if(audioEncoder != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createVideoStreamingEncoder(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    createVideoStreamingEncoder();

    if(videoStreamingEncoder != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createVideoRecordingEncoder(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    createVideoRecordingEncoder();

    if(videoRecordingEncoder != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createService(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    createService();
    if(service != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createRecordingSettings(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    if(createRecordingSettings() != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    
    createStreamingOutput();

    if(streamingOutput != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_createRecordingOutput(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    createRecordingOutput();

    if(recordingOutput != NULL) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_startStreaming(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    //Create output
    createStreamingOutput();
    
    //Creating video encoder
    createVideoStreamingEncoder();
    
    //Creating audio encoder
    createAudioEncoder();
    
    //Reset audio
    resetAudioContext();
    
    //Reset video
    resetVideoContext(NULL);
    
    //Associate A/V encoders with current context
    associateAudioAndVideoToTheCurrentStreamingContext();
    
    //Load service
    createService();
    
    //Update stream output settings
    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
    
    //Assign service to an ouput
    setServiceToTheStreamingOutput();


    string result;
    if(startStreaming()) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    int timeout = 8;

    while(!obs_output_active(streamingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    stopStreaming();

    timeout = 8;
    while(obs_output_active(streamingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_startRecording(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    createRecordingOutput();
    
    //Creating video encoder
    createVideoRecordingEncoder();

	//Creating audio encoder
	createAudioEncoder();
    
    //Reset audio
    resetAudioContext();
    
    //Reset video
    resetVideoContext(NULL);
    
    //Associate A/V encoders with current context
    associateAudioAndVideoToTheCurrentRecordingContext();
    
    //Update stream output A/V encoders
    associateAudioAndVideoEncodersToTheCurrentRecordingOutput();

    //Setting recording output settings
    setRecordingSettings();

    string result;
    if(startRecording()) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    int timeout = 8;

    while(!obs_output_active(recordingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    stopRecording();

    timeout = 8;
    while(obs_output_active(recordingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_stopStreaming(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

	//Create output
    createStreamingOutput();
    
    //Creating video encoder
    createVideoStreamingEncoder();
    
    //Creating audio encoder
    createAudioEncoder();
    
    //Reset audio
    resetAudioContext();
    
    //Reset video
    resetVideoContext(NULL);
    
    //Associate A/V encoders with current context
    associateAudioAndVideoToTheCurrentStreamingContext();
    
    //Load service
    createService();
    
    //Update stream output settings
    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
    
    //Assign service to an ouput
    setServiceToTheStreamingOutput();

    startStreaming();

	int timeout = 8;

    while(!obs_output_active(streamingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    stopStreaming();

    timeout = 8;
    while(obs_output_active(streamingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    if(!obs_output_active(streamingOutput)) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_stopRecording(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    createRecordingOutput();
    
    //Creating video encoder
    createVideoRecordingEncoder();

	//Creating audio encoder
	createAudioEncoder();
    
    //Reset audio
    resetAudioContext();
    
    //Reset video
    resetVideoContext(NULL);
    
    //Associate A/V encoders with current context
    associateAudioAndVideoToTheCurrentRecordingContext();
    
    //Update stream output A/V encoders
    associateAudioAndVideoEncodersToTheCurrentRecordingOutput();

    //Setting recording output settings
    setRecordingSettings();


    startRecording();

	int timeout = 8;

    while(!obs_output_active(recordingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    stopRecording();

    timeout = 8;
    while(obs_output_active(recordingOutput)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//usleep(500000);
		timeout--;
		if(timeout == 0){
			result = "FAILURE";
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		}
    }

    if(!obs_output_active(recordingOutput)) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_associateAudioAndVideoToTheCurrentStreamingContext(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    associateAudioAndVideoToTheCurrentStreamingContext();

    video_t* video = obs_encoder_video(videoStreamingEncoder);
    audio_t* audio = obs_encoder_audio(audioEncoder);

    if(video == obs_get_video() && audio == obs_get_audio()) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_associateAudioAndVideoToTheCurrentRecordingContext(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    associateAudioAndVideoToTheCurrentRecordingContext();

    video_t* video = obs_encoder_video(videoRecordingEncoder);
    audio_t* audio = obs_encoder_audio(audioEncoder);

    if(video == obs_get_video() && audio == obs_get_audio()) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();

    obs_encoder_t* currentVideoEncoder = obs_output_get_video_encoder(streamingOutput);
    obs_encoder_t* currentAudioEncoder = obs_output_get_audio_encoder(streamingOutput, 0);

    if(currentVideoEncoder == videoStreamingEncoder && currentAudioEncoder == audioEncoder) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    associateAudioAndVideoEncodersToTheCurrentRecordingOutput();

    obs_encoder_t* currentVideoEncoder = obs_output_get_video_encoder(recordingOutput);
    obs_encoder_t* currentAudioEncoder = obs_output_get_audio_encoder(recordingOutput, 0);

    if(currentVideoEncoder == videoRecordingEncoder && currentAudioEncoder == audioEncoder) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_setServiceToTheStreamingOutput(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    setServiceToTheStreamingOutput();
    obs_service_t* currentService = obs_output_get_service(streamingOutput);

    if(currentService == service) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";   
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_service::OBS_service_test_setRecordingSettings(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;

    setRecordingSettings();
    
    obs_data_t* currentSettings = obs_output_get_settings(recordingOutput);

    if(currentSettings != NULL) {
        result = "SUCCESS";
    }
    else {
        result = "FAILURE";
    }
    
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void LoadAudioDevice(const char *name, int channel, obs_data_t *parent)
{
    obs_data_t *data = obs_data_get_obj(parent, name);
    if (!data)
        return;
    
    obs_source_t *source = obs_load_source(data);
    if (source) {
        obs_set_output_source(channel, source);
        obs_source_release(source);
    }
    
    obs_data_release(data);
}

bool OBS_service::resetAudioContext(void)
{
    struct obs_audio_info ai;
    
    ai.samples_per_sec = 44100;
    ai.speakers = SPEAKERS_STEREO;
    
    // const char* file = "./config/Untitled.json";
    // obs_data_t *data = obs_data_create_from_json_file_safe(file, "bak");

    // //Loading all audio devices
    // LoadAudioDevice("DesktopAudioDevice1", 1, data);
    // LoadAudioDevice("DesktopAudioDevice2", 2, data);
    // LoadAudioDevice("AuxAudioDevice1", 3, data);
    // LoadAudioDevice("AuxAudioDevice2", 4, data);
    // LoadAudioDevice("AuxAudioDevice3", 5, data);

    return obs_reset_audio(&ai);
}

static inline enum video_format GetVideoFormatFromName(const char *name)
{
    if(name != NULL) 
    {
        if (astrcmpi(name, "I420") == 0)
            return VIDEO_FORMAT_I420;
        else if (astrcmpi(name, "NV12") == 0)
            return VIDEO_FORMAT_NV12;
        else if (astrcmpi(name, "I444") == 0)
            return VIDEO_FORMAT_I444;
    #if 0 //currently unsupported
        else if (astrcmpi(name, "YVYU") == 0)
            return VIDEO_FORMAT_YVYU;
        else if (astrcmpi(name, "YUY2") == 0)
            return VIDEO_FORMAT_YUY2;
        else if (astrcmpi(name, "UYVY") == 0)
            return VIDEO_FORMAT_UYVY;
    #endif
        else
            return VIDEO_FORMAT_RGBA;      
    } else {
        return VIDEO_FORMAT_I420;
    }

}

static inline enum obs_scale_type GetScaleType(config_t* config)
{
    const char *scaleTypeStr = config_get_string(config, "Video", "ScaleType");

    if(scaleTypeStr != NULL) 
    {
        if (astrcmpi(scaleTypeStr, "bilinear") == 0)
            return OBS_SCALE_BILINEAR;
        else if (astrcmpi(scaleTypeStr, "lanczos") == 0)
            return OBS_SCALE_LANCZOS;
        else
            return OBS_SCALE_BICUBIC; 
    } else 
    {
        return OBS_SCALE_BICUBIC;
    }


}

static inline const char *GetRenderModule(config_t* config) 
{
    const char* renderer = config_get_string(config, "Video", "Renderer");

    const char* DL_D3D11 = "libobs-d3d11.dll";
    const char* DL_OPENGL;

    #ifdef _WIN32
        DL_OPENGL = "libobs-opengl.dll";
    #else
        DL_OPENGL = "libobs-opengl.so";
    #endif

    if(renderer != NULL) 
    {
        return (astrcmpi(renderer, "Direct3D 11") == 0) ? DL_D3D11 : DL_OPENGL;    
    } else {
        return DL_D3D11;
    }   
}

void GetFPSInteger(config_t* basicConfig, uint32_t &num, uint32_t &den)
{
    num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSInt");
    den = 1;
}

void GetFPSFraction(config_t* basicConfig, uint32_t &num, uint32_t &den) 
{
    num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNum");
    den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSDen");
}

void GetFPSNanoseconds(config_t* basicConfig, uint32_t &num, uint32_t &den) 
{
    num = 1000000000;
    den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNS");
}

void GetFPSCommon(config_t* basicConfig, uint32_t &num, uint32_t &den) 
{
    const char *val = config_get_string(basicConfig, "Video", "FPSCommon");
    if(val != NULL) {
        if (strcmp(val, "10") == 0) {
            num = 10;
            den = 1;
        } else if (strcmp(val, "20") == 0) {
            num = 20;
            den = 1;
        } else if (strcmp(val, "24 NTSC") == 0) {
            num = 24000;
            den = 1001;
        } else if (strcmp(val, "25") == 0) {
            num = 25;
            den = 1;
        } else if (strcmp(val, "29.97") == 0) {
            num = 30000;
            den = 1001;
        } else if (strcmp(val, "48") == 0) {
            num = 48;
            den = 1;
        } else if (strcmp(val, "59.94") == 0) {
            num = 60000;
            den = 1001;
        } else if (strcmp(val, "60") == 0) {
            num = 60;
            den = 1;
        } else {
            num = 30;
            den = 1;
        }
    } else {
        num = 30;
        den = 1; 
        config_set_uint(basicConfig, "Video", "FPSType", 0);
        config_set_string(basicConfig, "Video", "FPSCommon", "30");
        config_save_safe(basicConfig, "tmp", nullptr);   
    }
}

void GetConfigFPS(config_t* basicConfig, uint32_t &num, uint32_t &den) 
{
    uint32_t type = config_get_uint(basicConfig, "Video", "FPSType");
    if (type == 1) //"Integer"
        GetFPSInteger(basicConfig, num, den);
    else if (type == 2) //"Fraction"
        GetFPSFraction(basicConfig, num, den);
    else if (false) //"Nanoseconds", currently not implemented
        GetFPSNanoseconds(basicConfig, num, den);
    else
        GetFPSCommon(basicConfig, num, den);
}

/* some nice default output resolution vals */
static const double vals[] =
{
    1.0,
    1.25,
    (1.0/0.75),
    1.5,
    (1.0/0.6),
    1.75,
    2.0,
    2.25,
    2.5,
    2.75,
    3.0
};

static const size_t numVals = sizeof(vals)/sizeof(double);

bool OBS_service::resetVideoContext(const char* outputType)
{
	#ifdef _WIN32
	#define PATH_DELIM "\\"
	#else
	#define PATH_DELIM "/"
	#endif

	// Retrieve current directory. TODO: Replace with proper initialization function so that CWD changes do not mess us up.
	std::string currentDirectory;
	{
		char* buffer = getcwd(nullptr, 1);
		currentDirectory = std::string(buffer);
		free(buffer);
	}

    config_t* basicConfig = OBS_API::openConfigFile(OBS_API::getBasicConfigPath());

	obs_video_info ovi;
	std::string gslib = "";
    #ifdef _WIN32
    gslib = std::string(currentDirectory + PATH_DELIM + "node-obs" + PATH_DELIM + GetRenderModule(basicConfig)).c_str();
    #else
	gslib = std::string(currentDirectory + PATH_DELIM + "node-obs" + PATH_DELIM + "libobs-opengl.dll").c_str();
    #endif
    ovi.graphics_module = gslib.c_str();

    ovi.base_width     = (uint32_t)config_get_uint(basicConfig, "Video", "BaseCX");
    ovi.base_height    = (uint32_t)config_get_uint(basicConfig, "Video", "BaseCY");

    const char* outputMode = config_get_string(basicConfig, "Output", "Mode");

	if (outputMode == NULL) {
		outputMode = "Simple";
    }

    ovi.output_width   = (uint32_t)config_get_uint(basicConfig, "Video", "OutputCX");
    ovi.output_height  = (uint32_t)config_get_uint(basicConfig, "Video", "OutputCY");

    if(strcmp(outputMode, "Advanced") == 0 && outputType != NULL) {
        if(strcmp(outputType, "Stream") == 0) {
            bool doRescale = config_get_bool(basicConfig, "AdvOut", "Rescale");
            if(doRescale) {
                const char* rescaleRes = config_get_string(basicConfig, "AdvOut", "RescaleRes");
                if (rescaleRes == NULL) {
                    rescaleRes = "1280x720";
                }
                sscanf(rescaleRes, "%ux%u", &ovi.output_width, &ovi.output_height);
            }
        } else if (strcmp(outputType, "Record") == 0) {
            bool doRescale = config_get_bool(basicConfig, "AdvOut", "RecRescale");
            if(doRescale) {
                const char* recRescaleRes = config_get_string(basicConfig, "AdvOut", "RecRescaleRes");
                if (recRescaleRes == NULL) {
                    recRescaleRes = "1280x720";
                }
                sscanf(recRescaleRes, "%ux%u", &ovi.output_width, &ovi.output_height);
            }
        }
    }

	std::vector<Screen> resolutions = OBS_API::availableResolutions();

    if (ovi.base_width == 0 || ovi.base_height == 0) {
		for (int i = 0; i<resolutions.size(); i++) {
			if (ovi.base_width * ovi.base_height < resolutions.at(i).width * resolutions.at(i).height) {
				ovi.base_width = resolutions.at(i).width;
				ovi.base_height = resolutions.at(i).height;
			}
		}
    } 

    config_set_uint(basicConfig, "Video", "BaseCX", ovi.base_width);
    config_set_uint(basicConfig, "Video", "BaseCY", ovi.base_height);

    if (ovi.output_width == 0 || ovi.output_height == 0) {
        if(ovi.base_width > 1280 && ovi.base_height > 720) {
            int idx = 0;
            do {
                ovi.output_width =  uint32_t(double(ovi.base_width) / vals[idx]);
                ovi.output_height = uint32_t(double(ovi.base_height) / vals[idx]);   
                idx++; 
            }
            while(ovi.output_width > 1280 && ovi.output_height > 720);
        } else {
            ovi.output_width = ovi.base_width;
            ovi.output_height = ovi.base_height;
        }

        config_set_uint(basicConfig, "Video", "OutputCX", ovi.output_width);
        config_set_uint(basicConfig, "Video", "OutputCY", ovi.output_height);
    }

    GetConfigFPS(basicConfig, ovi.fps_num, ovi.fps_den);

    const char *colorFormat = config_get_string(basicConfig, "Video","ColorFormat");
    const char *colorSpace = config_get_string(basicConfig, "Video", "ColorSpace");
    const char *colorRange = config_get_string(basicConfig, "Video","ColorRange");

    ovi.output_format = GetVideoFormatFromName(colorFormat);

    ovi.adapter = 0;
    ovi.gpu_conversion = true;

    ovi.colorspace     = astrcmpi(colorSpace, "601") == 0 ? VIDEO_CS_601 : VIDEO_CS_709;
    ovi.range          = astrcmpi(colorRange, "Full") == 0 ? VIDEO_RANGE_FULL : VIDEO_RANGE_PARTIAL;

    ovi.scale_type = GetScaleType(basicConfig);

    config_save_safe(basicConfig, "tmp", nullptr);

    return obs_reset_video(&ovi);
}

const char *FindAudioEncoderFromCodec(const char *type)
{
	const char *alt_enc_id = nullptr;
	size_t i = 0;

	while (obs_enum_encoder_types(i++, &alt_enc_id)) {
		const char *codec = obs_get_encoder_codec(alt_enc_id);
		if (strcmp(type, codec) == 0) {
			return alt_enc_id;
		}
	}

	return nullptr;
}

void OBS_service::createAudioEncoder(void)
{
    config_t* basicConfig = OBS_API::openConfigFile(OBS_API::getBasicConfigPath());

     int bitrate = FindClosestAvailableAACBitrate((int)
        config_get_uint(basicConfig, "SimpleOutput", "ABitrate"));

	const char *id = GetAACEncoderForBitrate(bitrate);
	if (!id) {
        audioEncoder = nullptr;
        return;
	}

	audioEncoder = obs_audio_encoder_create(id, "simple_audio", nullptr, 0, nullptr);
}


void OBS_service::createVideoStreamingEncoder()
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config;
    int result = config_open(&config, basicConfigFile.c_str(), CONFIG_OPEN_EXISTING);

    const char *encoder = config_get_string(config, "SimpleOutput", "StreamEncoder");

    if(encoder == NULL) {
        encoder = "obs_x264";
    }

    if(videoStreamingEncoder != NULL) {
        obs_encoder_release(videoStreamingEncoder);
    }

    videoStreamingEncoder = obs_video_encoder_create(encoder, "streaming_h264", nullptr, nullptr);

    if(result != CONFIG_SUCCESS) {
        obs_data_t *h264Settings = obs_data_create();
        config = config_create(basicConfigFile.c_str());
        h264Settings = obs_encoder_defaults("obs_x264");
        config_set_uint(config, "SimpleOutput", "VBitrate", 2500);
        config_save_safe(config, "tmp", nullptr);  
        obs_data_release(h264Settings);
    } else {
        updateVideoStreamingEncoder();
    } 
}

static inline bool valid_string(const char *str)
{
    while (str && *str) {
        if (*(str++) != ' ')
            return true;
    }

    return false;
}
static void replace_text(struct dstr *str, size_t pos, size_t len,
        const char *new_text)
{
    struct dstr front = {0};
    struct dstr back = {0};

    dstr_left(&front, str, pos);
    dstr_right(&back, str, pos + len);
    dstr_copy_dstr(str, &front);
    dstr_cat(str, new_text);
    dstr_cat_dstr(str, &back);
    dstr_free(&front);
    dstr_free(&back);
}

static void erase_ch(struct dstr *str, size_t pos)
{
    struct dstr new_str = {0};
    dstr_left(&new_str, str, pos);
    dstr_cat(&new_str, str->array + pos + 1);
    dstr_free(str);
    *str = new_str;
}

char *os_generate_formatted_filename(const char *extension, bool space,
        const char *format)
{
    time_t now = time(0);
    struct tm *cur_time;
    cur_time = localtime(&now);

    const size_t spec_count = 23;
    static const char *spec[][2] = {
        {"%CCYY", "%Y"},
        {"%YY",   "%y"},
        {"%MM",   "%m"},
        {"%DD",   "%d"},
        {"%hh",   "%H"},
        {"%mm",   "%M"},
        {"%ss",   "%S"},
        {"%%",    "%%"},

        {"%a",    ""},
        {"%A",    ""},
        {"%b",    ""},
        {"%B",    ""},
        {"%d",    ""},
        {"%H",    ""},
        {"%I",    ""},
        {"%m",    ""},
        {"%M",    ""},
        {"%p",    ""},
        {"%S",    ""},
        {"%y",    ""},
        {"%Y",    ""},
        {"%z",    ""},
        {"%Z",    ""},
    };

    char convert[128] = {0};
    struct dstr sf;
    struct dstr c = {0};
    size_t pos = 0;

    dstr_init_copy(&sf, format);

    while (pos < sf.len) {
        for (size_t i = 0; i < spec_count && !convert[0]; i++) {
            size_t len = strlen(spec[i][0]);

            const char *cmp = sf.array + pos;

            if (astrcmp_n(cmp, spec[i][0], len) == 0) {
                if (strlen(spec[i][1]))
                    strftime(convert, sizeof(convert),
                            spec[i][1], cur_time);
                else
                    strftime(convert, sizeof(convert),
                            spec[i][0], cur_time);


                dstr_copy(&c, convert);
                if (c.len && valid_string(c.array))
                    replace_text(&sf, pos, len, convert);
            }
        }

        if (convert[0]) {
            pos += strlen(convert);
            convert[0] = 0;
        } else if (!convert[0] && sf.array[pos] == '%') {
            erase_ch(&sf, pos);
        } else {
            pos++;
        }
    }

    if (!space)
        dstr_replace(&sf, " ", "_");

    dstr_cat_ch(&sf, '.');
    dstr_cat(&sf, extension);
    dstr_free(&c);

    if (sf.len > 255)
        dstr_mid(&sf, &sf, 0, 255);

    return sf.array;
}

std::string GenerateSpecifiedFilename(const char *extension, bool noSpace,
        const char *format)
{
    BPtr<char> filename = os_generate_formatted_filename(extension,
            !noSpace, format);
    return string(filename);
}

static void ensure_directory_exists(string &path)
{
    replace(path.begin(), path.end(), '\\', '/');

    size_t last = path.rfind('/');
    if (last == string::npos)
        return;

    string directory = path.substr(0, last);
    os_mkdirs(directory.c_str());
}

static void FindBestFilename(string &strPath, bool noSpace)
{
    int num = 2;

    if (!os_file_exists(strPath.c_str()))
        return;

    const char *ext = strrchr(strPath.c_str(), '.');
    if (!ext)
        return;

    int extStart = int(ext - strPath.c_str());
    for (;;) {
        string testPath = strPath;
        string numStr;

        numStr = noSpace ? "_" : " (";
        numStr += to_string(num++);
        if (!noSpace)
            numStr += ")";

        testPath.insert(extStart, numStr);

        if (!os_file_exists(testPath.c_str())) {
            strPath = testPath;
            break;
        }
    }
}

static void remove_reserved_file_characters(string &s)
{
    replace(s.begin(), s.end(), '/', '_');
    replace(s.begin(), s.end(), '\\', '_');
    replace(s.begin(), s.end(), '*', '_');
    replace(s.begin(), s.end(), '?', '_');
    replace(s.begin(), s.end(), '"', '_');
    replace(s.begin(), s.end(), '|', '_');
    replace(s.begin(), s.end(), ':', '_');
    replace(s.begin(), s.end(), '>', '_');
    replace(s.begin(), s.end(), '<', '_');
}

void OBS_service::createVideoRecordingEncoder()
{
    if(videoRecordingEncoder != NULL) {
        obs_encoder_release(videoRecordingEncoder);
    }
    videoRecordingEncoder = obs_video_encoder_create("obs_x264", "simple_h264_recording", nullptr, nullptr);
}

void OBS_service::createService()
{
    const char *type;

    struct stat buffer;   
    std::string serviceConfigFile = OBS_API::getServiceConfigPath();
    bool fileExist = (stat (serviceConfigFile.c_str(), &buffer) == 0); 

    obs_data_t *data;
    obs_data_t *settings;
    obs_data_t *hotkey_data;

    if(!fileExist) {
        service = obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
        data     = obs_data_create();
        settings = obs_service_get_settings(service);

        obs_data_set_string(settings, "streamType", "rtmp_common");
        obs_data_set_string(settings, "service", "Twitch");
        obs_data_set_bool(settings, "show_all", 0);
        obs_data_set_string(settings, "server", "rtmp://live.twitch.tv/app");
        obs_data_set_string(settings, "key", "");

        obs_data_set_string(data, "type", obs_service_get_type(service));
        obs_data_set_obj(data, "settings", settings);

    } else {
        data = obs_data_create_from_json_file_safe(serviceConfigFile.c_str(), "bak");

        obs_data_set_default_string(data, "type", "rtmp_common");
        type = obs_data_get_string(data, "type");
        
        settings = obs_data_get_obj(data, "settings");
        hotkey_data = obs_data_get_obj(data, "hotkeys");
        
        service = obs_service_create(type, "default_service", settings, hotkey_data);


        obs_data_release(hotkey_data);
    }

    if (!obs_data_save_json_safe(data, serviceConfigFile.c_str(), "tmp", "bak")) {
        blog(LOG_WARNING, "Failed to save service %s", serviceConfigFile.c_str());
    }

    obs_data_release(settings);
    obs_data_release(data);
}

obs_data_t* OBS_service::createRecordingSettings(void)
{
	obs_data_t *settings = obs_data_create();
    /*obs_data_set_string(settings, "format_name", "avi");
    obs_data_set_string(settings, "video_encoder", "utvideo");
    obs_data_set_string(settings, "audio_encoder", "pcm_s16le");
    obs_data_set_string(settings, "path", "./recording_1.avi");*/
    // obs_data_t *settings = obs_encoder_get_settings(videoRecordingEncoder);


    return settings;
}

void OBS_service::createStreamingOutput(void)
{
    streamingOutput = obs_output_create("rtmp_output", "simple_stream", nullptr, nullptr);
}

void OBS_service::createRecordingOutput(void)
{
	recordingOutput = obs_output_create("ffmpeg_muxer", "simple_file_output", nullptr, nullptr);
    // updateRecordingOutput();
}

bool OBS_service::startStreaming(void)
{
    updateService();
    updateStreamSettings();

    return obs_output_start(streamingOutput);
}

bool OBS_service::startRecording(void)
{
    updateRecordSettings();

	return obs_output_start(recordingOutput);
}

void OBS_service::stopStreaming(void)
{
	obs_output_stop(streamingOutput);
}

void OBS_service::stopRecording(void)
{
	obs_output_stop(recordingOutput);
}

void OBS_service::associateAudioAndVideoToTheCurrentStreamingContext(void)
{
    obs_encoder_set_video(videoStreamingEncoder, obs_get_video());
    obs_encoder_set_audio(audioEncoder, obs_get_audio());
}

void OBS_service::associateAudioAndVideoToTheCurrentRecordingContext(void)
{
    obs_encoder_set_video(videoRecordingEncoder, obs_get_video());
    obs_encoder_set_audio(audioEncoder, obs_get_audio());
}

void OBS_service::associateAudioAndVideoEncodersToTheCurrentStreamingOutput(void)
{
    obs_output_set_video_encoder(streamingOutput, videoStreamingEncoder);
    obs_output_set_audio_encoder(streamingOutput, audioEncoder, 0);
}

void OBS_service::associateAudioAndVideoEncodersToTheCurrentRecordingOutput(void)
{
    obs_output_set_video_encoder(recordingOutput, videoRecordingEncoder);
    obs_output_set_audio_encoder(recordingOutput, audioEncoder, 0);
}

void OBS_service::setServiceToTheStreamingOutput(void)
{
    obs_output_set_service(streamingOutput, service);
}

void OBS_service::setRecordingSettings(void)
{
    /* obs_data_t *settings = createRecordingSettings();
    obs_output_update(recordingOutput, settings);
    obs_data_release(settings); */
}

obs_service_t* OBS_service::getService(void)
{
    const char* serviceType = obs_service_get_type(service);
    return service;
}

void OBS_service::setService(obs_service_t* newService)
{
    obs_service_release(service);
    service = newService;
}

void OBS_service::saveService(void) 
{
    if (!service)
        return;

    obs_data_t *data     = obs_data_create();
    obs_data_t *settings = obs_service_get_settings(service);

    const char* serviceType = obs_service_get_type(service);

    obs_data_set_string(data, "type", obs_service_get_type(service));
    obs_data_set_obj(data, "settings", settings);

    if (!obs_data_save_json_safe(data, OBS_API::getServiceConfigPath().c_str(), "tmp", "bak"))
        blog(LOG_WARNING, "Failed to save service");

    obs_service_update(service, settings);

	serviceType = obs_service_get_type(service);

    // obs_data_release(settings);
    // obs_data_release(data);
}

bool OBS_service::isStreamingOutputActive(void)
{
    return obs_output_active(streamingOutput);
}

int GetAudioBitrate()
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    int bitrate = (int)config_get_uint(config, "SimpleOutput",
            "ABitrate");

    return FindClosestAvailableAACBitrate(bitrate);
}

void OBS_service::updateVideoStreamingEncoder()
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    obs_data_t *h264Settings = obs_data_create();
    obs_data_t *aacSettings  = obs_data_create();

    int videoBitrate = config_get_uint(config, "SimpleOutput","VBitrate");
    int audioBitrate = GetAudioBitrate();
    bool advanced = config_get_bool(config, "SimpleOutput","UseAdvanced");
    bool enforceBitrate = config_get_bool(config, "SimpleOutput", "EnforceBitrate");
    const char *custom = config_get_string(config,"SimpleOutput", "x264Settings");
    const char *encoder = config_get_string(config, "SimpleOutput", "StreamEncoder");
	const char *encoderID;
    const char *presetType;
    const char *preset;

    if(encoder != NULL)
    {
        if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0) {
            presetType = "QSVPreset";
			encoderID = "obs_qsv11";
        } else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0) {
            presetType = "AMDPreset";
            UpdateStreamingSettings_amd(h264Settings, videoBitrate);
			encoderID = "amd_amf_h264";
        } else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0) {
            presetType = "NVENCPreset";
			encoderID = "ffmpeg_nvenc";
        } else {
            presetType = "Preset";
			encoderID = "obs_x264";
        }
        preset = config_get_string(config, "SimpleOutput", presetType);

		if(strcmp(obs_encoder_get_id(videoStreamingEncoder), encoderID) != 0) {
            if(videoStreamingEncoder != NULL) {
                obs_encoder_release(videoStreamingEncoder);
            }
            videoStreamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", nullptr, nullptr);
        }
    }

    if(videoBitrate == 0) {
        videoBitrate = 2500;
        config_set_uint(config, "SimpleOutput","VBitrate", videoBitrate);
        config_save_safe(config, "tmp", nullptr);
    }

    obs_data_set_string(h264Settings, "rate_control", "CBR");
    obs_data_set_int(h264Settings, "bitrate", videoBitrate);

    if (advanced) {
        obs_data_set_string(h264Settings, "preset", preset);
        obs_data_set_string(h264Settings, "x264opts", custom);
    }

    obs_data_set_string(aacSettings, "rate_control", "CBR");
    obs_data_set_int(aacSettings, "bitrate", audioBitrate);

    const char* url = obs_service_get_url(service);

    obs_service_apply_encoder_settings(service, h264Settings, aacSettings);

    if (advanced && !enforceBitrate) {
        obs_data_set_int(h264Settings, "bitrate", videoBitrate);
        obs_data_set_int(aacSettings, "bitrate", audioBitrate);
    }

    video_t *video = obs_get_video();
    enum video_format format = video_output_get_format(video);

    if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420)
        obs_encoder_set_preferred_video_format(videoStreamingEncoder,
                VIDEO_FORMAT_NV12);

    obs_encoder_update(videoStreamingEncoder, h264Settings);
    obs_encoder_update(audioEncoder,  aacSettings);

    obs_data_release(h264Settings);
    obs_data_release(aacSettings);
}


std::string OBS_service::GetDefaultVideoSavePath(void)
{
    wchar_t path_utf16[MAX_PATH];
    char    path_utf8[MAX_PATH] = {};

    SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT,
        path_utf16);

    os_wcs_to_utf8(path_utf16, wcslen(path_utf16), path_utf8, MAX_PATH);
    return std::string(path_utf8);
}

void OBS_service::updateService(void)
{
    setServiceToTheStreamingOutput();
}

void OBS_service::updateStreamingOutput(void)
{
    updateVideoStreamingEncoder();

    associateAudioAndVideoToTheCurrentStreamingContext();
    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
}

void OBS_service::updateRecordingOutput(void)
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    const char *path = config_get_string(config, "SimpleOutput", "FilePath");
    const char *format = config_get_string(config, "SimpleOutput", "RecFormat");
    const char *mux = config_get_string(config, "SimpleOutput", "MuxerCustom");
    bool noSpace = config_get_bool(config, "SimpleOutput", "FileNameWithoutSpace");
    const char *filenameFormat = config_get_string(config, "Output", "FilenameFormatting");
    bool overwriteIfExists = config_get_bool(config, "Output", "OverwriteIfExists");
    const char *rbPrefix = config_get_string(config, "SimpleOutput", "RecRBPrefix");
    const char *rbSuffix = config_get_string(config, "SimpleOutput", "RecRBSuffix");
    int rbTime = config_get_int(config, "SimpleOutput", "RecRBTime");
    int rbSize = config_get_int(config, "SimpleOutput", "RecRBSize");

	os_dir_t *dir = path && path[0] ? os_opendir(path) : nullptr;

    if(filenameFormat == NULL) {
        filenameFormat = "%CCYY-%MM-%DD %hh-%mm-%ss";
    } 
    string strPath;
    strPath += path;


    char lastChar = strPath.back();
    if (lastChar != '/' && lastChar != '\\')
        strPath += "/";

    bool ffmpegOutput = false;
    bool usingRecordingPreset = true;

    if(filenameFormat != NULL && format != NULL) {
        strPath += GenerateSpecifiedFilename(ffmpegOutput ? "avi" : format, noSpace, filenameFormat);
        if(!strPath.empty())
            ensure_directory_exists(strPath);
    }
    if (!overwriteIfExists)
        FindBestFilename(strPath, noSpace);

	obs_data_t *settings = obs_data_create();
    bool useReplayBuffer = config_get_bool(config, "SimpleOutput", "RecRB");

    if (useReplayBuffer) {
        string f;

        if (rbPrefix && *rbPrefix) {
            f += rbPrefix;
            if (f.back() != ' ')
                f += " ";
        }

        f += filenameFormat;

        if (rbSuffix && *rbSuffix) {
            if (*rbSuffix != ' ')
                f += " ";
            f += rbSuffix;
        }

        remove_reserved_file_characters(f);

        obs_data_set_string(settings, "directory", path);
        obs_data_set_string(settings, "extension", format);
        obs_data_set_int(settings, "max_time_sec", rbTime);
        obs_data_set_int(settings, "max_size_mb", usingRecordingPreset ? rbSize : 0);
    } else {
        obs_data_set_string(settings, ffmpegOutput ? "url" : "path", strPath.c_str());
    }

    obs_output_update(recordingOutput, settings);
}

void OBS_service::updateAdvancedRecordingOutput(void)
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

	const char *path = config_get_string(config, "AdvOut",
			"RecFilePath");
	const char *mux = config_get_string(config, "AdvOut",
			"RecMuxerCustom");
	bool rescale = config_get_bool(config, "AdvOut",
			"RecRescale");
	const char *rescaleRes = config_get_string(config, "AdvOut",
			"RecRescaleRes");
    int tracks = config_get_int(config, "AdvOut", "RecTracks");

	const char *recFormat;
	const char *filenameFormat;
	bool noSpace = false;
	bool overwriteIfExists = false;

    recFormat = config_get_string(config, "AdvOut", "RecFormat");
    filenameFormat = config_get_string(config, "Output", "FilenameFormatting");
    overwriteIfExists = config_get_bool(config, "Output", "OverwriteIfExists");
    noSpace = config_get_bool(config, "AdvOut", "RecFileNameWithoutSpace");

    os_dir_t *dir = path && path[0] ? os_opendir(path) : nullptr;

    os_closedir(dir);

    string strPath;
    strPath += path;

    char lastChar = strPath.back();
    if (lastChar != '/' && lastChar != '\\')
        strPath += "/";

    strPath += GenerateSpecifiedFilename(recFormat, noSpace,
                        filenameFormat);
    ensure_directory_exists(strPath);
    if (!overwriteIfExists)
        FindBestFilename(strPath, noSpace);


	obs_data_t *settings = obs_data_create();
	unsigned int cx = 0;
	unsigned int cy = 0;
    int idx = 0;

    // To be changed to the actual value
    bool useStreamEncoder = false;

	if (useStreamEncoder) {
		obs_output_set_video_encoder(recordingOutput, videoStreamingEncoder);
	} else {
		if (rescale && rescaleRes && *rescaleRes) {
			if (sscanf(rescaleRes, "%ux%u", &cx, &cy) != 2) {
				cx = 0;
				cy = 0;
			}
		}

		obs_encoder_set_scaled_size(videoRecordingEncoder, cx, cy);
		obs_encoder_set_video(videoRecordingEncoder, obs_get_video());
		obs_output_set_video_encoder(recordingOutput, videoRecordingEncoder);
	}

	// for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
	// 	if ((tracks & (1<<i)) != 0) {
	// 		obs_output_set_audio_encoder(recordingOutput, aacTrack[i],
	// 				idx++);
	// 	}
	// }

	obs_data_set_string(settings, "path", path);
	obs_data_set_string(settings, "muxer_settings", mux);
	obs_output_update(recordingOutput, settings);
	// obs_data_release(settings);
}

void OBS_service::LoadRecordingPreset_Lossless()
{
    if(recordingOutput != NULL) {
        obs_output_release(recordingOutput);
    }
	recordingOutput = obs_output_create("ffmpeg_output",
			"simple_ffmpeg_output", nullptr, nullptr);
	if (!recordingOutput)
		throw "Failed to create recording FFmpeg output "
		      "(simple output)";
	// obs_output_release(recordingOutput);

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "format_name", "avi");
	obs_data_set_string(settings, "video_encoder", "utvideo");
	obs_data_set_string(settings, "audio_encoder", "pcm_s16le");

	obs_output_update(recordingOutput, settings);
	// obs_data_release(settings);
}

void OBS_service::LoadRecordingPreset_h264(const char *encoderId)
{
    if(videoRecordingEncoder != NULL) {
        obs_encoder_release(videoRecordingEncoder);
    }
	videoRecordingEncoder = obs_video_encoder_create(encoderId,
			"simple_h264_recording", nullptr, nullptr);
	if (!videoRecordingEncoder)
		throw "Failed to create h264 recording encoder (simple output)";
	// obs_encoder_release(videoRecordingEncoder);
}

static bool update_ffmpeg_output(config_t* config)
{
	if (config_has_user_value(config, "AdvOut", "FFOutputToFile"))
		return false;

	const char *url = config_get_string(config, "AdvOut", "FFURL");
	if (!url)
		return false;

	bool isActualURL = strstr(url, "://") != nullptr;
	if (isActualURL)
		return false;

	string urlStr = url;
	string extension;

	for (size_t i = urlStr.length(); i > 0; i--) {
		size_t idx = i - 1;

		if (urlStr[idx] == '.') {
			extension = &urlStr[i];
		}

		if (urlStr[idx] == '\\' || urlStr[idx] == '/') {
			urlStr[idx] = 0;
			break;
		}
	}

	if (urlStr.empty() || extension.empty())
		return false;

	config_remove_value(config, "AdvOut", "FFURL");
	config_set_string(config, "AdvOut", "FFFilePath", urlStr.c_str());
	config_set_string(config, "AdvOut", "FFExtension", extension.c_str());
	config_set_bool(config, "AdvOut", "FFOutputToFile", true);
	return true;
}

void OBS_service::UpdateFFmpegOutput(void)
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    update_ffmpeg_output(config);

    if(recordingOutput != NULL) {
        obs_output_release(recordingOutput);
    }
	recordingOutput = obs_output_create("ffmpeg_output",
			"simple_ffmpeg_output", nullptr, nullptr);

	const char *url = config_get_string(config, "AdvOut", "FFURL");
	int vBitrate = config_get_int(config, "AdvOut",
			"FFVBitrate");
	int gopSize = config_get_int(config, "AdvOut",
			"FFVGOPSize");
	bool rescale = config_get_bool(config, "AdvOut",
			"FFRescale");
	const char *rescaleRes = config_get_string(config, "AdvOut",
			"FFRescaleRes");
	const char *formatName = config_get_string(config, "AdvOut",
			"FFFormat");
	const char *mimeType = config_get_string(config, "AdvOut",
			"FFFormatMimeType");
	const char *muxCustom = config_get_string(config, "AdvOut",
			"FFMCustom");
	const char *vEncoder = config_get_string(config, "AdvOut",
			"FFVEncoder");
	int vEncoderId = config_get_int(config, "AdvOut",
			"FFVEncoderId");
	const char *vEncCustom = config_get_string(config, "AdvOut",
			"FFVCustom");
	int aBitrate = config_get_int(config, "AdvOut",
			"FFABitrate");
	int aTrack = config_get_int(config, "AdvOut",
			"FFAudioTrack");
	const char *aEncoder = config_get_string(config, "AdvOut",
			"FFAEncoder");
	int aEncoderId = config_get_int(config, "AdvOut",
			"FFAEncoderId");
	const char *aEncCustom = config_get_string(config, "AdvOut",
			"FFACustom");
	obs_data_t *settings = obs_data_create();

	obs_data_set_string(settings, "url", url);
	obs_data_set_string(settings, "format_name", formatName);
	obs_data_set_string(settings, "format_mime_type", mimeType);
	obs_data_set_string(settings, "muxer_settings", muxCustom);
	obs_data_set_int(settings, "gop_size", gopSize);
	obs_data_set_int(settings, "video_bitrate", vBitrate);
	obs_data_set_string(settings, "video_encoder", vEncoder);
	obs_data_set_int(settings, "video_encoder_id", vEncoderId);
	obs_data_set_string(settings, "video_settings", vEncCustom);
	obs_data_set_int(settings, "audio_bitrate", aBitrate);
	obs_data_set_string(settings, "audio_encoder", aEncoder);
	obs_data_set_int(settings, "audio_encoder_id", aEncoderId);
	obs_data_set_string(settings, "audio_settings", aEncCustom);

	if (rescale && rescaleRes && *rescaleRes) {
		int width;
		int height;
		int val = sscanf(rescaleRes, "%dx%d", &width, &height);

		if (val == 2 && width && height) {
			obs_data_set_int(settings, "scale_width", width);
			obs_data_set_int(settings, "scale_height", height);
		}
	}

	obs_output_set_mixer(recordingOutput, aTrack - 1);
	obs_output_set_media(recordingOutput, obs_get_video(), obs_get_audio());
	obs_output_update(recordingOutput, settings);

	obs_data_release(settings);
}

void OBS_service::updateVideoRecordingEncoder()
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

	const char *quality = config_get_string(config, "SimpleOutput",
			"RecQuality");
	const char *encoder = config_get_string(config, "SimpleOutput",
            "RecEncoder");

    videoEncoder = encoder;
    videoQuality = quality;
    ffmpegOutput = false;

	if (strcmp(quality, "Stream") == 0) {
		if (videoRecordingEncoder != videoStreamingEncoder) {
			obs_encoder_release(videoRecordingEncoder);
			videoRecordingEncoder = videoStreamingEncoder;
			usingRecordingPreset = false;
		}
		return;

	} else if (strcmp(quality, "Lossless") == 0) {
		LoadRecordingPreset_Lossless();
		usingRecordingPreset = true;
		ffmpegOutput = true;
		UpdateRecordingSettings();
		return;

	} else {
		lowCPUx264 = false;
		if (strcmp(encoder, SIMPLE_ENCODER_X264) == 0) {
			LoadRecordingPreset_h264("obs_x264");
		} else if (strcmp(encoder, SIMPLE_ENCODER_X264_LOWCPU) == 0) {
			LoadRecordingPreset_h264("obs_x264");
			lowCPUx264 = true;
		} else if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0) {
			LoadRecordingPreset_h264("obs_qsv11");
		} else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0) {
			LoadRecordingPreset_h264("amd_amf_h264");
		} else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0) {
			LoadRecordingPreset_h264("ffmpeg_nvenc");
		}
		usingRecordingPreset = true;

		// if (!CreateAACEncoder(aacRecording, aacRecEncID, 192,
		// 			"simple_aac_recording", 0))
		// 	throw "Failed to create aac recording encoder "
		// 	      "(simple output)";
    }
    UpdateRecordingSettings();
}

static bool icq_available(obs_encoder_t *encoder)
{
	obs_properties_t *props = obs_encoder_properties(encoder);
	obs_property_t *p = obs_properties_get(props, "rate_control");
	bool icq_found = false;

	size_t num = obs_property_list_item_count(p);
	for (size_t i = 0; i < num; i++) {
		const char *val = obs_property_list_item_string(p, i);
		if (strcmp(val, "ICQ") == 0) {
			icq_found = true;
			break;
		}
	}

	obs_properties_destroy(props);
	return icq_found;
}

void OBS_service::UpdateRecordingSettings_qsv11(int crf)
{
	bool icq = icq_available(videoRecordingEncoder);

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "profile", "high");

	if (icq) {
		obs_data_set_string(settings, "rate_control", "ICQ");
		obs_data_set_int(settings, "icq_quality", crf);
	} else {
		obs_data_set_string(settings, "rate_control", "CQP");
		obs_data_set_int(settings, "qpi", crf);
		obs_data_set_int(settings, "qpp", crf);
		obs_data_set_int(settings, "qpb", crf);
	}

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

void OBS_service::UpdateRecordingSettings_nvenc(int cqp)
{
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "rate_control", "CQP");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset", "hq");
	obs_data_set_int(settings, "cqp", cqp);

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

void OBS_service::UpdateStreamingSettings_amd(obs_data_t *settings,
		int bitrate)
{
	// Static Properties
	obs_data_set_int(settings, "Usage", 0);
	obs_data_set_int(settings, "Profile", 100); // High

	// Rate Control Properties
	obs_data_set_int(settings, "RateControlMethod", 3);
	obs_data_set_int(settings, "Bitrate.Target", bitrate);
	obs_data_set_int(settings, "FillerData", 1);
	obs_data_set_int(settings, "VBVBuffer", 1);
	obs_data_set_int(settings, "VBVBuffer.Size", bitrate);

	// Picture Control Properties
	obs_data_set_double(settings, "KeyframeInterval", 2.0);
	obs_data_set_int(settings, "BFrame.Pattern", 0);
}

void OBS_service::UpdateRecordingSettings_amd_cqp(int cqp)
{
	obs_data_t *settings = obs_data_create();

	// Static Properties
	obs_data_set_int(settings, "Usage", 0);
	obs_data_set_int(settings, "Profile", 100); // High

	// Rate Control Properties
	obs_data_set_int(settings, "RateControlMethod", 0);
	obs_data_set_int(settings, "QP.IFrame", cqp);
	obs_data_set_int(settings, "QP.PFrame", cqp);
	obs_data_set_int(settings, "QP.BFrame", cqp);
	obs_data_set_int(settings, "VBVBuffer", 1);
	obs_data_set_int(settings, "VBVBuffer.Size", 100000);

	// Picture Control Properties
	obs_data_set_double(settings, "KeyframeInterval", 2.0);
	obs_data_set_int(settings, "BFrame.Pattern", 0);

	// Update and release
	obs_encoder_update(videoRecordingEncoder, settings);
	obs_data_release(settings);
}

void OBS_service::UpdateRecordingSettings_x264_crf(int crf)
{
	obs_data_t *settings = obs_data_create();
	obs_data_set_int(settings, "crf", crf);
	obs_data_set_bool(settings, "use_bufsize", true);
	obs_data_set_string(settings, "rate_control", "CRF");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset",
			lowCPUx264 ? "ultrafast" : "veryfast");

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

#define CROSS_DIST_CUTOFF 2000.0

int CalcCRF(int crf)
{
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

	int cx = config_get_uint(config, "Video", "OutputCX");
	int cy = config_get_uint(config, "Video", "OutputCY");
	double fCX = double(cx);
	double fCY = double(cy);

	if (lowCPUx264)
		crf -= 2;

	double crossDist = sqrt(fCX * fCX + fCY * fCY);
	double crfResReduction =
		fmin(CROSS_DIST_CUTOFF, crossDist) / CROSS_DIST_CUTOFF;
	crfResReduction = (1.0 - crfResReduction) * 10.0;

	return crf - int(crfResReduction);
}

void OBS_service::UpdateRecordingSettings()
{
	bool ultra_hq = (videoQuality == "HQ");
	int crf = CalcCRF(ultra_hq ? 16 : 23);

	if (astrcmp_n(videoEncoder.c_str(), "obs_x264", 4) == 0) {
		UpdateRecordingSettings_x264_crf(crf);

	} else if (videoEncoder == SIMPLE_ENCODER_QSV) {
		UpdateRecordingSettings_qsv11(crf);

	} else if (videoEncoder == SIMPLE_ENCODER_AMD) {
		UpdateRecordingSettings_amd_cqp(crf);

	} else if (videoEncoder == SIMPLE_ENCODER_NVENC) {
		UpdateRecordingSettings_nvenc(crf);
	}
}

obs_encoder_t* OBS_service::getStreamingEncoder(void)
{
    return videoStreamingEncoder;
}

void OBS_service::setStreamingEncoder(obs_encoder_t* encoder)
{
    obs_encoder_release(videoStreamingEncoder);
    videoStreamingEncoder = encoder;
}

obs_encoder_t* OBS_service::getRecordingEncoder(void)
{
    return videoRecordingEncoder;
}

void OBS_service::setRecordingEncoder(obs_encoder_t* encoder)
{
    obs_encoder_release(videoRecordingEncoder);
    videoRecordingEncoder = encoder;
}

obs_encoder_t* OBS_service::getAudioEncoder(void)
{
    return audioEncoder;
}

void OBS_service::setAudioEncoder(obs_encoder_t* encoder)
{
    obs_encoder_release(audioEncoder);
    audioEncoder = encoder;
}

obs_output_t* OBS_service::getStreamingOutput(void)
{
    return streamingOutput;
}

void OBS_service::setStreamingOutput(obs_output_t* output)
{
    obs_output_release(streamingOutput);
    streamingOutput = output;
}

obs_output_t* OBS_service::getRecordingOutput(void)
{
    return recordingOutput;
}

void OBS_service::setRecordingOutput(obs_output_t* output)
{
    obs_output_release(recordingOutput);
    recordingOutput = output;
}

void OBS_service::updateStreamSettings(void)
{
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    const char* currentOutputMode = config_get_string(config, "Output", "Mode");

	if(strcmp(currentOutputMode, "Simple") == 0) {
        OBS_service::updateVideoStreamingEncoder();
	} else if (strcmp(currentOutputMode, "Advanced") == 0) {

    }

    resetVideoContext("Stream");
    associateAudioAndVideoToTheCurrentStreamingContext();
    associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
}

void OBS_service::updateRecordSettings(void)
{
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    const char* currentOutputMode = config_get_string(config, "Output", "Mode");

	if(strcmp(currentOutputMode, "Simple") == 0) {
        updateVideoRecordingEncoder();
		updateRecordingOutput();
	} else if (strcmp(currentOutputMode, "Advanced") == 0) {
        const char* recType = config_get_string(config, "AdvOut", "RecType");
        if(recType != NULL && strcmp(recType, "Custom Output (FFmpeg)") == 0) {
            resetVideoContext("Record");
            associateAudioAndVideoToTheCurrentRecordingContext();
            UpdateFFmpegOutput();
            return;
        }
        // updateAdvancedRecordingOutput();
		updateRecordingOutput();
    }

    resetVideoContext("Record");
    associateAudioAndVideoToTheCurrentRecordingContext();
    associateAudioAndVideoEncodersToTheCurrentRecordingOutput();
}

