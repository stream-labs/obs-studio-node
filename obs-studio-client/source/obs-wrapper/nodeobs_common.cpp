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

#include <iomanip>
#include <map>
#include "nodeobs_content.h"

/* For sceneitem transform modifications.
 * We should consider moving this to another module */
#include <graphics/matrix4.h>

#include "obs-shared.hpp"

#include <thread>

std::map<std::string, OBS::Display*> displays;
std::string                          sourceSelected;
bool                                 firstDisplayCreation = true;

std::thread* windowMessage = NULL;
bool displayCreated = false;

/* A lot of the sceneitem functionality is a lazy copy-pasta from the Qt UI. */
// https://github.com/jp9000/obs-studio/blob/master/UI/window-basic-main.cpp#L4888
static void GetItemBox(obs_sceneitem_t* item, vec3& tl, vec3& br)
{
	matrix4 boxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);

	vec3_set(&tl, M_INFINITE, M_INFINITE, 0.0f);
	vec3_set(&br, -M_INFINITE, -M_INFINITE, 0.0f);

	auto GetMinPos = [&](float x, float y) {
		vec3 pos;
		vec3_set(&pos, x, y, 0.0f);
		vec3_transform(&pos, &pos, &boxTransform);
		vec3_min(&tl, &tl, &pos);
		vec3_max(&br, &br, &pos);
	};

	GetMinPos(0.0f, 0.0f);
	GetMinPos(1.0f, 0.0f);
	GetMinPos(0.0f, 1.0f);
	GetMinPos(1.0f, 1.0f);
}

static vec3 GetItemTL(obs_sceneitem_t* item)
{
	vec3 tl, br;
	GetItemBox(item, tl, br);
	return tl;
}

static void SetItemTL(obs_sceneitem_t* item, const vec3& tl)
{
	vec3 newTL;
	vec2 pos;

	obs_sceneitem_get_pos(item, &pos);
	newTL = GetItemTL(item);
	pos.x += tl.x - newTL.x;
	pos.y += tl.y - newTL.y;
	obs_sceneitem_set_pos(item, &pos);
}

static bool CenterAlignSelectedItems(obs_scene_t* scene, obs_sceneitem_t* item, void* param)
{
	obs_bounds_type boundsType = *reinterpret_cast<obs_bounds_type*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	obs_video_info ovi;
	obs_get_video_info(&ovi);

	obs_transform_info itemInfo;
	vec2_set(&itemInfo.pos, 0.0f, 0.0f);
	vec2_set(&itemInfo.scale, 1.0f, 1.0f);
	itemInfo.alignment = OBS_ALIGN_LEFT | OBS_ALIGN_TOP;
	itemInfo.rot       = 0.0f;

	vec2_set(&itemInfo.bounds, float(ovi.base_width), float(ovi.base_height));
	itemInfo.bounds_type      = boundsType;
	itemInfo.bounds_alignment = OBS_ALIGN_CENTER;

	obs_sceneitem_set_info(item, &itemInfo);

	UNUSED_PARAMETER(scene);
	return true;
}

static bool MultiplySelectedItemScale(obs_scene_t* scene, obs_sceneitem_t* item, void* param)
{
	vec2& mul = *reinterpret_cast<vec2*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	vec3 tl = GetItemTL(item);

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);
	vec2_mul(&scale, &scale, &mul);
	obs_sceneitem_set_scale(item, &scale);

	SetItemTL(item, tl);

	UNUSED_PARAMETER(scene);
	return true;
}

#ifdef _WIN32

static void OnDeviceLost(void* data)
{
	// Do nothing
	UNUSED_PARAMETER(data);
}

static void OnDeviceRebuilt(void* device, void* data)
{
	for (const auto& p : displays) {
		if (auto display = p.second) {
			// After device is rebuilt, there are can be problems with incorrect size of display
			// In order to fix this, need to update the display by adjusting it's size
			// TODO: find a better way to fix this
			const auto size = display->GetSize();
			display->SetSize(size.first, size.second);
		}
	}
}
#endif

void popupAeroDisabledWindow(void)
{
#ifdef WIN32
	MessageBox(
	    NULL,
	    TEXT("Streamlabs OBS needs Aero enabled to run properly on Windows 7.  "
	         "If you've disabled Aero for performance reasons, "
	         "you may still use the app, but you will need to keep the window maximized.\n\n\n\n\n"
	         "This is a workaround to keep Streamlabs OBS running and not the preferred route. "
	         "We recommend upgrading to Windows 10 or enabling Aero."),
	    TEXT("Aero is disabled"),
	    MB_OK);
#endif
}

void OBS_content::OBS_content_createDisplay(uint64_t windowHandle, std::string key, int32_t displayMode)
{
	auto found = displays.find(key);

	/* If found, do nothing since it would
	be a memory leak otherwise. */
	if (found != displays.end()) {
		std::cerr << "Duplicate key provided to createDisplay: " << key << std::endl;
		return;
	}

	enum obs_video_rendering_mode mode = OBS_MAIN_VIDEO_RENDERING;
	switch (displayMode) {
	case 0:
		mode = OBS_MAIN_VIDEO_RENDERING;
		break;
	case 1:
		mode = OBS_STREAMING_VIDEO_RENDERING;
		break;
	case 2:
		mode = OBS_RECORDING_VIDEO_RENDERING;
		break;
	}

#ifdef WIN32
	displays.insert_or_assign(key, new OBS::Display(windowHandle, mode));
	if (!IsWindows8OrGreater()) {
		BOOL enabled = FALSE;
		DwmIsCompositionEnabled(&enabled);
		if (!enabled && firstDisplayCreation) {
			windowMessage = new std::thread(popupAeroDisabledWindow);
		}
	}
#else
	OBS::Display *display = new OBS::Display(windowHandle, mode);
	displays.insert_or_assign(key, display);
#endif

	// device rebuild functionality available only with D3D
#ifdef _WIN32
	if (firstDisplayCreation) {
		obs_enter_graphics();

		gs_device_loss callbacks;
		callbacks.device_loss_release = &OnDeviceLost;
		callbacks.device_loss_rebuild = &OnDeviceRebuilt;
		callbacks.data                = nullptr;

		gs_register_loss_callbacks(&callbacks);
		obs_leave_graphics();
	}
#endif

	firstDisplayCreation = false;
}

void OBS_content::OBS_content_destroyDisplay(std::string key)
{
	auto found = displays.find(key);

	if (found == displays.end()) {
		std::cerr << "Failed to find key for destruction: " << key << std::endl;
		return;
	}

	if (windowMessage != NULL && windowMessage->joinable())
		windowMessage->join();
    
    delete found->second;
    displays.erase(found);
}

void OBS_content::OBS_content_shutdownDisplays()
{
	blog(LOG_DEBUG, "Displays remaining till shutdown %d", displays.size());
	while (displays.size() > 0) {
		auto itr = displays.begin();
		delete itr->second;
		displays.erase(itr);
	}
}

void OBS_content::OBS_content_createSourcePreviewDisplay(
    uint64_t windowHandle,
	std::string sourceName,
	std::string key)
{
	auto found = displays.find(key);

	/* If found, do nothing since it would
	be a memory leak otherwise. */
	if (found != displays.end()) {
		std::cerr << "Duplicate key provided to createDisplay!" << std::endl;
		return;
	}

	OBS::Display *display = new OBS::Display(windowHandle, OBS_MAIN_VIDEO_RENDERING, sourceName);
	displays.insert_or_assign(key, display);
}

void OBS_content::OBS_content_resizeDisplay(
    std::string key,
	uint32_t width,
	uint32_t height)
{
	auto value = displays.find(key);
	if (value == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to resizeDisplay: %s", key.c_str());
		return;
	}

	OBS::Display* display = value->second;

	display->m_gsInitData.cx = width;
	display->m_gsInitData.cy = height;

	// Resize Display
    obs_display_resize(display->m_display,
		display->m_gsInitData.cx,
		display->m_gsInitData.cy);

    // Store new size.
    display->UpdatePreviewArea();

#ifdef WIN32
	display->SetSize(display->m_gsInitData.cx, display->m_gsInitData.cy);
#endif
}

void OBS_content::OBS_content_moveDisplay(
    std::string key,
	uint32_t x,
	uint32_t y)
{
	auto value = displays.find(key);
	if (value == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to moveDisplay: %s", key.c_str());
		return;
	}

	OBS::Display* display = value->second;
	display->m_position.first  = x;
	display->m_position.second = y;

	display->SetPosition(x, y);
}

void OBS_content::OBS_content_setPaddingSize(std::string key, uint32_t paddingSize)
{
	// Find Display
	auto it = displays.find(key);
	if (it == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to setPaddingSize: %s", key.c_str());
		return;
	}

	it->second->SetPaddingSize(paddingSize);
}

void OBS_content::OBS_content_setPaddingColor(
    std::string key, uint32_t r,
	uint32_t g,	uint32_t b, uint32_t a)
{
	union
	{
		uint32_t rgba;
		uint8_t  c[4];
	} color;

	// Assign Color
	color.c[0] = (uint8_t)(r);
	color.c[1] = (uint8_t)(g);
	color.c[2] = (uint8_t)(b);
	color.c[3] = (uint8_t)(a);

	// Find Display
	auto it = displays.find(key);
	if (it == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to setPaddingColor: %s", key.c_str());
		return;
	}

	it->second->SetPaddingColor(color.c[0], color.c[1], color.c[2], color.c[3]);
}

void OBS_content::OBS_content_setOutlineColor(
    std::string key, uint32_t r,
	uint32_t g,	uint32_t b, uint32_t a)
{
	union
	{
		uint32_t rgba;
		uint8_t  c[4];
	} color;

	// Assign Color
	color.c[0] = (uint8_t)(r);
	color.c[1] = (uint8_t)(g);
	color.c[2] = (uint8_t)(b);
	color.c[3] = (uint8_t)(a);

	// Find Display
	auto it = displays.find(key);
	if (it == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to setOutlineColor: %s", key.c_str());
		return;
	}

	it->second->SetOutlineColor(color.c[0], color.c[1], color.c[2], color.c[3]);
}

void OBS_content::OBS_content_setShouldDrawUI(std::string key, bool drawUI)
{
	// Find Display
	auto it = displays.find(key);
	if (it == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to setShouldDrawUI: %s", key.c_str());
		return;
	}

	it->second->SetDrawUI(drawUI);
}

std::pair<int32_t, int32_t> OBS_content::OBS_content_getDisplayPreviewOffset(std::string key)
{
	auto value = displays.find(key);
	if (value == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to getDisplayPreviewOffset: %s", key.c_str());
		return std::make_pair(0, 0);
	}

	OBS::Display* display = value->second;
	return display->GetPreviewOffset();
}

std::pair<int32_t, int32_t> OBS_content::OBS_content_getDisplayPreviewSize(std::string key)
{
	auto value = displays.find(key);
	if (value == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to getDisplayPreviewSize: %s", key.c_str());
		return std::make_pair(0, 0);
	}

	OBS::Display* display = value->second;
	return display->GetPreviewSize();
}

void OBS_content::OBS_content_setDrawGuideLines(std::string key, bool drawGuideLines)
{
	// Find Display
	auto it = displays.find(key);
	if (it == displays.end()) {
		blog(LOG_ERROR, "Invalid key provided to setDrawGuideLines: %s", key.c_str());
		return;
	}
	it->second->SetDrawGuideLines(drawGuideLines);
}

// void OBS_content::OBS_content_createIOSurface(
//     void*                          data,
//     const int64_t                  id,
//     const std::vector<ipc::value>& args,
//     std::vector<ipc::value>&       rval)
// {
// #ifdef __APPLE__
// 	// Find Display
// 	auto it = displays.find(args[0].value_str);
// 	if (it == displays.end()) {
// 		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
// 		rval.push_back(ipc::value("Display key is not valid!"));
// 		return;
// 	}
// 	if (it->second->m_display == nullptr) {
// 		return;
// 	}

// 	uint32_t surfaceID =
// 		obs_display_create_iosurface(it->second->m_display, 
// 			it->second->GetSize().first,
// 			it->second->GetSize().second);

// 	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
// 	rval.push_back(ipc::value((uint32_t)surfaceID));
// #elif WIN32
// 	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
// #endif
// 	AUTO_DEBUG;
// }
