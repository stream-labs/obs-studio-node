#include "usb-notification.h"
#include "obs.h"
#include <Dbt.h>
#include <initguid.h>
#include <usbiodef.h>
#include <iostream>
#include <regex>

USBNotification::USBNotification() :
	m_hWnd(0),
	m_szClassName("USBNotification"),
	m_windowThread()
{
}

USBNotification::~USBNotification()
{
	if (m_hWnd) {
		DestroyWindow(m_hWnd);
	}
}

void USBNotification::StartNotification() {
	m_windowThread = std::thread(&USBNotification::CreateNotificationWindow, this);
}

void USBNotification::StopNotification() {
	PostThreadMessage(GetThreadId(m_windowThread.native_handle()), WM_USB_CLOSE, 0, 0);
	m_windowThread.join();
}

void USBNotification::CreateNotificationWindow() {
	HWND       hWnd = nullptr;
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc   = WindowProc;
	wcex.hInstance     = GetModuleHandle(0);
	wcex.lpszClassName = (LPCWSTR)m_szClassName.c_str();

	if (RegisterClassEx(&wcex)) {
		m_hWnd = CreateWindow(
		    (LPCWSTR)m_szClassName.c_str(),
		    nullptr,
		    0,
		    CW_USEDEFAULT,
		    CW_USEDEFAULT,
		    CW_USEDEFAULT,
		    CW_USEDEFAULT,
		    HWND_MESSAGE,
		    nullptr,
		    nullptr,
		    nullptr);
	}

	BOOL bRet;
	MSG  msg;
	bool closeNotification = false;

	while (!closeNotification && (bRet = GetMessage(&msg, 0, 0, 0) != 0)) {
		if (bRet == -1) {
			break;
		} else {
			if (msg.message == WM_USB_CLOSE) {
				closeNotification = true;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CALLBACK USBNotification::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_NCCREATE: {
		return TRUE;
	}

	case WM_CREATE:
	{
		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

		ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));

		NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

		HDEVNOTIFY deviceNotification = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if (deviceNotification == NULL) {
			blog(LOG_DEBUG, "Could not register for device notifications!");
		}

		break;
	}

	case WM_DEVICECHANGE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		PDEV_BROADCAST_DEVICEINTERFACE lpdbv = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
		std::string path;
		if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			path = WCharToString(lpdbv->dbcc_name);

			if (wParam == DBT_DEVICEARRIVAL) {
				blog(
				    LOG_DEBUG,
				    "USB Device connected | Vendor ID: %s, Product ID: %s, Serial Number: %s",
				    GetDeviceInfo(path, VENDOR_ID).c_str(),
				    GetDeviceInfo(path, PRODUCT_ID).c_str(),
				    GetDeviceInfo(path, SERIAL_NUMBER).c_str());
			} else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
				blog(
				    LOG_DEBUG,
				    "USB Device disconnected | Vendor ID: %s, Product ID: %s, Serial Number: %s",
				    GetDeviceInfo(path, VENDOR_ID).c_str(),
				    GetDeviceInfo(path, PRODUCT_ID).c_str(),
				    GetDeviceInfo(path, SERIAL_NUMBER).c_str());
			}
		}

		break;
	}

	case WM_DESTROY: {
		USBNotification* pNotification = (USBNotification*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (pNotification) {
			pNotification->m_hWnd = 0;
		}

		break;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

std::string USBNotification::WCharToString(wchar_t* text) {
	std::string convertedText;

	while (*text) {
		convertedText += (char)*text++;
	}

	return convertedText;
}

std::string USBNotification::GetDeviceInfo(const std::string& path, USBDeviceInfo type) {
	std::regex  vidRegex(".*VID_(\\w+)&.*");
	std::regex  pidRegex(".*PID_(\\w+)#.*");
	std::regex  snRegex(".*#(\\w+)#.*");
	std::smatch match;
	std::string value;

	if (type == VENDOR_ID) {
		if (std::regex_search(path.begin(), path.end(), match, vidRegex))
			value = match[1];
	} else if (type == PRODUCT_ID) {
		if (std::regex_search(path.begin(), path.end(), match, pidRegex))
			value = match[1];
	} else if (type == SERIAL_NUMBER) {
		if (std::regex_search(path.begin(), path.end(), match, snRegex))
			value = match[1];
	}

	return value;
}
