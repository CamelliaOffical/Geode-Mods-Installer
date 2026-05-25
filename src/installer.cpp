#include "installer.h"

#include "common.h"
#include "notifications.h"
#include "registry.h"

#include <windows.h>
#include <shellapi.h>

#include <iostream>
#include <system_error>

namespace geoode {
namespace fs = std::filesystem;

bool validateGeometryDashPath(const std::wstring& rawPath,
                              fs::path* normalizedPath,
                              std::wstring* error) {
  const fs::path path = stripOuterQuotes(rawPath);
  std::error_code ec;
  const fs::path absolutePath = fs::absolute(path, ec);
  const fs::path candidate = ec ? path : absolutePath;

  if (!fs::exists(candidate, ec) || ec) {
    if (error) {
      *error = L"File does not exist: " + candidate.wstring();
    }
    return false;
  }

  if (!fs::is_regular_file(candidate, ec) || ec) {
    if (error) {
      *error = L"Path is not a file: " + candidate.wstring();
    }
    return false;
  }

  if (!equalsIgnoreCase(candidate.extension().wstring(), L".exe")) {
    if (error) {
      *error = L"Expected an .exe file: " + candidate.wstring();
    }
    return false;
  }

  if (normalizedPath) {
    *normalizedPath = candidate;
  }
  return true;
}

bool launchGeometryDash(const fs::path& geometryDashExe, std::wstring* error) {
  const fs::path workingDir = geometryDashExe.parent_path();
  const HINSTANCE result =
      ShellExecuteW(nullptr, L"open", geometryDashExe.c_str(), nullptr,
                    workingDir.c_str(), SW_SHOWNORMAL);

  if (reinterpret_cast<intptr_t>(result) <= 32) {
    if (error) {
      *error = L"ShellExecute failed with code " +
               std::to_wstring(reinterpret_cast<intptr_t>(result));
    }
    return false;
  }
  return true;
}

bool installGeodeFile(const std::wstring& rawFile, std::wstring* error) {
  const auto gamePathValue = loadGeometryDashPath();
  if (!gamePathValue) {
    if (error) {
      *error = L"Geometry Dash path is not set. Run: geoode set "
               L"<path-to-GeometryDash.exe>";
    }
    return false;
  }

  fs::path geometryDashExe;
  if (!validateGeometryDashPath(*gamePathValue, &geometryDashExe, error)) {
    return false;
  }

  const fs::path source = stripOuterQuotes(rawFile);
  std::error_code ec;
  if (!fs::exists(source, ec) || ec) {
    if (error) {
      *error = L"File does not exist: " + source.wstring();
    }
    return false;
  }

  if (!fs::is_regular_file(source, ec) || ec) {
    if (error) {
      *error = L"Path is not a file: " + source.wstring();
    }
    return false;
  }

  if (!isGeodeFile(source)) {
    if (error) {
      *error = L"Expected a .geode file: " + source.wstring();
    }
    return false;
  }

  const fs::path modsDir = modsDirectoryForGame(geometryDashExe);
  fs::create_directories(modsDir, ec);
  if (ec) {
    if (error) {
      *error = L"Could not create mods folder: " + modsDir.wstring() + L" (" +
               widenAscii(ec.message()) + L")";
    }
    return false;
  }

  const fs::path destination = modsDir / source.filename();
  if (!fs::equivalent(source, destination, ec)) {
    ec.clear();
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
    if (ec) {
      if (error) {
        *error = L"Could not copy mod: " + widenAscii(ec.message());
      }
      return false;
    }
  }

  std::wcout << L"Installed: " << destination.wstring() << L"\n";

  if (!isAutorunEnabled()) {
    showGeodeInstalledNotification(source);
    std::wcout << L"Autorun is off. Geometry Dash was not launched.\n";
    return true;
  }

  if (!launchGeometryDash(geometryDashExe, error)) {
    return false;
  }

  std::wcout << L"Geometry Dash launched.\n";
  return true;
}

} // namespace geoode
