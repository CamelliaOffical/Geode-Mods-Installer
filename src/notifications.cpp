#include "notifications.h"

#include <windows.h>
#include <shellapi.h>

#include <cwchar>
#include <string>

namespace geoode {
namespace {

template <size_t Size>
void copyNotifyText(wchar_t (&target)[Size], const std::wstring& value) {
  wcsncpy_s(target, Size, value.c_str(), _TRUNCATE);
}

HWND createNotificationWindow(bool* ownsWindow) {
  if (ownsWindow) {
    *ownsWindow = false;
  }

  const HWND consoleWindow = GetConsoleWindow();
  if (consoleWindow) {
    return consoleWindow;
  }

  const HINSTANCE instance = GetModuleHandleW(nullptr);
  constexpr const wchar_t* kWindowClass = L"GeoodeNotificationWindow";

  WNDCLASSW windowClass = {};
  windowClass.lpfnWndProc = DefWindowProcW;
  windowClass.hInstance = instance;
  windowClass.lpszClassName = kWindowClass;
  RegisterClassW(&windowClass);

  const HWND window =
      CreateWindowExW(0, kWindowClass, L"Geoode Installer", 0, 0, 0, 0, 0,
                      nullptr, nullptr, instance, nullptr);

  if (window && ownsWindow) {
    *ownsWindow = true;
  }
  return window;
}

} // namespace

void showGeodeInstalledNotification(const std::filesystem::path& source) {
  bool ownsWindow = false;
  const HWND window = createNotificationWindow(&ownsWindow);
  if (!window) {
    MessageBeep(MB_ICONINFORMATION);
    return;
  }

  const HINSTANCE instance = GetModuleHandleW(nullptr);
  const HICON icon = LoadIconW(instance, MAKEINTRESOURCEW(1));

  NOTIFYICONDATAW data = {};
  data.cbSize = sizeof(data);
  data.hWnd = window;
  data.uID = 1;
  data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  data.uCallbackMessage = WM_APP + 1;
  data.hIcon = icon ? icon : LoadIconW(nullptr, IDI_INFORMATION);
  copyNotifyText(data.szTip, L"Geoode Installer");

  Shell_NotifyIconW(NIM_DELETE, &data);
  if (!Shell_NotifyIconW(NIM_ADD, &data)) {
    MessageBeep(MB_ICONINFORMATION);
    if (ownsWindow) {
      DestroyWindow(window);
    }
    return;
  }

  data.uVersion = NOTIFYICON_VERSION_4;
  Shell_NotifyIconW(NIM_SETVERSION, &data);

  data.uFlags = NIF_INFO;
  copyNotifyText(data.szInfoTitle, L"Geode Mod Installed");
  (void)source;
  copyNotifyText(data.szInfo, L"Installed into geode mods folder");
  data.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
  data.hBalloonIcon = data.hIcon;

  if (!Shell_NotifyIconW(NIM_MODIFY, &data)) {
    MessageBeep(MB_ICONINFORMATION);
  }

  Sleep(2500);
  Shell_NotifyIconW(NIM_DELETE, &data);

  if (ownsWindow) {
    DestroyWindow(window);
  }
}

} // namespace geoode
