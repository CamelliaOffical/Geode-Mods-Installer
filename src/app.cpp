#include "app.h"

#include "common.h"
#include "console_ui.h"
#include "installer.h"
#include "registry.h"

#include <windows.h>

#include <filesystem>
#include <iostream>

namespace geoode {
namespace fs = std::filesystem;

void printConfig() {
  const auto gd = loadGeometryDashPath();
  if (!gd) {
    printBlueGradient(L"Geometry Dash: ");
    printPathGradient(L"not set");
    std::wcout << L"\n";
    return;
  }

  printBlueGradient(L"Geometry Dash: ");
  printPathGradient(*gd);
  std::wcout << L"\n";
  printBlueGradient(L"Mods folder:   ");
  printPathGradient(modsDirectoryForGame(fs::path(*gd)).wstring());
  std::wcout << L"\n";
}

void printHelp() {
  printBlueGradientLine(L"Geoode Installer");
  std::wcout << L"\n";
  printBlueGradientLine(L"Usage:");
  printHelpCommand(L"geoode", L"Show this help and current config");
  printHelpCommand(L"geoode set <GeometryDash.exe>",
                   L"Save Geometry Dash path");
  printHelpCommand(L"geoode register [default]",
                   L"Register \"Install Into Geode\"");
  printHelpCommand(L"geoode unset", L"Forget saved Geometry Dash path");
  printHelpCommand(L"geoode unregister", L"Remove .geode registration");
  printHelpCommand(L"geoode autorun on/off",
                   L"Launch GD after installing a .geode");
  std::wcout << L"\n";
  printConfig();
  std::wcout << L"\n";
  printBlueGradient(L"Created by ");
  printPathGradient(L"t.me/SHARKofficala");
  std::wcout << L"\n";
}

bool runSetup(bool makeDefault, std::wstring* error) {
  bool pathChanged = false;
  if (!addSelfDirectoryToUserPath(&pathChanged, error)) {
    return false;
  }

  if (!registerOpenWith(makeDefault, error)) {
    return false;
  }

  std::wcout << L"PATH: "
             << (pathChanged ? L"updated" : L"already contains geoode")
             << L"\n";
  std::wcout << L".geode: registered as \"" << kMenuName << L"\"";
  if (makeDefault) {
    std::wcout << L" and set as default";
  }
  std::wcout << L"\n";
  if (pathChanged) {
    std::wcout << L"Open a new terminal before using geoode by name.\n";
  }
  return true;
}

bool autorunCommand(int argc, wchar_t** argv, std::wstring* error) {
  if (argc < 3) {
    printBlueGradient(L"Autorun: ");
    printPathGradient(isAutorunEnabled() ? L"on" : L"off");
    std::wcout << L"\n";
    return true;
  }

  const std::wstring value = toLower(stripOuterQuotes(argv[2]));
  if (value == L"on") {
    if (!saveAutorunEnabled(true, error)) {
      return false;
    }
    std::wcout << L"Autorun enabled.\n";
    return true;
  }

  if (value == L"off") {
    if (!saveAutorunEnabled(false, error)) {
      return false;
    }
    std::wcout << L"Autorun disabled.\n";
    return true;
  }

  if (error) {
    *error = L"Expected: geoode autorun on or geoode autorun off";
  }
  return false;
}

bool setGeometryDashCommand(const std::wstring& rawPath, std::wstring* error) {
  fs::path normalized;
  if (!validateGeometryDashPath(rawPath, &normalized, error)) {
    return false;
  }

  if (!equalsIgnoreCase(normalized.filename().wstring(), L"GeometryDash.exe")) {
    std::wcout
        << L"Warning: file name is not GeometryDash.exe, saving it anyway.\n";
  }

  if (!saveGeometryDashPath(normalized.wstring(), error)) {
    return false;
  }

  std::wcout << L"Saved Geometry Dash path:\n" << normalized.wstring() << L"\n";
  std::wcout << L"Mods folder:\n"
             << modsDirectoryForGame(normalized).wstring() << L"\n";
  return true;
}

bool openGeometryDashCommand(std::wstring* error) {
  const auto gd = loadGeometryDashPath();
  if (!gd) {
    if (error) {
      *error = L"Geometry Dash path is not set. Run: geoode set "
               L"<path-to-GeometryDash.exe>";
    }
    return false;
  }

  fs::path normalized;
  if (!validateGeometryDashPath(*gd, &normalized, error)) {
    return false;
  }

  return launchGeometryDash(normalized, error);
}

int interactiveSetup() {
  printAsciiLogo();
  std::wcout << L"\n";

  printBlueGradientLine(L"Geoode Installer setup");
  std::wcout << L"\n";

  const auto gd = loadGeometryDashPath();
  printBlueGradient(L"Geometry Dash: ");
  printPathGradient(gd ? *gd : std::wstring(L"not set"));
  std::wcout << L"\n\n";

  printBlueGradient(L"Path: ");

  std::wstring input;
  std::getline(std::wcin, input);
  input = stripOuterQuotes(std::move(input));
  if (input.empty()) {
    std::wcout << L"No path entered.\n";
    return 1;
  }

  std::wstring error;
  if (!setGeometryDashCommand(input, &error)) {
    std::wcerr << L"Error: " << error << L"\n";
    return 1;
  }

  if (!runSetup(true, &error)) {
    std::wcerr << L"Setup warning: " << error << L"\n";
    return 1;
  }

  return 0;
}

bool isFileInstallInvocation(int argc, wchar_t** argv) {
  if (argc <= 1) {
    return false;
  }

  const std::wstring command = toLower(argv[1]);
  if (command == L"install" && argc >= 3) {
    return true;
  }

  if (command == L"set" || command == L"open" || command == L"setup" ||
      command == L"register" || command == L"path" || command == L"where" ||
      command == L"unset" || command == L"unregister" ||
      command == L"autorun" || command == L"help" || command == L"--help" ||
      command == L"-h" || command == L"/?") {
    return false;
  }

  return isGeodeFile(fs::path(joinArgs(argc, argv, 1)));
}

void hideConsoleWindow() {
  const HWND console = GetConsoleWindow();
  if (console) {
    ShowWindow(console, SW_HIDE);
  }
}

void showInstallError(const std::wstring& message) {
  MessageBoxW(nullptr, message.c_str(), L"Geoode Installer",
              MB_OK | MB_ICONERROR);
}

} // namespace geoode
