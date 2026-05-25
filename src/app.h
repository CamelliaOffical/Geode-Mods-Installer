#pragma once

#include <string>

namespace geoode {

void printConfig();
void printHelp();
bool runSetup(bool makeDefault, std::wstring* error = nullptr);
bool autorunCommand(int argc, wchar_t** argv, std::wstring* error = nullptr);
bool setGeometryDashCommand(const std::wstring& rawPath,
                            std::wstring* error = nullptr);
bool openGeometryDashCommand(std::wstring* error = nullptr);
int interactiveSetup();
bool isFileInstallInvocation(int argc, wchar_t** argv);
void hideConsoleWindow();
void showInstallError(const std::wstring& message);

} // namespace geoode
