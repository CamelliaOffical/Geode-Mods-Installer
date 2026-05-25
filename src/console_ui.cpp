#include "console_ui.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

namespace geoode {
namespace {

constexpr const wchar_t* kAsciiLogo = LR"ASCII(         @%########%@         
      %##%@@@@@@@@@@%##%      
    ##%@@@@@@+..#@@@@@@%##    
  %#%@@@@@+. -@@@@@@@@@@@%#%  
 %#%@@@=  -@@@--@@@@@@%@@@@#% 
%#%@@* =@@%- .+=. :%@@+.#@@%#%
#*@##* *@-.+@@@@@@=.:@+.*##@*#
#*###+ +###%@@@@@@%.:#=.+###*#
**###+ +##***++++*#.:#=.+###**
#+###+ =+:.-++++++-.:+-.+###+*
*++++= -++*- .--. :*++-.=++++*
 *==+*=. .=++*--*++-  .=*+==* 
  +==+++*=. .=++-. .+*+++==+  
    =-=++++*+.  .+*++++=-=    
      +=-=++++**++++=-=+      
         #==------==*         )ASCII";

bool g_colorOutput = false;

void printGradient(const std::wstring& text, int startR, int startG, int startB,
                   int endR, int endG, int endB) {
  if (!g_colorOutput || text.empty()) {
    std::wcout << text;
    return;
  }

  const size_t last = text.size() > 1 ? text.size() - 1 : 1;
  for (size_t i = 0; i < text.size(); ++i) {
    const double t = static_cast<double>(i) / static_cast<double>(last);
    const int r = static_cast<int>(startR + (endR - startR) * t);
    const int g = static_cast<int>(startG + (endG - startG) * t);
    const int b = static_cast<int>(startB + (endB - startB) * t);
    std::wcout << L"\x1b[38;2;" << r << L";" << g << L";" << b << L"m"
               << text[i];
  }
  std::wcout << L"\x1b[0m";
}

void printVerticalGradientLines(const std::wstring& text, int startR,
                                int startG, int startB, int endR, int endG,
                                int endB) {
  std::vector<std::wstring> lines;
  size_t start = 0;
  while (start <= text.size()) {
    const size_t end = text.find(L'\n', start);
    lines.push_back(text.substr(
        start, end == std::wstring::npos ? std::wstring::npos : end - start));
    if (end == std::wstring::npos) {
      break;
    }
    start = end + 1;
  }

  const size_t last = lines.size() > 1 ? lines.size() - 1 : 1;
  for (size_t i = 0; i < lines.size(); ++i) {
    const double t = static_cast<double>(i) / static_cast<double>(last);
    const int r = static_cast<int>(startR + (endR - startR) * t);
    const int g = static_cast<int>(startG + (endG - startG) * t);
    const int b = static_cast<int>(startB + (endB - startB) * t);
    printGradient(lines[i], r, g, b, r, g, b);
    std::wcout << L"\n";
  }
}

} // namespace

bool enableVirtualTerminalOutput() {
  const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (output == INVALID_HANDLE_VALUE || output == nullptr) {
    return false;
  }

  DWORD mode = 0;
  if (!GetConsoleMode(output, &mode)) {
    return false;
  }

  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  return SetConsoleMode(output, mode) != 0;
}

void setColorOutput(bool enabled) {
  g_colorOutput = enabled;
}

void printBlueGradient(const std::wstring& text) {
  printGradient(text, 14, 43, 126, 0, 150, 255);
}

void printBlueGradientLine(const std::wstring& text) {
  printBlueGradient(text);
  std::wcout << L"\n";
}

void printPathGradient(const std::wstring& text) {
  printGradient(text, 14, 43, 126, 142, 45, 226);
}

void printAsciiLogo() {
  printVerticalGradientLines(kAsciiLogo, 0x0a, 0x00, 0x5f, 0x6d, 0x8b, 0xbc);
}

void printHelpCommand(const std::wstring& command,
                      const std::wstring& description) {
  constexpr size_t commandColumnWidth = 31;

  std::wcout << L"  ";
  printBlueGradient(command);
  if (command.size() < commandColumnWidth) {
    std::wcout << std::wstring(commandColumnWidth - command.size(), L' ');
  } else {
    std::wcout << L' ';
  }
  std::wcout << description << L"\n";
}

} // namespace geoode
