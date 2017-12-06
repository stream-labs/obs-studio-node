#pragma once

#include <node.h>
#include "obs.h"
#include <system_error>
#include <algorithm>
#include <vector>

#if defined(_WIN32)
#ifdef NOWINOFFSETS
#undef NOWINOFFSETS
#endif
#include <windows.h>
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

#elif defined(__APPLE__)

#elif defined(__linux__) || defined(__FreeBSD__)

#endif

namespace OBS {
	struct VertexHelper {
		VertexHelper();

		vec3 pos, normal, tangent;
		uint32_t color;
		vec2 uv0, uv1;
	};

	class VertexBufferHelper {
		private:
		gs_vertbuffer_t* vb;
		gs_vb_data* vbdata;
		std::vector<VertexHelper> vertices;
		std::vector<gs_tvertarray> vbuvdata;
		std::vector<vec3> t_vertices, t_normals, t_tangents;
		std::vector<uint32_t> t_colors;
		std::vector<vec2> t_uv0, t_uv1;

		public:
		VertexBufferHelper(size_t initialSize = 65535);
		virtual ~VertexBufferHelper();

		void clear();
		VertexHelper* add();
		gs_vertbuffer_t* update();
		size_t size();
	};

	class Display {
		#pragma region Constructors & Finalizer
		private:
		Display();

		public:
		Display(uint64_t windowHandle); // Create a Main Preview one
		Display(uint64_t windowHandle, std::string sourceName); // Create a Source-Specific one
		~Display();
		#pragma endregion Constructors & Finalizer

		void SetPosition(uint32_t x, uint32_t y);
		std::pair<uint32_t, uint32_t> GetPosition();

		void SetSize(uint32_t width, uint32_t height);
		std::pair<uint32_t, uint32_t> GetSize();

		std::pair<int32_t, int32_t> GetPreviewOffset();
		std::pair<uint32_t, uint32_t> GetPreviewSize();

		void SetDrawUI(bool v = true);
		bool GetDrawUI();

		void SetPaddingSize(uint32_t pixels);
		void SetPaddingColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);
		void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);
		void SetOutlineColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);
		void SetGuidelineColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);
		void SetResizeBoxOuterColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);
		void SetResizeBoxInnerColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u);

		private:
		static void DisplayCallback(OBS::Display* displayPtr, uint32_t cx, uint32_t cy);
		static bool DrawSelectedSource(obs_scene_t* scene, obs_sceneitem_t* item, void* param);
		void UpdatePreviewArea();

		private:
		gs_init_data m_gsInitData;
		obs_display_t* m_display;
		obs_source_t* m_source;

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
		static bool DisplayWndClassRegistered;
		static WNDCLASSEX DisplayWndClassObj;
		static ATOM DisplayWndClassAtom;
		static void DisplayWndClass();
		static LRESULT CALLBACK DisplayWndProc(
			_In_ HWND   hwnd,
			_In_ UINT   uMsg,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam
		);
		#elif defined(__APPLE__)
		#elif defined(__linux__) || defined(__FreeBSD__)
		#endif
	};
}

