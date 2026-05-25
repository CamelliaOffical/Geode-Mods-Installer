#include "app.h"
#include "common.h"
#include "console_ui.h"
#include "installer.h"
#include "registry.h"

#include <windows.h>

#include <filesystem>
#include <iostream>
#include <string>

int wmain(int argc, wchar_t** argv) {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  geoode::setColorOutput(geoode::enableVirtualTerminalOutput());

  const bool launchedInOwnConsole = geoode::isOwnConsole();
  const bool fileInstallInvocation = geoode::isFileInstallInvocation(argc, argv);
  if (launchedInOwnConsole && fileInstallInvocation) {
    geoode::hideConsoleWindow();
  }
  const bool pauseOnExit = launchedInOwnConsole && !fileInstallInvocation;

  if (argc <= 1) {
    if (launchedInOwnConsole) {
      const int result = geoode::interactiveSetup();
      geoode::pauseIfNeeded(true);
      return result;
    }

    geoode::printHelp();
    return 0;
  }

  const std::wstring command = geoode::toLower(argv[1]);
  std::wstring error;
  bool ok = false;

  if (command == L"set") {
    if (argc < 3) {
      std::wcerr << L"Error: missing path to GeometryDash.exe\n";
      geoode::pauseIfNeeded(pauseOnExit);
      return 1;
    }
    ok = geoode::setGeometryDashCommand(geoode::joinArgs(argc, argv, 2), &error);
  } else if (command == L"install") {
    if (argc < 3) {
      std::wcerr << L"Error: missing .geode file\n";
      geoode::pauseIfNeeded(pauseOnExit);
      return 1;
    }
    ok = geoode::installGeodeFile(geoode::joinArgs(argc, argv, 2), &error);
  } else if (command == L"open") {
    ok = geoode::openGeometryDashCommand(&error);
    if (ok) {
      std::wcout << L"Geometry Dash launched.\n";
    }
  } else if (command == L"setup") {
    ok = geoode::runSetup(true, &error);
  } else if (command == L"register") {
    const bool makeDefault = argc >= 3 && geoode::toLower(argv[2]) == L"default";
    ok = geoode::registerOpenWith(makeDefault, &error);
    if (ok) {
      std::wcout << L".geode registered as \"" << geoode::kMenuName << L"\"";
      if (makeDefault) {
        std::wcout << L" and set as default";
      }
      std::wcout << L".\n";
    }
  } else if (command == L"path") {
    bool changed = false;
    ok = geoode::addSelfDirectoryToUserPath(&changed, &error);
    if (ok) {
      std::wcout << L"PATH: "
                 << (changed ? L"updated. Open a new terminal."
                             : L"already contains geoode.")
                 << L"\n";
    }
  } else if (command == L"where") {
    geoode::printConfig();
    return 0;
  } else if (command == L"unset") {
    ok = geoode::unsetGeometryDashPath();
    if (ok) {
      std::wcout << L"Geometry Dash path removed.\n";
    } else {
      std::wcout << L"Geometry Dash path was not set.\n";
      ok = true;
    }
  } else if (command == L"unregister") {
    ok = geoode::unregisterOpenWith(&error);
    if (ok) {
      std::wcout << L".geode registration removed.\n";
    }
  } else if (command == L"autorun") {
    ok = geoode::autorunCommand(argc, argv, &error);
  } else if (command == L"help" || command == L"--help" || command == L"-h" ||
             command == L"/?") {
    geoode::printHelp();
    return 0;
  } else if (geoode::isGeodeFile(
                 std::filesystem::path(geoode::joinArgs(argc, argv, 1)))) {
    ok = geoode::installGeodeFile(geoode::joinArgs(argc, argv, 1), &error);
  } else {
    std::wcerr << L"Error: unknown command or not a .geode file.\n\n";
    geoode::printHelp();
    geoode::pauseIfNeeded(pauseOnExit);
    return 1;
  }

  if (!ok) {
    if (launchedInOwnConsole && fileInstallInvocation) {
      geoode::showInstallError(L"Error: " + error);
    } else {
      std::wcerr << L"Error: " << error << L"\n";
      geoode::pauseIfNeeded(pauseOnExit);
    }
    return 1;
  }

  return 0;
}
