#pragma once

#include "obspp/obspp-display.hpp"
#include "Common.h"

#include <nan.h>

namespace osn
{

struct VertexHelper {
	VertexHelper();
	vec3 pos, normal, tangent;
	uint32_t color;
	vec2 uv0, uv1;
};

class VertexBufferHelper
{
private:
	gs_vertbuffer_t *vb;
	gs_vb_data *vbdata;
	std::vector<VertexHelper> vertices;
	std::vector<gs_tvertarray> vbuvdata;
	std::vector<vec3> t_vertices, t_normals, t_tangents;
	std::vector<uint32_t> t_colors;
	std::vector<vec2> t_uv0, t_uv1;
public:
	VertexBufferHelper(size_t initialSize = 65535);
	virtual ~VertexBufferHelper();
	void clear();
	VertexHelper *add();
	gs_vertbuffer_t *update();
	size_t size();
};

class Display : public Nan::ObjectWrap
{
public:
	static Nan::Persistent<v8::FunctionTemplate> prototype;

	typedef common::Object<Display, obs::display *> Object;
	friend Object;

	obs::display *handle;

	Display(uint64_t windowHandle, obs_source_t *source);
	Display(uint64_t windowHandle);
	Display();
	~Display();

	static void DisplayCallback(void *dp, uint32_t cx, uint32_t cy);
	static bool DrawSelectedSource(obs_scene_t *scene, obs_sceneitem_t *item, void *param);
	void UpdatePreviewArea();

	static NAN_METHOD(setPosition);
	static NAN_METHOD(getPosition);
	static NAN_METHOD(setSize);
	static NAN_METHOD(getSize);

	static NAN_METHOD(getPreviewOffset);
	static NAN_METHOD(getPreviewSize);

	static NAN_SETTER(shouldDrawUI);
	static NAN_GETTER(shouldDrawUI);

	static NAN_GETTER(paddingSize);
	static NAN_SETTER(paddingSize);
	static NAN_METHOD(setPaddingColor);
	static NAN_METHOD(setBackgroundColor);
	static NAN_METHOD(setOutlineColor);
	static NAN_METHOD(setGuidelineColor);
	static NAN_METHOD(setResizeBoxOuterColor);
	static NAN_METHOD(setResizeBoxInnerColor);

private:
	gs_init_data m_gsInitData;
	obs_source_t *m_source;

	// Preview
	/// Window Position
	std::pair<uint32_t, uint32_t> m_position;
	/// Actual Preview Offset into Window
	std::pair<int32_t, int32_t> m_previewOffset;
	/// Actual Preview Size
	std::pair<uint32_t, uint32_t> m_previewSize;
	vec2 m_worldToPreviewScale;

	// OBS Graphics API
	gs_effect_t
	*m_gsSolidEffect,
	*m_textEffect;
	gs_texture_t *m_textTexture;

	VertexBufferHelper
	*m_lines,
	*m_triangles,
	*m_textVertices;

	// Theme/Style
	/// Padding
	uint32_t m_paddingSize = 10;
	std::vector<float_t> m_paddingColor = { 0.1328125, 0.1328125, 0.1328125, 1.0 };
	/// Other
	uint32_t m_backgroundColor = 0xFF000000;
	uint32_t m_outlineColor = 0xFFFF7EFF;
	uint32_t m_guidelineColor = 0xFF0000FF;
	uint32_t m_resizeOuterColor = 0xFF7E7E7E;
	uint32_t m_resizeInnerColor = 0xFFFFFFFF;
	bool m_shouldDrawUI = true;

#if defined(_WIN32)
	HWND m_ourWindow;
#elif defined(__APPLE__)
#elif defined(__linux__) || defined(__FreeBSD__)
#endif

public:
	static NAN_MODULE_INIT(Init);
	static NAN_METHOD(New);

	static NAN_METHOD(create);
	static NAN_METHOD(destroy);
};

};