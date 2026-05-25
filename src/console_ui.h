#pragma once

#include <string>

namespace geoode {

bool enableVirtualTerminalOutput();
void setColorOutput(bool enabled);
void printBlueGradient(const std::wstring& text);
void printBlueGradientLine(const std::wstring& text);
void printPathGradient(const std::wstring& text);
void printAsciiLogo();
void printHelpCommand(const std::wstring& command, const std::wstring& description);

} // namespace geoode
