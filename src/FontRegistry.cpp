#include "FontRegistry.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>

#include <iostream>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <unistd.h>
#endif

void FontRegistry::setFontDirOverride(std::string dir) {
  font_dir_override_ = std::move(dir);
}

std::string FontRegistry::exeDir() {
#ifdef _WIN32
  wchar_t wpath[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, wpath, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return {};

  // Convert wide -> UTF-8 using GLib
  gchar* utf8 = g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(wpath),
                                -1, nullptr, nullptr, nullptr);
  if (!utf8) return {};

  char* dir_c = g_path_get_dirname(utf8);
  std::string out = dir_c ? dir_c : "";

  g_free(dir_c);
  g_free(utf8);
  return out;
#else
  char buf[4096] = {0};
  const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (n <= 0) return {};
  buf[n] = '\0';

  char* d = g_path_get_dirname(buf);
  std::string out = d ? d : "";
  g_free(d);
  return out;
#endif
}

std::string FontRegistry::findFontDir() const {
  if (!font_dir_override_.empty()) return font_dir_override_;

  // Env override (works on Linux + Windows; run.bat sets this)
  if (const char* env = g_getenv("SV_DASHBOARD_FONT_DIR"); env && *env) {
    return std::string(env);
  }

  // Dev: ./assets/fonts relative to CWD
  if (g_file_test("assets/fonts", G_FILE_TEST_IS_DIR)) return "assets/fonts";

  // Relative to executable
  const auto ed = exeDir();
  if (!ed.empty()) {
    // Portable bundle layout: <exe_dir>/share/sv-dashboard-gtk/fonts
    {
      char* p = g_build_filename(ed.c_str(), "share", "sv-dashboard-gtk", "fonts", nullptr);
      std::string candidate = p ? p : "";
      g_free(p);
      if (!candidate.empty() && g_file_test(candidate.c_str(), G_FILE_TEST_IS_DIR)) return candidate;
    }

    // Installed layout: <exe_dir>/../share/sv-dashboard-gtk/fonts
    {
      char* p = g_build_filename(ed.c_str(), "..", "share", "sv-dashboard-gtk", "fonts", nullptr);
      std::string candidate = p ? p : "";
      g_free(p);
      if (!candidate.empty() && g_file_test(candidate.c_str(), G_FILE_TEST_IS_DIR)) return candidate;
    }

    // Optional: flat portable layout: <exe_dir>/fonts
    {
      char* p = g_build_filename(ed.c_str(), "fonts", nullptr);
      std::string candidate = p ? p : "";
      g_free(p);
      if (!candidate.empty() && g_file_test(candidate.c_str(), G_FILE_TEST_IS_DIR)) return candidate;
    }
  }

  // Common install locations (Linux)
  const char* prefixes[] = { "/usr/local/share", "/usr/share", nullptr };
  for (int i = 0; prefixes[i]; ++i) {
    char* p = g_build_filename(prefixes[i], "sv-dashboard-gtk", "fonts", nullptr);
    std::string candidate = p ? p : "";
    g_free(p);
    if (!candidate.empty() && g_file_test(candidate.c_str(), G_FILE_TEST_IS_DIR)) return candidate;
  }

  return {};
}

bool FontRegistry::registerBundledFonts() {
  const char* dirEnv = std::getenv("SV_DASHBOARD_FONT_DIR");
  if (!dirEnv || !*dirEnv) return false;

  std::filesystem::path fontDir(dirEnv);
  if (!std::filesystem::exists(fontDir)) return false;

#ifdef _WIN32
  int added = 0;

  for (const auto& it : std::filesystem::directory_iterator(fontDir)) {
    if (!it.is_regular_file()) continue;

    auto ext = it.path().extension().wstring();
    if (_wcsicmp(ext.c_str(), L".ttf") != 0 && _wcsicmp(ext.c_str(), L".otf") != 0)
      continue;

    const std::wstring wpath = it.path().wstring();

    // Install privately for this process only (no admin, no system install)
    if (AddFontResourceExW(wpath.c_str(), FR_PRIVATE, nullptr) > 0) {
      ++added;
    }
  }

  if (added > 0) {
    // Force GDI/PangoWin32 to refresh font list
    SendMessageW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    return true;
  }
  return false;

#else
  // Your existing Linux/fontconfig path here
  return false;
#endif
}