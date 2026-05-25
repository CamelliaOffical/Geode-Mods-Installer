#pragma once

#include <filesystem>
#include <string>

namespace geoode {

bool validateGeometryDashPath(const std::wstring& rawPath,
                              std::filesystem::path* normalizedPath,
                              std::wstring* error);
bool launchGeometryDash(const std::filesystem::path& geometryDashExe,
                        std::wstring* error = nullptr);
bool installGeodeFile(const std::wstring& rawFile, std::wstring* error = nullptr);

} // namespace geoode
