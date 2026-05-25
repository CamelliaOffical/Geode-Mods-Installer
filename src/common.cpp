#include "common.h"

#include <algorithm>
#include <cwctype>
#include <iostream>

namespace geoode {

std::wstring winError(DWORD code) {
  wchar_t* buffer = nullptr;
  const DWORD size = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

  std::wstring message =
      size && buffer ? std::wstring(buffer, size) : L"Unknown Windows error";
  if (buffer) {
    LocalFree(buffer);
  }

  while (!message.empty() &&
         (message.back() == L'\r' || message.back() == L'\n' ||
          std::iswspace(message.back()))) {
    message.pop_back();
  }

  return message;
}

std::wstring trim(std::wstring value) {
  const auto begin =
      std::find_if_not(value.begin(), value.end(),
                       [](wchar_t ch) { return std::iswspace(ch) != 0; });
  const auto end =
      std::find_if_not(value.rbegin(), value.rend(), [](wchar_t ch) {
        return std::iswspace(ch) != 0;
      }).base();

  if (begin >= end) {
    return {};
  }

  return std::wstring(begin, end);
}

std::wstring stripOuterQuotes(std::wstring value) {
  value = trim(std::move(value));
  if (value.size() >= 2) {
    const wchar_t first = value.front();
    const wchar_t last = value.back();
    if ((first == L'"' && last == L'"') || (first == L'\'' && last == L'\'')) {
      value = value.substr(1, value.size() - 2);
    }
  }
  return trim(std::move(value));
}

std::wstring toLower(std::wstring value) {
  std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towlower(ch));
  });
  return value;
}

std::wstring widenAscii(const std::string& value) {
  return std::wstring(value.begin(), value.end());
}

bool equalsIgnoreCase(const std::wstring& left, const std::wstring& right) {
  return toLower(left) == toLower(right);
}

std::wstring joinArgs(int argc, wchar_t** argv, int start) {
  std::wstring result;
  for (int i = start; i < argc; ++i) {
    if (!result.empty()) {
      result += L' ';
    }
    result += argv[i];
  }
  return stripOuterQuotes(std::move(result));
}

std::wstring quoteForCommand(const std::wstring& value) {
  return L"\"" + value + L"\"";
}

std::wstring modulePath() {
  std::wstring buffer(32768, L'\0');
  const DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                          static_cast<DWORD>(buffer.size()));
  if (length == 0 || length >= buffer.size()) {
    return {};
  }
  buffer.resize(length);
  return buffer;
}

std::wstring normalizePathForCompare(std::wstring value) {
  value = stripOuterQuotes(std::move(value));
  while (!value.empty() && (value.back() == L'\\' || value.back() == L'/')) {
    value.pop_back();
  }
  std::replace(value.begin(), value.end(), L'/', L'\\');
  return toLower(std::move(value));
}

bool isOwnConsole() {
  DWORD processIds[2] = {};
  const DWORD count = GetConsoleProcessList(processIds, 2);
  return count == 1;
}

void pauseIfNeeded(bool shouldPause) {
  if (!shouldPause) {
    return;
  }

  std::wcout << L"\nPress Enter to close...";
  std::wstring ignored;
  std::getline(std::wcin, ignored);
}

std::filesystem::path modsDirectoryForGame(const std::filesystem::path& geometryDashExe) {
  return geometryDashExe.parent_path() / L"geode" / L"mods";
}

bool isGeodeFile(const std::filesystem::path& path) {
  return equalsIgnoreCase(path.extension().wstring(), L".geode");
}

} // namespace geoode
