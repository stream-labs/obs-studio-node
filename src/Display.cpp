#include "Display.h"
#include "RenderPlugin.h"

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


Nan::Persistent<v8::FunctionTemplate> Display::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

NAN_MODULE_INIT(Display::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(create);
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Display"));
    Nan::SetMethod(locProto, "create", create);
    Nan::SetPrototypeMethod(locProto, "addDrawer", addDrawer);
    Nan::SetPrototypeMethod(locProto, "removeDrawer", removeDrawer);
    Nan::SetPrototypeMethod(locProto, "resize", resize);
    Nan::SetPrototypeMethod(locProto, "destroy", destroy);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("enabled"), enabled, enabled);
    Nan::Set(target, FIELD_NAME("Display"), locProto->GetFunction());
    prototype.Reset(locProto);
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

NAN_METHOD(Display::create)
{
    v8::Local<v8::Object> init_object;
 
    ASSERT_INFO_LENGTH(info, 1);
    ASSERT_GET_VALUE(info[0], init_object);

    gs_init_data init_data = {};

    DisplayWndClass();

    ASSERT_GET_OBJECT_FIELD(init_object, "width", init_data.cx);
    ASSERT_GET_OBJECT_FIELD(init_object, "height", init_data.cy);
    ASSERT_GET_OBJECT_FIELD(init_object, "format", init_data.format);
    ASSERT_GET_OBJECT_FIELD(init_object, "zsformat", init_data.zsformat);

    DisplayWndClass();

    init_data.window.hwnd = CreateWindowEx(
        0, //WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        TEXT("Win32DisplayClass"), TEXT("SlobsChildWindowPreview"),
        WS_VISIBLE | WS_POPUP,
        0, 0, init_data.cx, init_data.cy,
        NULL, NULL, NULL, NULL);

    Display *binding = new Display(init_data);
    auto object = Display::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_GETTER(Display::status)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_METHOD(Display::destroy)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    handle.destroy();
}

NAN_METHOD(Display::addDrawer)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);
    
    std::string path;

    ASSERT_GET_VALUE(info[0], path);

    void *module = osn_load_plugin(handle.dangerous(), path.c_str());

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
    obs::display &handle = Display::Object::GetHandle(info.Holder());

}

NAN_GETTER(Display::enabled)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.enabled());
}

NAN_SETTER(Display::enabled)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    bool is_enabled;

    ASSERT_GET_VALUE(value, is_enabled);

    handle.enabled(is_enabled);
}

NAN_SETTER(Display::backgroundColor)
{
    obs::display &handle = Display::Object::GetHandle(info.Holder());

    uint32_t color;

    ASSERT_GET_VALUE(value, color);

    handle.background_color(color);
}

}