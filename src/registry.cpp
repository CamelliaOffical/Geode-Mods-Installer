#include "registry.h"

#include "common.h"

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <filesystem>

namespace geoode {
namespace fs = std::filesystem;
namespace {

bool writeRegString(HKEY root, const std::wstring& subkey,
                    const std::wstring& valueName, const std::wstring& value,
                    std::wstring* error = nullptr) {
  HKEY key = nullptr;
  const LONG createStatus =
      RegCreateKeyExW(root, subkey.c_str(), 0, nullptr, 0, KEY_SET_VALUE,
                      nullptr, &key, nullptr);
  if (createStatus != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(createStatus));
    }
    return false;
  }

  const auto* name = valueName.empty() ? nullptr : valueName.c_str();
  const auto byteCount =
      static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
  const LONG setStatus =
      RegSetValueExW(key, name, 0, REG_SZ,
                     reinterpret_cast<const BYTE*>(value.c_str()), byteCount);
  RegCloseKey(key);

  if (setStatus != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(setStatus));
    }
    return false;
  }

  return true;
}

bool writeRegNone(HKEY root, const std::wstring& subkey,
                  const std::wstring& valueName, std::wstring* error = nullptr) {
  HKEY key = nullptr;
  const LONG createStatus =
      RegCreateKeyExW(root, subkey.c_str(), 0, nullptr, 0, KEY_SET_VALUE,
                      nullptr, &key, nullptr);
  if (createStatus != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(createStatus));
    }
    return false;
  }

  const LONG setStatus =
      RegSetValueExW(key, valueName.c_str(), 0, REG_NONE, nullptr, 0);
  RegCloseKey(key);

  if (setStatus != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(setStatus));
    }
    return false;
  }

  return true;
}

std::optional<std::wstring> readRegString(HKEY root, const std::wstring& subkey,
                                          const std::wstring& valueName) {
  DWORD type = 0;
  DWORD bytes = 0;
  const auto* name = valueName.empty() ? nullptr : valueName.c_str();
  LONG status = RegGetValueW(root, subkey.c_str(), name,
                             RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, &type,
                             nullptr, &bytes);

  if (status != ERROR_SUCCESS || bytes == 0) {
    return std::nullopt;
  }

  std::wstring value(bytes / sizeof(wchar_t), L'\0');
  status = RegGetValueW(root, subkey.c_str(), name,
                        RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, &type,
                        value.data(), &bytes);

  if (status != ERROR_SUCCESS) {
    return std::nullopt;
  }

  value.resize(bytes / sizeof(wchar_t));
  while (!value.empty() && value.back() == L'\0') {
    value.pop_back();
  }
  return value;
}

bool deleteRegValue(HKEY root, const std::wstring& subkey,
                    const std::wstring& valueName) {
  HKEY key = nullptr;
  const LONG openStatus =
      RegOpenKeyExW(root, subkey.c_str(), 0, KEY_SET_VALUE, &key);
  if (openStatus != ERROR_SUCCESS) {
    return false;
  }
  const LONG deleteStatus = RegDeleteValueW(key, valueName.c_str());
  RegCloseKey(key);
  return deleteStatus == ERROR_SUCCESS;
}

} // namespace

bool saveGeometryDashPath(const std::wstring& path, std::wstring* error) {
  return writeRegString(HKEY_CURRENT_USER, kConfigKey, kConfigValue, path, error);
}

std::optional<std::wstring> loadGeometryDashPath() {
  return readRegString(HKEY_CURRENT_USER, kConfigKey, kConfigValue);
}

bool unsetGeometryDashPath() {
  return deleteRegValue(HKEY_CURRENT_USER, kConfigKey, kConfigValue);
}

bool saveAutorunEnabled(bool enabled, std::wstring* error) {
  return writeRegString(HKEY_CURRENT_USER, kConfigKey, kAutorunValue,
                        enabled ? L"on" : L"off", error);
}

bool isAutorunEnabled() {
  const auto value =
      readRegString(HKEY_CURRENT_USER, kConfigKey, kAutorunValue);
  if (!value) {
    return true;
  }

  const std::wstring normalized = toLower(stripOuterQuotes(*value));
  return normalized != L"off" && normalized != L"false" && normalized != L"0";
}

bool addSelfDirectoryToUserPath(bool* changed, std::wstring* error) {
  if (changed) {
    *changed = false;
  }

  const fs::path self = modulePath();
  const std::wstring selfDir = self.parent_path().wstring();
  if (selfDir.empty()) {
    if (error) {
      *error = L"Could not resolve geoode.exe location.";
    }
    return false;
  }

  HKEY key = nullptr;
  LONG status =
      RegCreateKeyExW(HKEY_CURRENT_USER, L"Environment", 0, nullptr, 0,
                      KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &key, nullptr);
  if (status != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(status));
    }
    return false;
  }

  DWORD type = REG_EXPAND_SZ;
  DWORD bytes = 0;
  std::wstring current;
  status = RegQueryValueExW(key, L"Path", nullptr, &type, nullptr, &bytes);
  if (status == ERROR_SUCCESS && bytes > 0 &&
      (type == REG_SZ || type == REG_EXPAND_SZ)) {
    current.assign(bytes / sizeof(wchar_t), L'\0');
    status = RegQueryValueExW(key, L"Path", nullptr, &type,
                              reinterpret_cast<BYTE*>(current.data()), &bytes);
    if (status == ERROR_SUCCESS) {
      current.resize(bytes / sizeof(wchar_t));
      while (!current.empty() && current.back() == L'\0') {
        current.pop_back();
      }
    }
  } else if (status == ERROR_FILE_NOT_FOUND) {
    type = REG_EXPAND_SZ;
    status = ERROR_SUCCESS;
  }

  if (status != ERROR_SUCCESS) {
    RegCloseKey(key);
    if (error) {
      *error = winError(static_cast<DWORD>(status));
    }
    return false;
  }

  const std::wstring needle = normalizePathForCompare(selfDir);
  bool alreadyPresent = false;
  size_t start = 0;
  while (start <= current.size()) {
    const size_t end = current.find(L';', start);
    std::wstring part = current.substr(
        start, end == std::wstring::npos ? std::wstring::npos : end - start);
    if (normalizePathForCompare(std::move(part)) == needle) {
      alreadyPresent = true;
      break;
    }
    if (end == std::wstring::npos) {
      break;
    }
    start = end + 1;
  }

  if (alreadyPresent) {
    RegCloseKey(key);
    return true;
  }

  std::wstring next = current;
  if (!next.empty() && next.back() != L';') {
    next += L';';
  }
  next += selfDir;

  const DWORD byteCount =
      static_cast<DWORD>((next.size() + 1) * sizeof(wchar_t));
  status =
      RegSetValueExW(key, L"Path", 0, type,
                     reinterpret_cast<const BYTE*>(next.c_str()), byteCount);
  RegCloseKey(key);

  if (status != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(status));
    }
    return false;
  }

  SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                      reinterpret_cast<LPARAM>(L"Environment"),
                      SMTO_ABORTIFHUNG, 5000, nullptr);
  if (changed) {
    *changed = true;
  }
  return true;
}

bool registerOpenWith(bool makeDefault, std::wstring* error) {
  const fs::path selfPath = modulePath();
  const std::wstring handler = selfPath.wstring();
  const std::wstring exeName = selfPath.filename().wstring();
  const std::wstring command = quoteForCommand(handler) + L" \"%1\"";
  const std::wstring appKey = L"Software\\Classes\\Applications\\" + exeName;

  const struct StringWrite {
    std::wstring subkey;
    std::wstring valueName;
    std::wstring value;
  } writes[] = {
      {appKey, L"FriendlyAppName", kMenuName},
      {appKey + L"\\DefaultIcon", L"", handler + L",0"},
      {appKey + L"\\SupportedTypes", L".geode", L""},
      {appKey + L"\\shell\\open", L"", kMenuName},
      {appKey + L"\\shell\\open\\command", L"", command},
      {L"Software\\Classes\\" + std::wstring(kProgId), L"", L"Geode Mod"},
      {L"Software\\Classes\\" + std::wstring(kProgId) + L"\\DefaultIcon", L"",
       handler + L",0"},
      {L"Software\\Classes\\" + std::wstring(kProgId) + L"\\shell\\open", L"",
       kMenuName},
      {L"Software\\Classes\\" + std::wstring(kProgId) +
           L"\\shell\\open\\command",
       L"", command},
  };

  for (const auto& write : writes) {
    if (!writeRegString(HKEY_CURRENT_USER, write.subkey, write.valueName,
                        write.value, error)) {
      return false;
    }
  }

  if (!writeRegNone(HKEY_CURRENT_USER,
                    L"Software\\Classes\\.geode\\OpenWithProgids", kProgId,
                    error)) {
    return false;
  }

  HKEY openWithListKey = nullptr;
  const std::wstring openWithList =
      L"Software\\Classes\\.geode\\OpenWithList\\" + exeName;
  const LONG listStatus =
      RegCreateKeyExW(HKEY_CURRENT_USER, openWithList.c_str(), 0, nullptr, 0,
                      KEY_SET_VALUE, nullptr, &openWithListKey, nullptr);
  if (listStatus != ERROR_SUCCESS) {
    if (error) {
      *error = winError(static_cast<DWORD>(listStatus));
    }
    return false;
  }
  RegCloseKey(openWithListKey);

  if (makeDefault &&
      !writeRegString(HKEY_CURRENT_USER, L"Software\\Classes\\.geode", L"",
                      kProgId, error)) {
    return false;
  }

  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
  return true;
}

bool unregisterOpenWith(std::wstring* error) {
  const fs::path selfPath = modulePath();
  const std::wstring exeNames[] = {
      selfPath.filename().wstring(),
      L"geoode.exe",
  };

  for (const auto& exeName : exeNames) {
    RegDeleteTreeW(HKEY_CURRENT_USER,
                   (L"Software\\Classes\\Applications\\" + exeName).c_str());
    RegDeleteTreeW(
        HKEY_CURRENT_USER,
        (L"Software\\Classes\\.geode\\OpenWithList\\" + exeName).c_str());
  }
  RegDeleteTreeW(HKEY_CURRENT_USER,
                 (L"Software\\Classes\\" + std::wstring(kProgId)).c_str());
  deleteRegValue(HKEY_CURRENT_USER,
                 L"Software\\Classes\\.geode\\OpenWithProgids", kProgId);

  const auto extDefault =
      readRegString(HKEY_CURRENT_USER, L"Software\\Classes\\.geode", L"");
  if (extDefault && *extDefault == kProgId) {
    HKEY key = nullptr;
    const LONG openStatus =
        RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\.geode", 0,
                      KEY_SET_VALUE, &key);
    if (openStatus == ERROR_SUCCESS) {
      RegDeleteValueW(key, nullptr);
      RegCloseKey(key);
    } else if (error) {
      *error = winError(static_cast<DWORD>(openStatus));
      return false;
    }
  }

  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
  return true;
}

} // namespace geoode
