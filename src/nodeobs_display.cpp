#include "nodeobs_display.h"
#include "nodeobs_module.h"
#include <iostream>
#include <string>
#include <map>

#include <graphics/vec4.h>
#include <graphics/matrix4.h>
#include <util/platform.h>

std::vector<std::pair<std::string, std::pair<uint32_t, uint32_t>>> sourcesSize;
v8::Local<v8::Function> g_updateSourceSize;

extern std::string currentScene; /* defined in OBS_content.cpp */

static const uint32_t grayPaddingArea = 10ul;

static BOOL CALLBACK EnumChromeWindowsProc(HWND hwnd, LPARAM lParam)
{
	char buf[256];
	if (GetClassNameA(hwnd, buf, sizeof(buf) / sizeof(*buf))) {
		if (strstr(buf, "Intermediate D3D Window")) {
			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			if ((style & WS_CLIPSIBLINGS) == 0) {
				style |= WS_CLIPSIBLINGS;
				(void)SetWindowLongPtr(hwnd, GWL_STYLE, style);
			}
		}
	}
	return TRUE;
}

static void FixChromeD3DIssue(HWND chromeWindow)
{
	(void)EnumChildWindows(chromeWindow, EnumChromeWindowsProc, (LPARAM)NULL);

	LONG_PTR style = GetWindowLongPtr(chromeWindow, GWL_STYLE);
	if ((style & WS_CLIPCHILDREN) == 0) {
		style |= WS_CLIPCHILDREN;
		(void)SetWindowLongPtr(chromeWindow, GWL_STYLE, style);
	}
}

static void RecalculateApectRatioConstrainedSize(
      uint32_t origW, uint32_t origH, uint32_t sourceW, uint32_t sourceH,
      int32_t &outX, int32_t &outY, uint32_t &outW, uint32_t &outH)
{

	double_t sourceAR = double_t(sourceW) / double_t(sourceH);
	double_t origAR = double_t(origW) / double_t(origH);
	if (origAR > sourceAR) {
		outW = uint32_t(double_t(origH) * sourceAR);
		outH = origH;
	} else {
		outW = origW;
		outH = uint32_t(double_t(origW) * (1.0 / sourceAR));
	}
	outX = (int32_t(origW / 2) - int32_t(outW / 2));
	outY = (int32_t(origH / 2) - int32_t(outH / 2));
}

OBS::Display::Display()
{
#if defined(_WIN32)
	DisplayWndClass();
#elif defined(__APPLE__)
#elif defined(__linux__) || defined(__FreeBSD__)
#endif

	m_gsInitData.adapter = 0;
	m_gsInitData.cx = 960;
	m_gsInitData.cy = 540;
	m_gsInitData.format = GS_RGBA;
	m_gsInitData.zsformat = GS_ZS_NONE;
	m_gsInitData.num_backbuffers = 1;
	m_display = nullptr;
	m_source = nullptr;
	m_position.first = 0;
	m_position.second = 0;

	obs_enter_graphics();
	m_gsSolidEffect = obs_get_base_effect(OBS_EFFECT_SOLID);

	GS::Vertex v(nullptr, nullptr, nullptr, nullptr, nullptr);

	m_boxLine = std::make_unique<GS::VertexBuffer>(6);
	m_boxLine->Resize(6);
	v = m_boxLine->At(0);
	vec3_set(v.position, 0, 0, 0);
	vec4_set(v.uv[0], 0, 0, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxLine->At(1);
	vec3_set(v.position, 1, 0, 0);
	vec4_set(v.uv[0], 1, 0, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxLine->At(2);
	vec3_set(v.position, 1, 1, 0);
	vec4_set(v.uv[0], 1, 1, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxLine->At(3);
	vec3_set(v.position, 0, 1, 0);
	vec4_set(v.uv[0], 0, 1, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxLine->At(4);
	vec3_set(v.position, 0, 0, 0);
	vec4_set(v.uv[0], 0, 0, 0, 0);
	*v.color = 0xFFFFFFFF;
	m_boxLine->Update();

	m_boxTris = std::make_unique<GS::VertexBuffer>(4);
	m_boxTris->Resize(4);
	v = m_boxTris->At(0);
	vec3_set(v.position, 0, 0, 0);
	vec4_set(v.uv[0], 0, 0, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxTris->At(1);
	vec3_set(v.position, 1, 0, 0);
	vec4_set(v.uv[0], 1, 0, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxTris->At(2);
	vec3_set(v.position, 0, 1, 0);
	vec4_set(v.uv[0], 0, 1, 0, 0);
	*v.color = 0xFFFFFFFF;
	v = m_boxTris->At(3);
	vec3_set(v.position, 1, 1, 0);
	vec4_set(v.uv[0], 1, 1, 0, 0);
	*v.color = 0xFFFFFFFF;
	m_boxTris->Update();

	// Text
	m_textVertices = new GS::VertexBuffer(65535);
	m_textEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	m_textTexture = gs_texture_create_from_file((g_moduleDirectory +
	                                             "/resources/roboto.png").c_str());
	if (!m_textTexture)
		throw std::runtime_error("couldn't load roboto font");

	obs_leave_graphics();

	SetOutlineColor(26, 230, 168);
	SetGuidelineColor(26, 230, 168);

	UpdatePreviewArea();

	m_drawGuideLines = true;
}

OBS::Display::Display(uint64_t windowHandle) : Display()
{
#if defined(_WIN32)
	FixChromeD3DIssue((HWND)windowHandle);

	m_ourWindow = CreateWindowEx(
	                    0,//WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
	                    TEXT("Win32DisplayClass"), TEXT("SlobsChildWindowPreview"),
	                    WS_VISIBLE | WS_POPUP,
	                    0, 0, m_gsInitData.cx, m_gsInitData.cy,
	                    NULL,
	                    NULL, NULL, this);
	if (m_ourWindow == NULL) {
		DWORD errorCode = GetLastError();
		LPSTR errorStr = nullptr;
		DWORD errorStrSize = 16,
		      errorStrLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		                                  FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, LANG_USER_DEFAULT, errorStr,
		                                  errorStrSize, NULL);
		std::string exceptionMessage(errorStr, errorStrLen);
		exceptionMessage = "Unexpected WinAPI error: " + exceptionMessage;
		LocalFree(errorStr);

		throw std::system_error(errorCode, std::system_category(), exceptionMessage);
	}

	SetParent(m_ourWindow, (HWND)windowHandle);
	m_gsInitData.window.hwnd = reinterpret_cast<void *>(m_ourWindow);
#elif defined(__APPLE__)
	// ToDo
#elif defined(__linux__) || defined(__FreeBSD__)
	// ToDo
#endif

	m_display = obs_display_create(&m_gsInitData);
	if (!m_display)
		throw std::runtime_error("unable to create display");

	obs_display_add_draw_callback(m_display,
	                              (void(*)(void *, uint32_t, uint32_t))DisplayCallback, this);
	obs_display_set_background_color(m_display, 0x0);
}

OBS::Display::Display(uint64_t windowHandle,
                      std::string sourceName) : Display(windowHandle)
{
	std::cout << "creating display" << std::endl;
	m_source = obs_get_source_by_name(sourceName.c_str());
}

OBS::Display::~Display()
{
	if (m_source)
		obs_source_release(m_source);
	if (m_display)
		obs_display_destroy(m_display);

	obs_enter_graphics();
	if (m_textVertices)
		delete m_textVertices;
	m_boxLine = nullptr;
	m_boxTris = nullptr;
	obs_leave_graphics();

#if defined(_WIN32)
	if (m_ourWindow)
		DestroyWindow((HWND)(m_gsInitData.window.hwnd));
#elif defined(__APPLE__)
	// ToDo
#elif defined(__linux__) || defined(__FreeBSD__)
	// ToDo
#endif
}

void OBS::Display::SetPosition(uint32_t x, uint32_t y)
{
	if (m_source != NULL) {
		blog(LOG_DEBUG, "<" __FUNCTION__
		     "> Adjusting display position for source %s to %ldx%ld.",
		     obs_source_get_name(m_source), x, y);
	}

	// Move Window
#if defined(_WIN32)
	SetWindowPos(m_ourWindow, NULL,
	             x, y, m_gsInitData.cx, m_gsInitData.cy,
	             SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOACTIVATE);
#elif defined(__APPLE__)
#elif defined(__linux__) || defined(__FreeBSD__)
#endif

// Store new position.
	m_position.first = x;
	m_position.second = y;
}

std::pair<uint32_t, uint32_t> OBS::Display::GetPosition()
{
	return m_position;
}

void OBS::Display::SetSize(uint32_t width, uint32_t height)
{
	if (m_source != NULL) {
		blog(LOG_DEBUG, "<" __FUNCTION__
		     "> Adjusting display size for source %s to %ldx%ld.",
		     obs_source_get_name(m_source), width, height);
	}

	// Resize Window
#if defined(_WIN32)
	SetWindowPos((HWND)(m_gsInitData.window.hwnd), NULL,
	             m_position.first, m_position.second, width, height,
	             SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOACTIVATE);
#elif defined(__APPLE__)
#elif defined(__linux__) || defined(__FreeBSD__)
#endif

// Resize Display
	obs_display_resize(m_display, width, height);

	// Store new size.
	m_gsInitData.cx = width;
	m_gsInitData.cy = height;
	UpdatePreviewArea();
}

std::pair<uint32_t, uint32_t> OBS::Display::GetSize()
{
	return std::make_pair(m_gsInitData.cx, m_gsInitData.cy);
}

std::pair<int32_t, int32_t> OBS::Display::GetPreviewOffset()
{
	return m_previewOffset;
}

std::pair<uint32_t, uint32_t> OBS::Display::GetPreviewSize()
{
	return m_previewSize;
}

void OBS::Display::SetDrawUI(bool v /*= true*/)
{
	m_shouldDrawUI = v;
}

bool OBS::Display::GetDrawUI()
{
	return m_shouldDrawUI;
}

void OBS::Display::SetPaddingColor(uint8_t r, uint8_t g, uint8_t b,
                                   uint8_t a /*= 255u*/)
{
	m_paddingColor[0] = float_t(r) / 255.0f;
	m_paddingColor[1] = float_t(g) / 255.0f;
	m_paddingColor[2] = float_t(b) / 255.0f;
	m_paddingColor[3] = float_t(a) / 255.0f;
}

void OBS::Display::SetPaddingSize(uint32_t pixels)
{
	m_paddingSize = pixels;
	UpdatePreviewArea();
}

void OBS::Display::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b,
                                      uint8_t a /*= 255u*/)
{
	m_backgroundColor = a << 24 | b << 16 | g << 8 | r;
}

void OBS::Display::SetOutlineColor(uint8_t r, uint8_t g, uint8_t b,
                                   uint8_t a /*= 255u*/)
{
	m_outlineColor = a << 24 | b << 16 | g << 8 | r;
}

void OBS::Display::SetGuidelineColor(uint8_t r, uint8_t g, uint8_t b,
                                     uint8_t a /*= 255u*/)
{
	m_guidelineColor = a << 24 | b << 16 | g << 8 | r;
}

void OBS::Display::SetResizeBoxOuterColor(uint8_t r, uint8_t g, uint8_t b,
                                          uint8_t a /*= 255u*/)
{
	m_resizeOuterColor = a << 24 | b << 16 | g << 8 | r;
}

void OBS::Display::SetResizeBoxInnerColor(uint8_t r, uint8_t g, uint8_t b,
                                          uint8_t a /*= 255u*/)
{
	m_resizeInnerColor = a << 24 | b << 16 | g << 8 | r;
}

static void DrawGlyph(GS::VertexBuffer *vb, float_t x, float_t y, float_t scale,
                      float_t depth, char glyph, uint32_t color)
{
	// I'll be fully honest here, this code is pretty much shit. It works but
	//  it is far from ideal and can just render very basic text. It does the
	//  job but, well, lets just say it shouldn't be used for other things.

	float uvX = 0, uvY = 0, uvO = 1.0 / 4.0;
	switch (glyph) {
	default:
		return;
		break;
	case '1':
		uvX = 0;
		uvY = 0;
		break;
	case '2':
		uvX = uvO;
		uvY = 0;
		break;
	case '3':
		uvX = uvO * 2;
		uvY = 0;
		break;
	case '4':
		uvX = uvO * 3;
		uvY = 0;
		break;
	case '5':
		uvX = 0;
		uvY = uvO * 1;
		break;
	case '6':
		uvX = uvO;
		uvY = uvO * 1;
		break;
	case '7':
		uvX = uvO * 2;
		uvY = uvO * 1;
		break;
	case '8':
		uvX = uvO * 3;
		uvY = uvO * 1;
		break;
	case '9':
		uvX = 0;
		uvY = uvO * 2;
		break;
	case '0':
		uvX = uvO;
		uvY = uvO * 2;
		break;
	case 'p':
		uvX = uvO * 2;
		uvY = uvO * 2;
		break;
	case 'x':
		uvX = uvO * 3;
		uvY = uvO * 2;
		break;
	}

	GS::Vertex v(nullptr, nullptr, nullptr, nullptr, nullptr);
	size_t bs = vb->Size();
	vb->Resize(uint32_t(bs + 6));

	// Top Left
	v = vb->At(uint32_t(bs + 0));
	vec3_set(v.position, x, y, depth);
	vec4_set(v.uv[0], uvX, uvY, 0, 0);
	*v.color = color;
	// Top Right
	v = vb->At(uint32_t(bs + 1));
	vec3_set(v.position, x + scale, y, depth);
	vec4_set(v.uv[0], uvX + uvO, uvY, 0, 0);
	*v.color = color;
	// Bottom Left
	v = vb->At(uint32_t(bs + 2));
	vec3_set(v.position, x, y + scale * 2, depth);
	vec4_set(v.uv[0], uvX, uvY + uvO, 0, 0);
	*v.color = color;

	// Top Right
	v = vb->At(uint32_t(bs + 3));
	vec3_set(v.position, x + scale, y, depth);
	vec4_set(v.uv[0], uvX + uvO, uvY, 0, 0);
	*v.color = color;
	// Bottom Left
	v = vb->At(uint32_t(bs + 4));
	vec3_set(v.position, x, y + scale * 2, depth);
	vec4_set(v.uv[0], uvX, uvY + uvO, 0, 0);
	*v.color = color;
	// Bottom Right
	v = vb->At(uint32_t(bs + 5));
	vec3_set(v.position, x + scale, y + scale * 2, depth);
	vec4_set(v.uv[0], uvX + uvO, uvY + uvO, 0, 0);
	*v.color = color;
}

#define HANDLE_RADIUS 5.0f
#define HANDLE_DIAMETER 10.0f

inline bool CloseFloat(float a, float b, float epsilon = 0.01)
{
	return std::abs(a - b) <= epsilon;
}

inline void DrawOutline(OBS::Display *dp, matrix4 &mtx,
                        obs_transform_info &info)
{
	gs_matrix_push();
	gs_matrix_set(&mtx);
	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();
}

inline void DrawBoxAt(OBS::Display *dp, float_t x, float_t y, matrix4 &mtx)
{
	gs_matrix_push();

	vec3 pos = { x, y, 0.0f };
	vec3_transform(&pos, &pos, &mtx);

	vec3 offset = { -HANDLE_RADIUS, -HANDLE_RADIUS, 0.0f };
	offset.x *= dp->m_previewToWorldScale.x;
	offset.y *= dp->m_previewToWorldScale.y;

	gs_matrix_translate(&pos);
	gs_matrix_translate(&offset);
	gs_matrix_scale3f(
	      HANDLE_DIAMETER * dp->m_previewToWorldScale.x,
	      HANDLE_DIAMETER * dp->m_previewToWorldScale.y,
	      1.0f);

	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();
}

inline void DrawSquareAt(OBS::Display *dp, float_t x, float_t y, matrix4 &mtx)
{
	gs_matrix_push();

	vec3 pos = { x, y, 0.0f };
	vec3_transform(&pos, &pos, &mtx);

	vec3 offset = { -HANDLE_RADIUS, -HANDLE_RADIUS, 0.0f };
	offset.x *= dp->m_previewToWorldScale.x;
	offset.y *= dp->m_previewToWorldScale.y;


	gs_matrix_translate(&pos);
	gs_matrix_translate(&offset);
	gs_matrix_scale3f(
	      HANDLE_DIAMETER * dp->m_previewToWorldScale.x,
	      HANDLE_DIAMETER * dp->m_previewToWorldScale.y,
	      1.0f);

	gs_draw(GS_TRISTRIP, 0, 0);
	gs_matrix_pop();
}

inline void DrawGuideline(OBS::Display *dp, float_t x, float_t y, matrix4 &mtx)
{
	gs_rect rect;
	rect.x = dp->GetPreviewOffset().first;
	rect.y = dp->GetPreviewOffset().second;
	rect.cx = dp->GetPreviewSize().first;
	rect.cy = dp->GetPreviewSize().second;

	gs_set_scissor_rect(&rect);
	gs_matrix_push();

	vec3 center = { 0.5, 0.5, 0.0f };
	vec3_transform(&center, &center, &mtx);

	vec3 pos = { x, y, 0.0f };
	vec3_transform(&pos, &pos, &mtx);

	vec3 normal;
	vec3_sub(&normal, &center, &pos);
	vec3_norm(&normal, &normal);

	gs_matrix_translate(&pos);

	vec3 up = { 0, 1.0, 0 };
	vec3 dn = { 0, -1.0, 0 };
	vec3 lt = { -1.0, 0, 0 };
	vec3 rt = { 1.0, 0, 0 };

	if (vec3_dot(&up, &normal) > 0.5f) {
		// Dominantly looking up.
		gs_matrix_rotaa4f(0, 0, 1, RAD(-90.0f));
	} else if (vec3_dot(&dn, &normal) > 0.5f) {
		// Dominantly looking down.
		gs_matrix_rotaa4f(0, 0, 1, RAD(90.0f));
	} else if (vec3_dot(&lt, &normal) > 0.5f) {
		// Dominantly looking left.
		gs_matrix_rotaa4f(0, 0, 1, RAD(0.0f));
	} else if (vec3_dot(&rt, &normal) > 0.5f) {
		// Dominantly looking right.
		gs_matrix_rotaa4f(0, 0, 1, RAD(180.0f));
	}

	gs_matrix_scale3f(65535, 65535, 65535);

	gs_draw(GS_LINES, 0, 2);

	gs_matrix_pop();
	gs_set_scissor_rect(nullptr);
}

bool OBS::Display::DrawSelectedSource(obs_scene_t *scene, obs_sceneitem_t *item,
                                      void *param)
{
	// This is partially code from OBS Studio. See window-basic-preview.cpp in obs-studio for copyright/license.
	if (obs_sceneitem_locked(item))
		return true;

	obs_source_t *itemSource = obs_sceneitem_get_source(item);
	uint32_t flags = obs_source_get_output_flags(itemSource);
	bool isOnlyAudio = (flags & OBS_SOURCE_VIDEO) == 0;

	obs_source_t *sceneSource = obs_scene_get_source(scene);

	uint32_t sceneWidth = obs_source_get_width(
	                            sceneSource); // Xaymar: this actually works \o/
	uint32_t sceneHeight = obs_source_get_height(sceneSource);
	uint32_t itemWidth = obs_source_get_width(itemSource);
	uint32_t itemHeight = obs_source_get_height(itemSource);

	if (!obs_sceneitem_selected(item) || isOnlyAudio || ((itemWidth <= 0)
	                                                     && (itemHeight <= 0)))
		return true;

	matrix4 boxTransform;
	matrix4 invBoxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);
	matrix4_inv(&invBoxTransform, &boxTransform);

	{
		vec3 bounds[] = {
			{ { { 0.f, 0.f, 0.f } } },
			{ { { 1.f, 0.f, 0.f } } },
			{ { { 0.f, 1.f, 0.f } } },
			{ { { 1.f, 1.f, 0.f } } },
		};
		bool visible = std::all_of(std::begin(bounds), std::end(bounds),
		[&](const vec3 &b) {
			vec3 pos;
			vec3_transform(&pos, &b, &boxTransform);
			vec3_transform(&pos, &pos, &invBoxTransform);
			return CloseFloat(pos.x, b.x) && CloseFloat(pos.y, b.y);
		});

		if (!visible)
			return true;
	}

	OBS::Display *dp = reinterpret_cast<OBS::Display *>(param);

	vec4 color;
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *solid_color = gs_effect_get_param_by_name(solid, "color");

	obs_transform_info info;
	obs_sceneitem_get_info(item, &info);

	gs_load_vertexbuffer(dp->m_boxLine->Update(false));
	vec4_set(&color,
	         (dp->m_outlineColor & 0xFF) / 255.0f,
	         ((dp->m_outlineColor & 0xFF00) >> 8) / 255.0f,
	         ((dp->m_outlineColor & 0xFF0000) >> 16) / 255.0f,
	         ((dp->m_outlineColor & 0xFF000000) >> 24) / 255.0f);
	gs_effect_set_vec4(solid_color, &color);
	DrawOutline(dp, boxTransform, info);

	gs_load_vertexbuffer(dp->m_boxTris->Update(false));
	vec4_set(&color,
	         (dp->m_resizeInnerColor & 0xFF) / 255.0f,
	         ((dp->m_resizeInnerColor & 0xFF00) >> 8) / 255.0f,
	         ((dp->m_resizeInnerColor & 0xFF0000) >> 16) / 255.0f,
	         ((dp->m_resizeInnerColor & 0xFF000000) >> 24) / 255.0f);
	gs_effect_set_vec4(solid_color, &color);
	DrawSquareAt(dp, 0, 0, boxTransform);
	DrawSquareAt(dp, 1, 0, boxTransform);
	DrawSquareAt(dp, 0, 1, boxTransform);
	DrawSquareAt(dp, 1, 1, boxTransform);
	DrawSquareAt(dp, 0.5, 0, boxTransform);
	DrawSquareAt(dp, 0.5, 1, boxTransform);
	DrawSquareAt(dp, 0, 0.5, boxTransform);
	DrawSquareAt(dp, 1, 0.5, boxTransform);

	gs_load_vertexbuffer(dp->m_boxLine->Update(false));
	vec4_set(&color,
	         (dp->m_resizeOuterColor & 0xFF) / 255.0f,
	         ((dp->m_resizeOuterColor & 0xFF00) >> 8) / 255.0f,
	         ((dp->m_resizeOuterColor & 0xFF0000) >> 16) / 255.0f,
	         ((dp->m_resizeOuterColor & 0xFF000000) >> 24) / 255.0f);
	gs_effect_set_vec4(solid_color, &color);
	DrawBoxAt(dp, 0, 0, boxTransform);
	DrawBoxAt(dp, 1, 0, boxTransform);
	DrawBoxAt(dp, 0, 1, boxTransform);
	DrawBoxAt(dp, 1, 1, boxTransform);
	DrawBoxAt(dp, 0.5, 0, boxTransform);
	DrawBoxAt(dp, 0.5, 1, boxTransform);
	DrawBoxAt(dp, 0, 0.5, boxTransform);
	DrawBoxAt(dp, 1, 0.5, boxTransform);

	if (dp->m_drawGuideLines) {
		vec4_set(&color,
		         (dp->m_guidelineColor & 0xFF) / 255.0f,
		         ((dp->m_guidelineColor & 0xFF00) >> 8) / 255.0f,
		         ((dp->m_guidelineColor & 0xFF0000) >> 16) / 255.0f,
		         ((dp->m_guidelineColor & 0xFF000000) >> 24) / 255.0f);
		gs_effect_set_vec4(solid_color, &color);
		DrawGuideline(dp, 0.5, 0, boxTransform);
		DrawGuideline(dp, 0.5, 1, boxTransform);
		DrawGuideline(dp, 0, 0.5, boxTransform);
		DrawGuideline(dp, 1, 0.5, boxTransform);

		// TEXT RENDERING
		// THIS DESPERATELY NEEDS TO BE REWRITTEN INTO SHADER CODE
		// DO SO WHENEVER...
		GS::Vertex v(nullptr, nullptr, nullptr, nullptr, nullptr);

		matrix4 itemMatrix, sceneToView;
		obs_sceneitem_get_box_transform(item, &itemMatrix);
		matrix4_identity(&sceneToView);
		sceneToView.x.x = dp->m_worldToPreviewScale.x;
		sceneToView.y.y = dp->m_worldToPreviewScale.y;

		// Retrieve actual corner and edge positions.
		vec3 edge[4], center;
		{
			vec3_set(&edge[0], 0, 0.5, 0);
			vec3_transform(&edge[0], &edge[0], &itemMatrix);
			vec3_set(&edge[1], 0.5, 0, 0);
			vec3_transform(&edge[1], &edge[1], &itemMatrix);
			vec3_set(&edge[2], 1, 0.5, 0);
			vec3_transform(&edge[2], &edge[2], &itemMatrix);
			vec3_set(&edge[3], 0.5, 1, 0);
			vec3_transform(&edge[3], &edge[3], &itemMatrix);

			vec3_set(&center, 0.5, 0.5, 0);
			vec3_transform(&center, &center, &itemMatrix);;
		}

		std::vector<char> buf(8);
		float_t pt = 8 * dp->m_previewToWorldScale.y;
		for (size_t n = 0; n < 4; n++) {
			bool isIn = (edge[n].x >= 0) && (edge[n].x < sceneWidth)
			            && (edge[n].y >= 0) && (edge[n].y < sceneHeight);

			if (!isIn)
				continue;

			vec3 alignLeft = { -1, 0, 0 };
			vec3 alignTop = { 0, -1, 0 };

			vec3 temp;
			vec3_sub(&temp, &edge[n], &center);
			vec3_norm(&temp, &temp);
			float left = vec3_dot(&temp, &alignLeft),
			      top = vec3_dot(&temp, &alignTop);
			if (left > 0.5) { // LEFT
				float_t dist = edge[n].x;
				if (dist > (pt * 4)) {
					size_t len = (size_t)snprintf(buf.data(), buf.size(), "%ld px", (uint32_t)dist);
					float_t offset = float((pt * len) / 2.0);

					for (size_t p = 0; p < len; p++) {
						char v = buf.data()[p];
						DrawGlyph(dp->m_textVertices,
						          (edge[n].x / 2) - offset + (p * pt), edge[n].y - pt * 2,
						          pt, 0, v, dp->m_guidelineColor);
					}
				}
			} else if (left < -0.5) { // RIGHT
				float_t dist = sceneWidth - edge[n].x;
				if (dist >(pt * 4)) {
					size_t len = (size_t)snprintf(buf.data(), buf.size(), "%ld px", (uint32_t)dist);
					float_t offset = float((pt * len) / 2.0);

					for (size_t p = 0; p < len; p++) {
						char v = buf.data()[p];
						DrawGlyph(dp->m_textVertices,
						          edge[n].x + (dist / 2) - offset + (p * pt), edge[n].y - pt * 2,
						          pt, 0, v, dp->m_guidelineColor);
					}
				}
			} else if (top > 0.5) { // UP
				float_t dist = edge[n].y;
				if (dist > pt) {
					size_t len = (size_t)snprintf(buf.data(), buf.size(), "%ld px", (uint32_t)dist);
					float_t offset = float((pt * len) / 2.0);

					for (size_t p = 0; p < len; p++) {
						char v = buf.data()[p];
						DrawGlyph(dp->m_textVertices,
						          edge[n].x + (p * pt), edge[n].y - (dist / 2) - pt,
						          pt, 0, v, dp->m_guidelineColor);
					}
				}
			} else if (top < -0.5) { // DOWN
				float_t dist = sceneHeight - edge[n].y;
				if (dist >(pt * 4)) {
					size_t len = (size_t)snprintf(buf.data(), buf.size(), "%ld px", (uint32_t)dist);
					float_t offset = float((pt * len) / 2.0);

					for (size_t p = 0; p < len; p++) {
						char v = buf.data()[p];
						DrawGlyph(dp->m_textVertices,
						          edge[n].x + (p * pt), edge[n].y + (dist / 2) - pt,
						          pt, 0, v, dp->m_guidelineColor);
					}
				}
			}
		}
	}

	return true;
}

void OBS::Display::DisplayCallback(OBS::Display *dp, uint32_t cx, uint32_t cy)
{
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *solid_color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *solid_tech = gs_effect_get_technique(solid, "Solid");
	vec4 color;

	dp->UpdatePreviewArea();

	// Get proper source/base size.
	uint32_t sourceW, sourceH;
	if (dp->m_source) {
		sourceW = obs_source_get_width(dp->m_source);
		sourceH = obs_source_get_height(dp->m_source);
		if (sourceW == 0) sourceW = 1;
		if (sourceH == 0) sourceH = 1;
	} else {
		obs_video_info ovi;
		obs_get_video_info(&ovi);

		sourceW = ovi.base_width;
		sourceH = ovi.base_height;
		if (sourceW == 0) sourceW = 1;
		if (sourceH == 0) sourceH = 1;
	}

	gs_viewport_push();
	gs_projection_push();

	gs_ortho(0.0f, float(sourceW), 0.0f, float(sourceH), -100.0f, 100.0f);
	gs_set_viewport(dp->m_previewOffset.first, dp->m_previewOffset.second,
	                dp->m_previewSize.first, dp->m_previewSize.second);

	// Padding
	vec4_set(&color, dp->m_paddingColor[0], dp->m_paddingColor[1],
	         dp->m_paddingColor[2], dp->m_paddingColor[3]);
	gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH | GS_CLEAR_STENCIL, &color, 100, 0);

	// Background
	if (dp->m_boxTris) {
		vec4_set(&color,
		         ((dp->m_backgroundColor & 0xFF)) / 255.0f,
		         ((dp->m_backgroundColor & 0xFF00) >> 8) / 255.0f,
		         ((dp->m_backgroundColor & 0xFF0000) >> 16) / 255.0f,
		         ((dp->m_backgroundColor & 0xFF000000) >> 24) / 255.0f);
		gs_effect_set_vec4(solid_color, &color);

		gs_technique_begin(solid_tech);
		gs_technique_begin_pass(solid_tech, 0);

		gs_matrix_push();
		gs_matrix_identity();
		gs_matrix_scale3f(float(sourceW), float(sourceH), 1.0f);

		gs_load_vertexbuffer(dp->m_boxTris->Update(false));
		gs_draw(GS_TRISTRIP, 0, 0);

		gs_matrix_pop();

		gs_technique_end_pass(solid_tech);
		gs_technique_end(solid_tech);

		gs_load_vertexbuffer(nullptr);
	}

	// Source Rendering
	if (dp->m_source)
		obs_source_video_render(dp->m_source);

	else
		obs_render_main_view();
	gs_load_vertexbuffer(nullptr);

	if (!dp->m_source && dp->m_shouldDrawUI == true) {
		// Display-Aligned Drawing
		vec2 tlCorner = { (float)-dp->m_previewOffset.first, (float)-dp->m_previewOffset.second };
		vec2 brCorner = { (float)(cx - dp->m_previewOffset.first), (float)(cy - dp->m_previewOffset.second) };
		vec2_mul(&tlCorner, &tlCorner, &dp->m_previewToWorldScale);
		vec2_mul(&brCorner, &brCorner, &dp->m_previewToWorldScale);

		gs_ortho(
		      tlCorner.x, brCorner.x,
		      tlCorner.y, brCorner.y,
		      -100.0f, 100.0f);
		gs_reset_viewport();

		/* Here we assume that channel 0 holds the one and only transition.
		 * We also assume that the active source within that transition is
		 * the scene that we need */
		obs_source_t *transition = obs_get_output_source(0);
		obs_source_t *source = obs_transition_get_active_source(transition);
		obs_source_release(transition);
		obs_scene_t *scene = obs_scene_from_source(source);

		if (scene) {
			dp->m_textVertices->Resize(0);

			gs_technique_begin(solid_tech);
			gs_technique_begin_pass(solid_tech, 0);

			obs_scene_enum_items(scene, DrawSelectedSource, dp);
			gs_load_vertexbuffer(nullptr);

			gs_technique_end_pass(solid_tech);
			gs_technique_end(solid_tech);

			// Text Rendering
			if (dp->m_textVertices->Size() > 0) {
				gs_vertbuffer_t *vb = dp->m_textVertices->Update();
				while (gs_effect_loop(dp->m_textEffect, "Draw")) {
					gs_effect_set_texture(
					      gs_effect_get_param_by_name(dp->m_textEffect, "image"),
					      dp->m_textTexture);
					gs_load_vertexbuffer(vb);
					gs_load_indexbuffer(nullptr);
					gs_draw(GS_TRIS, 0, (uint32_t)dp->m_textVertices->Size());
				}
			}
		}

		obs_source_release(source);
	}

	gs_projection_pop();
	gs_viewport_pop();
}

void OBS::Display::UpdatePreviewArea()
{
	int32_t offsetX = 0, offsetY = 0;
	uint32_t sourceW, sourceH;
	if (m_source) {
		sourceW = obs_source_get_width(m_source);
		sourceH = obs_source_get_height(m_source);
		if (sourceW == 0) sourceW = 1;
		if (sourceH == 0) sourceH = 1;
	} else {
		obs_video_info ovi;
		obs_get_video_info(&ovi);

		sourceW = ovi.base_width;
		sourceH = ovi.base_height;

		if (sourceW == 0) sourceW = 1;
		if (sourceH == 0) sourceH = 1;

		offsetX = grayPaddingArea;
		offsetY = grayPaddingArea;
	}

	RecalculateApectRatioConstrainedSize(
	      m_gsInitData.cx, m_gsInitData.cy,
	      sourceW, sourceH,
	      m_previewOffset.first, m_previewOffset.second, m_previewSize.first,
	      m_previewSize.second
	);

	m_previewOffset.first += offsetX;
	m_previewOffset.second += offsetY;
	m_previewSize.first -= offsetX * 2;
	m_previewSize.second -= offsetY * 2;
	m_worldToPreviewScale.x = float_t(m_previewSize.first) / float_t(sourceW);
	m_worldToPreviewScale.y = float_t(m_previewSize.second) / float_t(sourceH);
	m_previewToWorldScale.x = float_t(sourceW) / float_t(m_previewSize.first);
	m_previewToWorldScale.y = float_t(sourceH) / float_t(m_previewSize.second);
}

#if defined(_WIN32)

bool OBS::Display::DisplayWndClassRegistered;

WNDCLASSEX OBS::Display::DisplayWndClassObj;

ATOM OBS::Display::DisplayWndClassAtom;

void OBS::Display::DisplayWndClass()
{
	if (DisplayWndClassRegistered)
		return;

	DisplayWndClassObj.cbSize = sizeof(WNDCLASSEX);
	DisplayWndClassObj.style = CS_OWNDC | CS_NOCLOSE | CS_HREDRAW |
	                           CS_VREDRAW;// CS_DBLCLKS | CS_HREDRAW | CS_NOCLOSE | CS_VREDRAW | CS_OWNDC;
	DisplayWndClassObj.lpfnWndProc = DisplayWndProc;
	DisplayWndClassObj.cbClsExtra = 0;
	DisplayWndClassObj.cbWndExtra = 0;
	DisplayWndClassObj.hInstance = NULL;// HINST_THISCOMPONENT;
	DisplayWndClassObj.hIcon = NULL;
	DisplayWndClassObj.hCursor = NULL;
	DisplayWndClassObj.hbrBackground = NULL;
	DisplayWndClassObj.lpszMenuName = NULL;
	DisplayWndClassObj.lpszClassName = TEXT("Win32DisplayClass");
	DisplayWndClassObj.hIconSm = NULL;

	DisplayWndClassAtom = RegisterClassEx(&DisplayWndClassObj);
	if (DisplayWndClassAtom == NULL) {
		DWORD errorCode = GetLastError();
		LPSTR errorStr = nullptr;
		DWORD errorStrSize = 16;
		DWORD errorStrLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		                                  FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, LANG_USER_DEFAULT, errorStr,
		                                  errorStrSize, NULL);
		//    MessageBox((HWND)windowHandle, errorStr, "Unexpected Runtime Error", MB_OK | MB_ICONERROR);
		std::string exceptionMessage(errorStr, errorStrLen);
		exceptionMessage = "Unexpected WinAPI error: " + exceptionMessage;
		LocalFree(errorStr);

		throw std::system_error(errorCode, std::system_category(), exceptionMessage);
	}

	DisplayWndClassRegistered = true;
}

LRESULT CALLBACK OBS::Display::DisplayWndProc(_In_ HWND hwnd, _In_ UINT uMsg,
                                              _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg) {
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif

bool OBS::Display::GetDrawGuideLines(void)
{
	return m_drawGuideLines;
}

void OBS::Display::SetDrawGuideLines(bool drawGuideLines)
{
	m_drawGuideLines = drawGuideLines;
}

