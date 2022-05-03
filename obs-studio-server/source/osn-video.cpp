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
#include "error.hpp"
#include "shared.hpp"

// DELETE ME WHEN REMOVING NODEOBS
#include "nodeobs_configManager.hpp"

void osn::Video::Register(ipc::server& srv)
{
    std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Video");
    cls->register_function(std::make_shared<ipc::function>(
        "GetSkippedFrames", std::vector<ipc::type>{}, GetSkippedFrames));
    cls->register_function(
        std::make_shared<ipc::function>("GetTotalFrames", std::vector<ipc::type>{}, GetTotalFrames));

    cls->register_function(
        std::make_shared<ipc::function>("GetVideoContext", std::vector<ipc::type>{}, GetVideoContext));
    cls->register_function(
        std::make_shared<ipc::function>("SetVideoContext", std::vector<ipc::type>{
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
            ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32, ipc::type::UInt32,
            ipc::type::UInt32, ipc::type::UInt32,
        }, SetVideoContext));
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
    obs_video_info video;
    if(!obs_get_video_info(&video)) {
        PRETTY_ERROR_RETURN(ErrorCode::Error, "No video context is currently set.");
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

    rval.push_back(ipc::value(video.fps_num));
    rval.push_back(ipc::value(video.fps_den));
    rval.push_back(ipc::value(video.base_width));
    rval.push_back(ipc::value(video.base_height));
    rval.push_back(ipc::value(video.output_width));
    rval.push_back(ipc::value(video.output_height));
    rval.push_back(ipc::value(video.output_format));
    rval.push_back(ipc::value(video.colorspace));
    rval.push_back(ipc::value(video.range));
    rval.push_back(ipc::value(video.scale_type));

    AUTO_DEBUG;
}

static inline const char* GetScaleType(enum obs_scale_type scaleType)
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

static inline const char* GetOutputFormat(enum video_format outputFormat)
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

static inline const char* GetColorSpace(enum video_colorspace colorSpace)
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

static inline const char* GetColorRange(enum video_range_type colorRange)
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

static inline void SaveVideoSettings(obs_video_info video)
{
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "FPSNum", video.fps_num);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "FPSDen", video.fps_den);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "BaseCX", video.base_width);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "BaseCY", video.base_height);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "OutputCX", video.output_width);
    config_set_uint(
        ConfigManager::getInstance().getBasic(), "Video", "OutputCY", video.output_height);
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ScaleType", GetScaleType(video.scale_type));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorFormat", GetOutputFormat(video.output_format));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorSpace", GetColorSpace(video.colorspace));
    config_set_string(
        ConfigManager::getInstance().getBasic(),
        "Video", "ColorRange", GetColorRange(video.range));

    config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}

void osn::Video::SetVideoContext(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    if (args.size() != 10) {
        PRETTY_ERROR_RETURN(ErrorCode::Error,
            "Invalid number of arguments to set the video context.");
    }

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

    obs_video_info video;
#ifdef _WIN32
    video.graphics_module = "libobs-d3d11.dll";
#else
    video.graphics_module = "libobs-opengl";
#endif
    video.fps_num = fpsNum;
    video.fps_den = fpsDen;
    video.base_width = baseWidth;
    video.base_height = baseHeight;
    video.output_width = outputWidth;
    video.output_height = outputHeight;
    video.output_format = (video_format)outputFormat;
    video.colorspace = (video_colorspace)colorspace;
    video.range = (video_range_type)range;
    video.scale_type = (obs_scale_type)scaleType;
    video.adapter = 0;
    video.gpu_conversion = true;

    try {
        obs_reset_video(&video);

        // DELETE ME WHEN REMOVING NODEOBS
        SaveVideoSettings(video);
    } catch (const char* error) {
        blog(LOG_ERROR, error);
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
}