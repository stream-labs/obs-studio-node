#pragma once

#include <Windows.h>
#include <string>
#include <thread>

class USBNotification {
public:
	enum WM_USB_MSG {
		WM_USB_CLOSE
	};

	enum USBDeviceInfo {
		VENDOR_ID,
		PRODUCT_ID,
		SERIAL_NUMBER
	};

	USBNotification();
	~USBNotification();

	void StartNotification();
	void StopNotification();

private:
	void CreateNotificationWindow();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static std::string WCharToString(wchar_t* text);
	static std::string GetDeviceInfo(const std::string& path, USBDeviceInfo type);

	HWND m_hWnd;
	std::string m_szClassName;
	std::thread m_windowThread;
};