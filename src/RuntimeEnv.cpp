#include "RuntimeEnv.h"

#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <shlobj.h>
#endif

using namespace std;

static string utf8_from_wide(const wchar_t* w) {
#ifdef _WIN32
  if (!w) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
  if (n <= 0) return {};
  string s;
  s.resize((size_t)n - 1);
  WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
  return s;
#else
  (void)w;
  return {};
#endif
}

string RuntimeEnv::toForwardSlashes(string s) {
  for (char& c : s) if (c == '\\') c = '/';
  return s;
}

void RuntimeEnv::ensureDir(const string& path) {
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::path(path), ec);
}

void RuntimeEnv::setEnv(const char* key, const string& val) {
#ifdef _WIN32
  _putenv_s(key, val.c_str());
#else
  setenv(key, val.c_str(), 1);
#endif
}

string RuntimeEnv::exeDir() {
#ifdef _WIN32
  wchar_t buf[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return ".";
  std::filesystem::path p = std::filesystem::path(utf8_from_wide(buf)).parent_path();
  return toForwardSlashes(p.generic_string());  // returns std::string with '/' separators
#else
  // Linux/macOS: keep your existing implementation
  return ".";
#endif
}

#ifdef _WIN32
string RuntimeEnv::localAppDataDir() {
  PWSTR wpath = nullptr;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wpath);
  if (FAILED(hr) || !wpath) return exeDir();
  string out = toForwardSlashes(utf8_from_wide(wpath));
  CoTaskMemFree(wpath);
  return out;
}
#endif

void RuntimeEnv::setup() {
  const string root = exeDir(); // where sv-dashboard.exe lives

#ifdef _WIN32
  // Put our folder first in PATH so GLib doesn't pick up Cygwin's gdbus.exe, etc.
  const char* oldPath = getenv("PATH");
  string newPath = root + ";" + (oldPath ? string(oldPath) : string());
  setEnv("PATH", newPath);

  // Use a guaranteed-writable per-user cache directory for fontconfig
  const string appBase = localAppDataDir() + "/sv-dashboard-gtk";
  const string cacheHome = appBase + "/cache";
  const string fcCache = cacheHome + "/fontconfig";
  ensureDir(fcCache);

  setEnv("HOME", appBase);                 // harmless; helps some stacks
  setEnv("XDG_CACHE_HOME", cacheHome);     // fontconfig cachedir prefix="xdg" uses this :contentReference[oaicite:3]{index=3}

  // Point to our bundled fontconfig config
  setEnv("FONTCONFIG_PATH", root + "/etc/fonts");
  setEnv("FONTCONFIG_FILE", root + "/etc/fonts/fonts.conf");

  // App resource locations
  setEnv("SV_DASHBOARD_FONT_DIR", root + "/share/sv-dashboard-gtk/fonts");
  setEnv("GSETTINGS_SCHEMA_DIR", root + "/share/glib-2.0/schemas");
  setEnv("GDK_PIXBUF_MODULEDIR", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders");
  setEnv("GDK_PIXBUF_MODULE_FILE", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache");
#endif
}
