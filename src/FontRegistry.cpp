#include "FontRegistry.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>

#include <iostream>
#include <string>

#include <pango/pangocairo.h>
#include <pango/pango.h>

#if __has_include(<pango/pangofc-fontmap.h>)
  #include <pango/pangofc-fontmap.h>
  #define SV_HAVE_PANGO_FC 1
#else
  #define SV_HAVE_PANGO_FC 0
#endif

#ifdef _WIN32
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
  const auto dir = findFontDir();
  if (dir.empty()) {
    std::cerr
      << "FontRegistry: could not locate bundled font directory.\n"
      << "Run ./scripts/fetch-fontawesome.sh or bundle fonts with the app.\n";
    return false;
  }

  // Sanity check: required files exist
  char* solid  = g_build_filename(dir.c_str(), "fa-solid-900.ttf", nullptr);
  char* brands = g_build_filename(dir.c_str(), "fa-brands-400.ttf", nullptr);

  const bool have_solid  = solid  && g_file_test(solid,  G_FILE_TEST_IS_REGULAR);
  const bool have_brands = brands && g_file_test(brands, G_FILE_TEST_IS_REGULAR);
  g_free(solid);
  g_free(brands);

  if (!have_solid || !have_brands) {
    std::cerr << "FontRegistry: missing FA font files in: " << dir << "\n";
    return false;
  }

  // ---- Path A: PangoFc backend (Fontconfig/FreeType) ----
#if SV_HAVE_PANGO_FC
  {
    // Ensure fontconfig is initialized using whatever FONTCONFIG_FILE/PATH points at
    FcConfig* cfg = FcInitLoadConfigAndFonts();
    if (!cfg) {
      std::cerr << "FontRegistry: FcInitLoadConfigAndFonts failed\n";
      // fall through to Win32 private-font install if available
    } else {
#ifdef _WIN32
      // Fontconfig behaves better with forward slashes on Windows
      std::string norm = dir;
      for (auto& ch : norm) if (ch == '\\') ch = '/';
      const FcBool ok = FcConfigAppFontAddDir(cfg, reinterpret_cast<const FcChar8*>(norm.c_str()));
#else
      const FcBool ok = FcConfigAppFontAddDir(cfg, reinterpret_cast<const FcChar8*>(dir.c_str()));
#endif
      if (!ok) {
        std::cerr << "FontRegistry: FcConfigAppFontAddDir failed for: " << dir << "\n";
      } else {
        // Rebuild font sets
        FcConfigBuildFonts(cfg);

        // Attach config to the *actual* Pango font map, so it sees app fonts.
        PangoFontMap* fm = pango_cairo_font_map_get_default();
        if (fm && PANGO_IS_FC_FONT_MAP(fm)) {
          pango_fc_font_map_set_config(PANGO_FC_FONT_MAP(fm), cfg);
          pango_fc_font_map_cache_clear(PANGO_FC_FONT_MAP(fm));
          pango_font_map_changed(fm);
          return true;
        }

        // If we’re here: Pango is not using Fc backend; fall through to Win32 path.
      }
    }
  }
#endif // SV_HAVE_PANGO_FC

  // ---- Path B: Windows Win32 backend (GDI) ----
#ifdef _WIN32
  {
    // Private per-process font install (no admin, no system install).
    int added = 0;

    GDir* gd = g_dir_open(dir.c_str(), 0, nullptr);
    if (!gd) {
      std::cerr << "FontRegistry: g_dir_open failed for: " << dir << "\n";
      return false;
    }

    for (const char* name = g_dir_read_name(gd); name; name = g_dir_read_name(gd)) {
      // crude suffix test (case-insensitive enough for our shipped filenames)
      if (!g_str_has_suffix(name, ".ttf") && !g_str_has_suffix(name, ".TTF") &&
          !g_str_has_suffix(name, ".otf") && !g_str_has_suffix(name, ".OTF")) {
        continue;
      }

      char* full = g_build_filename(dir.c_str(), name, nullptr);
      if (!full) continue;

      if (g_file_test(full, G_FILE_TEST_IS_REGULAR)) {
        gunichar2* w = g_utf8_to_utf16(full, -1, nullptr, nullptr, nullptr);
        if (w) {
          if (AddFontResourceExW(reinterpret_cast<const wchar_t*>(w), FR_PRIVATE, nullptr) > 0) {
            ++added;
          }
          g_free(w);
        }
      }

      g_free(full);
    }

    g_dir_close(gd);

    if (added > 0) {
      // Let GDI/PangoWin32 refresh font list
      SendMessageW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
      return true;
    }

    std::cerr << "FontRegistry: AddFontResourceExW added 0 fonts from: " << dir << "\n";
    return false;
  }
#else
  return false;
#endif
}
