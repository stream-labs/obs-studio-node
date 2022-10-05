/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "osn-video.hpp"
#include <ipc-server.hpp>
#include <obs.h>
#include "osn-error.hpp"
#include "shared.hpp"

// DELETE ME WHEN REMOVING NODEOBS
#include "nodeobs_configManager.hpp"
#include "nodeobs_api.h"

void osn::Video::Register(ipc::server& srv)
{
    std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Video");
    cls->register_function(
        std::make_shared<ipc::function>("GetSkippedFrames", std::vector<ipc::type>{}, GetSkippedFrames));
    cls->register_function(
        std::make_shared<ipc::function>("GetTotalFrames", std::vector<ipc::type>{}, GetTotalFrames));

    cls->register_function(
        std::make_shared<ipc::function>("AddVideoContext", std::vector<ipc::type>{}, AddVideoContext));
    cls->register_function(
        std::make_shared<ipc::function>("RemoveVideoContext", std::vector<ipc::type>{ipc::type::UInt32}, RemoveVideoContext));
    cls->register_function(
        std::make_shared<ipc::function>("GetVideoContext", std::vector<ipc::type>{ipc::type::UInt32}, GetVideoContext));
    cls->register_function(
        std::make_shared<ipc::function>("SetVideoContext", std::vector<ipc::type>{
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
	        ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt64
        }, SetVideoContext));

    cls->register_function(
        std::make_shared<ipc::function>("GetLegacySettings", std::vector<ipc::type>{}, GetLegacySettings));
    cls->register_function(
        std::make_shared<ipc::function>("SetLegacySettings", std::vector<ipc::type>{
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32
        }, SetLegacySettings));
    srv.register_collection(cls);
}

void osn::Video::GetSkippedFrames(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_skipped_frames(obs_get_video())));
	AUTO_DEBUG;
}

void osn::Video::GetTotalFrames(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_total_frames(obs_get_video())));
	AUTO_DEBUG;
}

void osn::Video::GetVideoContext(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_video_info* video = osn::Video::Manager::GetInstance().find(args[0].value_union.ui64);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

    rval.push_back(ipc::value(video->fps_num));
    rval.push_back(ipc::value(video->fps_den));
    rval.push_back(ipc::value(video->base_width));
	rval.push_back(ipc::value(video->base_height));
	rval.push_back(ipc::value(video->output_width));
	rval.push_back(ipc::value(video->output_height));
    rval.push_back(ipc::value(video->output_format));
    rval.push_back(ipc::value(video->colorspace));
    rval.push_back(ipc::value(video->range));
    rval.push_back(ipc::value(video->scale_type));
    // ??? FPSType

    AUTO_DEBUG;
}

static inline const char* GetScaleType(const enum obs_scale_type& scaleType)
{
    switch (scaleType) {
        case OBS_SCALE_BILINEAR:
            return "bilinear";
        case OBS_SCALE_BICUBIC:
            return "bicubic";
        case OBS_SCALE_LANCZOS:
            return "lanczos";
        default:
            return "bilinear";
    }
}

static inline enum obs_scale_type ScaleTypeFromStr(const std::string& value)
{
    if (value.compare("bilinear") == 0)
        return OBS_SCALE_BILINEAR;
    else if (value.compare("bicubic") == 0)
        return OBS_SCALE_BICUBIC;
    else if (value.compare("lanczos") == 0)
        return OBS_SCALE_LANCZOS;

    return OBS_SCALE_BILINEAR;
}

static inline const char* GetOutputFormat(const enum video_format& outputFormat)
{
    switch (outputFormat) {
        case VIDEO_FORMAT_I420:
            return "I420";
        case VIDEO_FORMAT_NV12:
            return "NV12";
        case VIDEO_FORMAT_YVYU:
            return "YVYU";
        case VIDEO_FORMAT_YUY2:
            return "YUY2";
        case VIDEO_FORMAT_UYVY:
            return "UYVY";
        case VIDEO_FORMAT_RGBA:
            return "RGBA";
        case VIDEO_FORMAT_BGRA:
            return "BGRA";
        case VIDEO_FORMAT_BGRX:
            return "BGRX";
        case VIDEO_FORMAT_Y800:
            return "Y800";
        case VIDEO_FORMAT_I444:
            return "I444";
        case VIDEO_FORMAT_BGR3:
            return "BGR3";
        case VIDEO_FORMAT_I422:
            return "I422";
        case VIDEO_FORMAT_I40A:
            return "I40A";
        case VIDEO_FORMAT_I42A:
            return "I42A";
        case VIDEO_FORMAT_YUVA:
            return "YUVA";
        case VIDEO_FORMAT_AYUV:
            return "AYUV";
        default:
            return "I420";
    }
}

static inline enum video_format OutputFormFromStr(const std::string& value)
{
    if (value.compare("I420") == 0)
        return VIDEO_FORMAT_I420;
    else if (value.compare("NV12") == 0)
        return VIDEO_FORMAT_NV12;
    else if (value.compare("YVYU") == 0)
        return VIDEO_FORMAT_YVYU;
    else if (value.compare("YUY2") == 0)
        return VIDEO_FORMAT_YUY2;
    else if (value.compare("UYVY") == 0)
        return VIDEO_FORMAT_UYVY;
    else if (value.compare("RGBA") == 0)
        return VIDEO_FORMAT_RGBA;
    else if (value.compare("BGRA") == 0)
        return VIDEO_FORMAT_BGRA;
    else if (value.compare("BGRX") == 0)
        return VIDEO_FORMAT_BGRX;
    else if (value.compare("Y800") == 0)
        return VIDEO_FORMAT_Y800;
    else if (value.compare("I444") == 0)
        return VIDEO_FORMAT_I444;
    else if (value.compare("BGR3") == 0)
        return VIDEO_FORMAT_BGR3;
    else if (value.compare("I422") == 0)
        return VIDEO_FORMAT_I422;
    else if (value.compare("I40A") == 0)
        return VIDEO_FORMAT_I40A;
    else if (value.compare("I42A") == 0)
        return VIDEO_FORMAT_I42A;
    else if (value.compare("YUVA") == 0)
        return VIDEO_FORMAT_YUVA;
    else if (value.compare("AYUV") == 0)
        return VIDEO_FORMAT_AYUV;

    return VIDEO_FORMAT_I420;
}

static inline const char* GetColorSpace(const enum video_colorspace& colorSpace)
{
    switch (colorSpace) {
        case VIDEO_CS_DEFAULT:
            return "709";
        case VIDEO_CS_601:
            return "601";
        case VIDEO_CS_709:
            return "709";
        case VIDEO_CS_SRGB:
            return "sRGB";
        default:
            return "709";
    }
}

static inline enum video_colorspace ColorSpaceFromStr(const std::string& value)
{
    if (value.compare("709") == 0)
        return VIDEO_CS_709;
    else if (value.compare("601") == 0)
        return VIDEO_CS_601;
    else if (value.compare("sRGB") == 0)
        return VIDEO_CS_SRGB;

    return VIDEO_CS_DEFAULT;
}

static inline const char* GetColorRange(const enum video_range_type& colorRange)
{
    switch (colorRange) {
        case VIDEO_RANGE_DEFAULT:
            return "Partial";
        case VIDEO_RANGE_PARTIAL:
            return "Partial";
        case VIDEO_RANGE_FULL:
            return "Full";
        default:
            return "Partial";
    }
}

static inline enum video_range_type ColoRangeFromStr(const std::string& value)
{
    if (value.compare("Partial") == 0)
        return VIDEO_RANGE_PARTIAL;
    else if (value.compare("Full") == 0)
        return VIDEO_RANGE_PARTIAL;

    return VIDEO_RANGE_DEFAULT;
}

void osn::Video::SetVideoContext(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    if (args.size() != 12) {
        PRETTY_ERROR_RETURN(ErrorCode::Error,
            "Invalid number of arguments to set the video context.");
    }
    
 	obs_video_info* canvas = osn::Video::Manager::GetInstance().find(args[11].value_union.ui64);
	obs_video_info  video  = {0};


    uint32_t        outputFormat = ;
	uint32_t        colorspace   = ;
	uint32_t        range        = ;
	uint32_t        scaleType    = ;
	uint32_t        fpsType      = ;

#ifdef _WIN32
    video.graphics_module = "libobs-d3d11.dll";
#else
    video.graphics_module = "libobs-opengl";
#endif
	video.fps_num = args[0].value_union.ui32;
	video.fps_den = args[1].value_union.ui32;

	video.base_width    = args[2].value_union.ui32;
	video.base_height   = args[3].value_union.ui32;
	video.output_width  = args[4].value_union.ui32;
	video.output_height = args[5].value_union.ui32;

    video.output_format  = (video_format)args[6].value_union.ui32;
	video.colorspace     = (video_colorspace)args[7].value_union.ui32;
	video.range          = (video_range_type)args[8].value_union.ui32;
	video.scale_type     = (obs_scale_type)args[9].value_union.ui32;
    video.adapter        = 0;
    video.gpu_conversion = true;
    // ???? fpsType args[10].value_union.ui32

    try {
		obs_set_video_info(canvas, &video);
    } catch (const char* error) {
        blog(LOG_ERROR, error);
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
}

void osn::Video::AddVideoContext(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    if (args.size() != 0) {
        PRETTY_ERROR_RETURN(ErrorCode::Error,
            "Invalid number of arguments to set the video context.");
    }
	obs_video_info* canvas = obs_create_video_info();
	utility::unique_id::id_t uid = osn::Video::Manager::GetInstance().allocate(canvas);

    if (canvas == NULL) 
        PRETTY_ERROR_RETURN(ErrorCode::OutOfBounds, "Failed to add canvas.");
 
    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
    AUTO_DEBUG;
}

void osn::Video::RemoveVideoContext(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    if (args.size() != 1) {
        PRETTY_ERROR_RETURN(ErrorCode::Error,
            "Invalid number of arguments to remove the video context.");
    }

	obs_video_info *canvas = osn::Video::Manager::GetInstance().find(args[0].value_union.ui64);

    if(!canvas) {
        PRETTY_ERROR_RETURN(ErrorCode::Error, "No video context is currently set.");
    }

	obs_remove_video_info(canvas);
	osn::Video::Manager::GetInstance().free(args[0].value_union.ui64);
    try {
        obs_reset_video();
    } catch (const char* error) {
        blog(LOG_ERROR, error);
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
}

osn::Video::Manager& osn::Video::Manager::GetInstance()
{
	static osn::Video::Manager _inst;
	return _inst;
}

void osn::Video::GetLegacySettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

    uint32_t fpsType =
         config_get_uint(
             ConfigManager::getInstance().getBasic(),
             "Video", "FPSType");
    switch (fpsType)
    {
    case 0: {
        std::string fpsCommon =
            config_get_string(
                ConfigManager::getInstance().getBasic(),
                "Video", "FPSCommon");
        if (fpsCommon.compare("10") == 0) {
            rval.push_back(ipc::value((uint32_t)10));
            rval.push_back(ipc::value((uint32_t)1));
        } else if (fpsCommon.compare("20") == 0) {
            rval.push_back(ipc::value((uint32_t)20));
            rval.push_back(ipc::value((uint32_t)1));
        } else if (fpsCommon.compare("24 NTSC") == 0) {
            rval.push_back(ipc::value((uint32_t)24000));
            rval.push_back(ipc::value((uint32_t)1001));
        } else if (fpsCommon.compare("25") == 0) {
            rval.push_back(ipc::value((uint32_t)25));
            rval.push_back(ipc::value((uint32_t)1));
        } else if (fpsCommon.compare("29.97") == 0) {
            rval.push_back(ipc::value((uint32_t)30000));
            rval.push_back(ipc::value((uint32_t)1001));
        } else if (fpsCommon.compare("48") == 0) {
            rval.push_back(ipc::value((uint32_t)48));
            rval.push_back(ipc::value((uint32_t)1));
        } else if (fpsCommon.compare("59.94") == 0) {
            rval.push_back(ipc::value((uint32_t)60000));
            rval.push_back(ipc::value((uint32_t)1001));
        } else if (fpsCommon.compare("60") == 0) {
            rval.push_back(ipc::value((uint32_t)60));
            rval.push_back(ipc::value((uint32_t)1));
        } else {
            rval.push_back(ipc::value((uint32_t)30));
            rval.push_back(ipc::value((uint32_t)1));
        }
        break;
    }
    case 1: {
        rval.push_back(ipc::value((uint32_t)
            config_get_uint(
                ConfigManager::getInstance().getBasic(),
                "Video", "FPSInt")));
        rval.push_back(ipc::value((uint32_t)1));
        break;
    }
    case 2: {
        rval.push_back(ipc::value((uint32_t)
            config_get_uint(
                ConfigManager::getInstance().getBasic(),
                "Video", "FPSNum")));
        rval.push_back(ipc::value((uint32_t)
            config_get_uint(
                ConfigManager::getInstance().getBasic(),
                "Video", "FPSDen")));
        break;
    }
    default: {
        rval.push_back(ipc::value((uint32_t)30));
        rval.push_back(ipc::value((uint32_t)1));
        break;
    }
    }

    rval.push_back(
        ipc::value(config_get_uint(
            ConfigManager::getInstance().getBasic(),
            "Video", "BaseCX")));
    rval.push_back(
        ipc::value(config_get_uint(
            ConfigManager::getInstance().getBasic(),
            "Video", "BaseCY")));
    rval.push_back(
        ipc::value(config_get_uint(
            ConfigManager::getInstance().getBasic(),
            "Video", "OutputCX")));
    rval.push_back(
        ipc::value(config_get_uint(
            ConfigManager::getInstance().getBasic(),
            "Video", "OutputCY")));
    rval.push_back(ipc::value(
        OutputFormFromStr(config_get_string(
            ConfigManager::getInstance().getBasic(),
            "Video", "ColorFormat"))));
    rval.push_back(ipc::value(
        ColorSpaceFromStr(config_get_string(
            ConfigManager::getInstance().getBasic(),
            "Video", "ColorSpace"))));
    rval.push_back(ipc::value(
        ColoRangeFromStr(config_get_string(
            ConfigManager::getInstance().getBasic(),
            "Video", "ColorRange"))));
    rval.push_back(ipc::value(
        ScaleTypeFromStr(config_get_string(
            ConfigManager::getInstance().getBasic(),
            "Video", "ScaleType"))));
    rval.push_back(ipc::value(
        config_get_uint(
             ConfigManager::getInstance().getBasic(),
             "Video", "FPSType")));

    AUTO_DEBUG;
}

void osn::Video::SetLegacySettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    uint32_t fpsNum = args[0].value_union.ui32;
    uint32_t fpsDen = args[1].value_union.ui32;
    uint32_t baseWidth = args[2].value_union.ui32;
    uint32_t baseHeight = args[3].value_union.ui32;
    uint32_t outputWidth = args[4].value_union.ui32;
    uint32_t outputHeight = args[5].value_union.ui32;
    uint32_t outputFormat = args[6].value_union.ui32;
    uint32_t colorspace = args[7].value_union.ui32;
    uint32_t range = args[8].value_union.ui32;
    uint32_t scaleType = args[9].value_union.ui32;

    config_set_uint(
        ConfigManager::getInstance().getBasic(),
        "Video", "FPSNum", fpsNum);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "FPSDen", fpsDen);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "BaseCX", baseWidth);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "BaseCY", baseHeight);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "OutputCX", outputWidth);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "OutputCY", outputHeight);
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ScaleType", GetScaleType((obs_scale_type)scaleType));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorFormat", GetOutputFormat((video_format)outputFormat));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorSpace", GetColorSpace((video_colorspace)colorspace));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorRange", GetColorRange((video_range_type)range));

    config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}