#pragma once

#include <optional>
#include <string>

namespace geoode {

bool saveGeometryDashPath(const std::wstring& path, std::wstring* error = nullptr);
std::optional<std::wstring> loadGeometryDashPath();
bool unsetGeometryDashPath();
bool saveAutorunEnabled(bool enabled, std::wstring* error = nullptr);
bool isAutorunEnabled();
bool addSelfDirectoryToUserPath(bool* changed, std::wstring* error = nullptr);
bool registerOpenWith(bool makeDefault, std::wstring* error = nullptr);
bool unregisterOpenWith(std::wstring* error = nullptr);

} // namespace geoode
