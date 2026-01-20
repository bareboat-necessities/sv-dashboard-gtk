#include "RuntimeEnv.h"

#include <glib.h>
#include <string>
#include <vector>

#ifdef _WIN32
  #include <windows.h>
#endif

namespace {

#ifdef _WIN32

static std::string exe_dir() {
  wchar_t wpath[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, wpath, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return {};

  // UTF-16 -> UTF-8 using GLib
  gchar* utf8 = g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(wpath),
                                -1, nullptr, nullptr, nullptr);
  if (!utf8) return {};

  gchar* dir_c = g_path_get_dirname(utf8);
  std::string out = dir_c ? dir_c : "";

  g_free(dir_c);
  g_free(utf8);
  return out;
}

static std::string join(std::initializer_list<std::string> parts) {
  std::vector<const gchar*> v;
  v.reserve(parts.size() + 1);
  for (const auto& s : parts) v.push_back(s.c_str());
  v.push_back(nullptr);

  gchar* p = g_build_filenamev(const_cast<gchar**>(v.data()));
  std::string out = p ? p : "";
  g_free(p);
  return out;
}

static bool is_dir(const std::string& p) {
  return !p.empty() && g_file_test(p.c_str(), G_FILE_TEST_IS_DIR);
}
static bool is_file(const std::string& p) {
  return !p.empty() && g_file_test(p.c_str(), G_FILE_TEST_IS_REGULAR);
}

static void mkdirp(const std::string& p) {
  if (!p.empty()) g_mkdir_with_parents(p.c_str(), 0700);
}

static void prepend_path_if(const std::string& dir, const std::string& marker_exe) {
  // Only touch PATH if we actually ship the marker exe (e.g. gdbus.exe)
  if (!is_file(marker_exe)) return;

  const char* old = g_getenv("PATH");
  std::string np = dir;
  if (old && *old) {
    np += ";";
    np += old;
  }
  g_setenv("PATH", np.c_str(), TRUE);
}

#endif

} // namespace

void RuntimeEnv::setup() {
#ifndef _WIN32
  return;
#else
  const auto ed = exe_dir();
  if (ed.empty()) return;

  // Detect "portable bundle" by presence of support dirs/files next to EXE
  const auto fonts_dir   = join({ed, "share", "sv-dashboard-gtk", "fonts"});
  const auto fc_file     = join({ed, "etc", "fonts", "fonts.conf"});
  const auto fc_path     = join({ed, "etc", "fonts"});
  const auto schemas_dir = join({ed, "share", "glib-2.0", "schemas"});
  const auto pix_cache   = join({ed, "lib", "gdk-pixbuf-2.0", "2.10.0", "loaders.cache"});
  const auto pix_moddir  = join({ed, "lib", "gdk-pixbuf-2.0", "2.10.0", "loaders"});
  const auto gdbus_exe   = join({ed, "gdbus.exe"});

  const bool looks_portable =
      is_dir(fonts_dir) || is_file(fc_file) || is_dir(schemas_dir) || is_file(pix_cache);

  if (!looks_portable) return;

  // Ensure our bundled gdbus.exe wins over cygwin/git-bash ones by PATH precedence.
  // (GIO searches for gdbus.exe; recommended to ship it next to GIO DLLs) :contentReference[oaicite:2]{index=2}
  prepend_path_if(ed, gdbus_exe);

  // Ensure HOME is set (some Windows environments don’t have it)
  if (!g_getenv("HOME")) {
    const char* home = g_get_home_dir();
    if (home && *home) g_setenv("HOME", home, TRUE);
  }

  // Ensure a writable cache dir for fontconfig:
  // default cache is under $XDG_CACHE_HOME/fontconfig :contentReference[oaicite:3]{index=3}
  if (!g_getenv("XDG_CACHE_HOME")) {
    const char* ucd = g_get_user_cache_dir(); // usually %LOCALAPPDATA%\... on Windows
    if (ucd && *ucd) {
      gchar* p = g_build_filename(ucd, "sv-dashboard-gtk", nullptr);
      std::string xdg = p ? p : "";
      g_free(p);

      if (!xdg.empty()) {
        mkdirp(xdg);
        mkdirp(join({xdg, "fontconfig"}));
        g_setenv("XDG_CACHE_HOME", xdg.c_str(), TRUE);
      }
    }
  }

  // Point GTK stack to bundled data/config
  if (is_dir(fonts_dir))   g_setenv("SV_DASHBOARD_FONT_DIR", fonts_dir.c_str(), TRUE);

  // FONTCONFIG_FILE / FONTCONFIG_PATH are documented env vars :contentReference[oaicite:4]{index=4}
  if (is_file(fc_file))    g_setenv("FONTCONFIG_FILE", fc_file.c_str(), TRUE);
  if (is_dir(fc_path))     g_setenv("FONTCONFIG_PATH", fc_path.c_str(), TRUE);

  if (is_dir(schemas_dir)) g_setenv("GSETTINGS_SCHEMA_DIR", schemas_dir.c_str(), TRUE);
  if (is_file(pix_cache))  g_setenv("GDK_PIXBUF_MODULE_FILE", pix_cache.c_str(), TRUE);
  if (is_dir(pix_moddir))  g_setenv("GDK_PIXBUF_MODULEDIR", pix_moddir.c_str(), TRUE);
#endif
}
