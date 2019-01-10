#include "nodeobs_display.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

static BOOL CALLBACK EnumChromeWindowsProc(HWND hwnd, LPARAM lParam)
{
	char buf[256];
	if (GetClassNameA(hwnd, buf, sizeof(buf) / sizeof(*buf))) {
		if (strstr(buf, "Intermediate D3D Window")) {
			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			if ((style & WS_CLIPSIBLINGS) == 0) {
				style |= WS_CLIPSIBLINGS;
				SetWindowLongPtr(hwnd, GWL_STYLE, style);
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
		SetWindowLongPtr(chromeWindow, GWL_STYLE, style);
	}
}

void display::OBS_content_createDisplay(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> bufferObj    = args[0].As<v8::Object>();
	unsigned char*        bufferData   = (unsigned char*)node::Buffer::Data(bufferObj);
	uint64_t              windowHandle = *reinterpret_cast<uint64_t*>(bufferData);

	FixChromeD3DIssue((HWND)windowHandle);

	std::string key;
	ASSERT_GET_VALUE(args[1], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display", "OBS_content_createDisplay", {ipc::value(windowHandle), ipc::value(key)});

	ValidateResponse(response);
}

void display::OBS_content_destroyDisplay(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;

	ASSERT_GET_VALUE(args[0], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_destroyDisplay", {ipc::value(key)});

	ValidateResponse(response);
}

void display::OBS_content_getDisplayPreviewOffset(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;

	ASSERT_GET_VALUE(args[0], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_getDisplayPreviewOffset", {ipc::value(key)});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Object> previewOffset = v8::Object::New(args.GetIsolate());

	utilv8::SetObjectField(previewOffset, "x", response[1].value_union.i32);
	utilv8::SetObjectField(previewOffset, "y", response[2].value_union.i32);

	args.GetReturnValue().Set(previewOffset);

	return;
}

void display::OBS_content_getDisplayPreviewSize(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;

	ASSERT_GET_VALUE(args[0], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_getDisplayPreviewSize", {ipc::value(key)});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Object> previewOffset = v8::Object::New(args.GetIsolate());

	utilv8::SetObjectField(previewOffset, "width", response[1].value_union.i32);
	utilv8::SetObjectField(previewOffset, "height", response[2].value_union.i32);

	args.GetReturnValue().Set(previewOffset);

	return;
}

void display::OBS_content_createSourcePreviewDisplay(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate*             isolate = args.GetIsolate();
	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Object> bufferObj    = args[0].As<v8::Object>();
	unsigned char*        bufferData   = (unsigned char*)node::Buffer::Data(bufferObj);
	uint64_t              windowHandle = *reinterpret_cast<uint64_t*>(bufferData);

	FixChromeD3DIssue((HWND)windowHandle);

	std::string sourceName, key;
	ASSERT_GET_VALUE(args[1], sourceName);
	ASSERT_GET_VALUE(args[2], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_createSourcePreviewDisplay",
	    {ipc::value(windowHandle), ipc::value(sourceName), ipc::value(key)});

	ValidateResponse(response);
}

void display::OBS_content_resizeDisplay(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	double_t    width_d, height_d;
	uint32_t    width, height;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], width_d);
	ASSERT_GET_VALUE(args[2], height_d);

	width  = uint32_t(width_d);
	height = uint32_t(height_d);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display", "OBS_content_resizeDisplay", {ipc::value(key), ipc::value(width), ipc::value(height)});

	ValidateResponse(response);
}

void display::OBS_content_moveDisplay(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	double_t    x_d, y_d;
	uint32_t    x, y;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], x_d);
	ASSERT_GET_VALUE(args[2], y_d);

	x = uint32_t(x_d);
	y = uint32_t(y_d);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display", "OBS_content_moveDisplay", {ipc::value(key), ipc::value(x), ipc::value(y)});

	ValidateResponse(response);
}

void display::OBS_content_setPaddingSize(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    paddingSize;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], paddingSize);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display", "OBS_content_setPaddingSize", {ipc::value(key), ipc::value(paddingSize)});

	ValidateResponse(response);
}

void display::OBS_content_setPaddingColor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    r, g, b, a = 255;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], r);
	ASSERT_GET_VALUE(args[2], g);
	ASSERT_GET_VALUE(args[3], b);

	if (args.Length() > 4)
		ASSERT_GET_VALUE(args[4], a);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_setPaddingColor",
	    {ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)});

	ValidateResponse(response);
}

void display::OBS_content_setOutlineColor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    r, g, b, a = 255;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], r);
	ASSERT_GET_VALUE(args[2], g);
	ASSERT_GET_VALUE(args[3], b);

	if (args.Length() > 4)
		ASSERT_GET_VALUE(args[4], a);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_setOutlineColor",
	    {ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)});

	ValidateResponse(response);
}

void display::OBS_content_setGuidelineColor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    r, g, b, a = 255;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], r);
	ASSERT_GET_VALUE(args[2], g);
	ASSERT_GET_VALUE(args[3], b);

	if (args.Length() > 4)
		ASSERT_GET_VALUE(args[4], a);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_setGuidelineColor",
	    {ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)});

	ValidateResponse(response);
}

void display::OBS_content_setResizeBoxInnerColor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    r, g, b, a = 255;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], r);
	ASSERT_GET_VALUE(args[2], g);
	ASSERT_GET_VALUE(args[3], b);

	if (args.Length() > 4)
		ASSERT_GET_VALUE(args[4], a);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_setResizeBoxInnerColor",
	    {ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)});

	ValidateResponse(response);
}

void display::OBS_content_setResizeBoxOuterColor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	uint32_t    r, g, b, a = 255;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], r);
	ASSERT_GET_VALUE(args[2], g);
	ASSERT_GET_VALUE(args[3], b);

	if (args.Length() > 4)
		ASSERT_GET_VALUE(args[4], a);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display",
	    "OBS_content_setResizeBoxOuterColor",
	    {ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)});

	ValidateResponse(response);
}

void display::OBS_content_setShouldDrawUI(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	bool        drawUI;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], drawUI);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_setShouldDrawUI", {ipc::value(key), ipc::value(drawUI)});

	ValidateResponse(response);
}

void display::OBS_content_selectSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	uint32_t x, y;

	ASSERT_GET_VALUE(args[0], x);
	ASSERT_GET_VALUE(args[1], y);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_selectSource", {ipc::value(x), ipc::value(y)});

	ValidateResponse(response);
}

void display::OBS_content_getDrawGuideLines(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;

	ASSERT_GET_VALUE(args[0], key);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Display", "OBS_content_getDrawGuideLines", {ipc::value(key)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(Nan::New<v8::Boolean>(response[1].value_union.ui32));
}

void display::OBS_content_setDrawGuideLines(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string key;
	bool        drawGuideLines;

	ASSERT_GET_VALUE(args[0], key);
	ASSERT_GET_VALUE(args[1], drawGuideLines);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Display", "OBS_content_setDrawGuideLines", {ipc::value(key), ipc::value(drawGuideLines)});

	ValidateResponse(response);
}

INITIALIZER(nodeobs_display)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_content_createDisplay", display::OBS_content_createDisplay);
		NODE_SET_METHOD(exports, "OBS_content_destroyDisplay", display::OBS_content_destroyDisplay);
		NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewOffset", display::OBS_content_getDisplayPreviewOffset);
		NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewSize", display::OBS_content_getDisplayPreviewSize);
		NODE_SET_METHOD(
		    exports, "OBS_content_createSourcePreviewDisplay", display::OBS_content_createSourcePreviewDisplay);
		NODE_SET_METHOD(exports, "OBS_content_resizeDisplay", display::OBS_content_resizeDisplay);
		NODE_SET_METHOD(exports, "OBS_content_moveDisplay", display::OBS_content_moveDisplay);
		NODE_SET_METHOD(exports, "OBS_content_setPaddingSize", display::OBS_content_setPaddingSize);
		NODE_SET_METHOD(exports, "OBS_content_setPaddingColor", display::OBS_content_setPaddingColor);
		NODE_SET_METHOD(exports, "OBS_content_setGuidelineColor", display::OBS_content_setGuidelineColor);
		NODE_SET_METHOD(exports, "OBS_content_setResizeBoxInnerColor", display::OBS_content_setResizeBoxInnerColor);
		NODE_SET_METHOD(exports, "OBS_content_setResizeBoxOuterColor", display::OBS_content_setResizeBoxOuterColor);
		NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", display::OBS_content_setShouldDrawUI);
		NODE_SET_METHOD(exports, "OBS_content_selectSource", display::OBS_content_selectSource);
		NODE_SET_METHOD(exports, "OBS_content_getDrawGuideLines", display::OBS_content_getDrawGuideLines);
		NODE_SET_METHOD(exports, "OBS_content_setDrawGuideLines", display::OBS_content_setDrawGuideLines);
	});
}
