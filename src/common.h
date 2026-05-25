#pragma once

#include <windows.h>

#include <filesystem>
#include <optional>
#include <string>

namespace geoode {

inline constexpr const wchar_t* kConfigKey = L"Software\\GeoodeInstaller";
inline constexpr const wchar_t* kConfigValue = L"GeometryDashExe";
inline constexpr const wchar_t* kAutorunValue = L"Autorun";
inline constexpr const wchar_t* kProgId = L"GeoodeInstaller.geode";
inline constexpr const wchar_t* kMenuName = L"Install Into Geode";

std::wstring winError(DWORD code);
std::wstring trim(std::wstring value);
std::wstring stripOuterQuotes(std::wstring value);
std::wstring toLower(std::wstring value);
std::wstring widenAscii(const std::string& value);
bool equalsIgnoreCase(const std::wstring& left, const std::wstring& right);
std::wstring joinArgs(int argc, wchar_t** argv, int start);
std::wstring quoteForCommand(const std::wstring& value);
std::wstring modulePath();
std::wstring normalizePathForCompare(std::wstring value);
bool isOwnConsole();
void pauseIfNeeded(bool shouldPause);
std::filesystem::path modsDirectoryForGame(const std::filesystem::path& geometryDashExe);
bool isGeodeFile(const std::filesystem::path& path);

} // namespace geoode
