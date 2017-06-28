#include "Display.h"
#include "RenderPlugin.h"
#include "Common.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace osn {

static bool DisplayWndClassRegistered;

static WNDCLASSEX DisplayWndClassObj;

static ATOM DisplayWndClassAtom;

static LRESULT CALLBACK DisplayWndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (uMsg) {
		case WM_NCHITTEST:
			return HTTRANSPARENT;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void DisplayWndClass() {
	if (DisplayWndClassRegistered)
		return;

	DisplayWndClassObj.cbSize = sizeof(WNDCLASSEX);
	DisplayWndClassObj.style = CS_OWNDC | CS_NOCLOSE | CS_HREDRAW | CS_VREDRAW;// CS_DBLCLKS | CS_HREDRAW | CS_NOCLOSE | CS_VREDRAW | CS_OWNDC;
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
		DWORD errorStrLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, LANG_USER_DEFAULT, errorStr, errorStrSize, NULL);
		//    MessageBox((HWND)windowHandle, errorStr, "Unexpected Runtime Error", MB_OK | MB_ICONERROR);
		std::string exceptionMessage(errorStr, errorStrLen);
		exceptionMessage = "Unexpected WinAPI error: " + exceptionMessage;
		LocalFree(errorStr);

		throw std::system_error(errorCode, std::system_category(), exceptionMessage);
	}

	DisplayWndClassRegistered = true;
}

static BOOL CALLBACK EnumChromeWindowsProc(HWND hwnd, LPARAM lParam) {
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

static void FixChromeD3DIssue(HWND chromeWindow) {
	(void)EnumChildWindows(chromeWindow, EnumChromeWindowsProc, (LPARAM)NULL);
}

Display::Display(gs_init_data &data)
 : handle(data)
{
}

NAN_MODULE_INIT(Display::Init)
{
    auto prototype = Nan::New<v8::FunctionTemplate>(New);
    prototype->InstanceTemplate()->SetInternalFieldCount(1);
    prototype->SetClassName(FIELD_NAME("Display"));
    Nan::SetPrototypeMethod(prototype, "addDrawer", addDrawer);
    Nan::SetPrototypeMethod(prototype, "removeDrawer", removeDrawer);
    Nan::SetPrototypeMethod(prototype, "resize", resize);
    Nan::SetPrototypeMethod(prototype, "destroy", destroy);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("enabled"), enabled, enabled);
    Nan::Set(target, FIELD_NAME("Display"), prototype->GetFunction());
}

static LRESULT CALLBACK sceneProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

NAN_METHOD(Display::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }

    if (!info[0]->IsObject()) {
        Nan::ThrowError("Expected object");
        return;
    }

    if (info.Length() != 1) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    auto init_object = info[0]->ToObject();

    gs_init_data init_data = {};

    DisplayWndClass();

    Nan::TypedArrayContents<uint8_t> hwnd_view(Nan::Get(init_object, FIELD_NAME("hwnd")).ToLocalChecked());
    auto cx_local = Nan::Get(init_object, FIELD_NAME("width")).ToLocalChecked()->ToUint32();
    auto cy_local = Nan::Get(init_object, FIELD_NAME("height")).ToLocalChecked()->ToUint32();
    auto format_local = Nan::Get(init_object, FIELD_NAME("format")).ToLocalChecked()->ToUint32();
    auto zsformat_local = Nan::Get(init_object, FIELD_NAME("zsformat")).ToLocalChecked()->ToUint32();

    init_data.cx = cx_local->Value();
    init_data.cy = cy_local->Value();
    init_data.format = static_cast<gs_color_format>(format_local->Value());
    init_data.zsformat = static_cast<gs_zstencil_format>(zsformat_local->Value());

    /* TODO Currently only handle the Windows scenario */
    if (*hwnd_view) {
        HWND hwnd = (HWND)*reinterpret_cast<uint64_t*>(*hwnd_view);
        init_data.window.hwnd = hwnd;
    }

    FixChromeD3DIssue((HWND)init_data.window.hwnd);
    DisplayWndClass();

    HWND new_window = CreateWindowEx(
        0, //WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        TEXT("Win32DisplayClass"), TEXT("SlobsChildWindowPreview"),
        WS_VISIBLE | WS_POPUP,
        0, 0, init_data.cx, init_data.cy,
        NULL, NULL, NULL, NULL);

    SetParent(new_window, (HWND)init_data.window.hwnd);

    /* RESOURCE LEAK */
    init_data.window.hwnd = new_window;

    Display *object = new Display(init_data);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

#define UNWRAP_DISPLAY \
    Display* object = Nan::ObjectWrap::Unwrap<Display>(info.Holder()); \
    obs::display &handle = object->handle;

NAN_GETTER(Display::status)
{
    UNWRAP_DISPLAY

    info.GetReturnValue().Set(handle.status());
}

NAN_METHOD(Display::destroy)
{
    UNWRAP_DISPLAY

    handle.destroy();
}

NAN_METHOD(Display::addDrawer)
{
    UNWRAP_DISPLAY

    if (!info[0]->IsString()) {
        Nan::ThrowTypeError("Expected string");
        return;
    }

    Nan::Utf8String path(info[0]);

    void *module = osn_load_plugin(handle.dangerous(), *path);

    if (!module) {
        Nan::ThrowError("Failed to find drawer plugin");
        return;
    }
}

NAN_METHOD(Display::removeDrawer)
{
    /* TODO */
}

NAN_METHOD(Display::resize)
{
    UNWRAP_DISPLAY

    if (info.Length() != 2) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsUint32() || !info[1]->IsUint32()) {
        Nan::ThrowTypeError("Expected unsigned integers");
        return;
    }

    uint32_t cx = Nan::To<uint32_t>(info[0]).FromJust();
    uint32_t cy = Nan::To<uint32_t>(info[1]).FromJust();
}

NAN_GETTER(Display::enabled)
{
    UNWRAP_DISPLAY

    info.GetReturnValue().Set(handle.enabled());
}

NAN_SETTER(Display::enabled)
{
    UNWRAP_DISPLAY

    bool is_enabled = Nan::To<bool>(value).FromJust();

    handle.enabled(is_enabled);
}

NAN_SETTER(Display::backgroundColor)
{
    UNWRAP_DISPLAY

    uint32_t color = Nan::To<uint32_t>(value).FromJust();

    handle.background_color(color);
}

}