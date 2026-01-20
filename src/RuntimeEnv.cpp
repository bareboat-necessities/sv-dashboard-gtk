#include "RuntimeEnv.h"

#include <cstdlib>
#include <filesystem>
#include <string>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <shlobj.h>
#endif

using namespace std;

#ifdef _WIN32
namespace {

// Wide → UTF-8 helper (FIXED: no overflow)
static string utf8_from_wide(const wchar_t* w) {
  if (!w) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
  if (n <= 0) return {};
  string s;
  s.resize(n); // includes NUL
  WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
  s.resize(n - 1); // drop NUL
  return s;
}

} // namespace
#endif

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
  // Make env visible to BOTH CRT getenv() and WinAPI readers.
  _putenv_s(key, val.c_str());
  SetEnvironmentVariableA(key, val.c_str());
#else
  setenv(key, val.c_str(), 1);
#endif
}

void RuntimeEnv::unsetEnv(const char* key) {
#ifdef _WIN32
  // Clear for BOTH CRT and WinAPI.
  _putenv_s(key, "");
  SetEnvironmentVariableA(key, nullptr);
#else
  unsetenv(key);
#endif
}

string RuntimeEnv::exeDir() {
#ifdef _WIN32
  wchar_t buf[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return ".";
  std::filesystem::path p = utf8_from_wide(buf);
  return toForwardSlashes(p.parent_path().string());
#else
  return ".";
#endif
}

#ifdef _WIN32
string RuntimeEnv::localAppDataDir() {
  PWSTR wpath = nullptr;
  if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wpath)) || !wpath)
    return exeDir();

  string out = toForwardSlashes(utf8_from_wide(wpath));
  CoTaskMemFree(wpath);
  return out;
}
#endif

void RuntimeEnv::setup() {
#ifdef _WIN32
  const string root = exeDir();

  // Prefer bundled DLLs/tools
  if (const char* oldPath = getenv("PATH"))
    setEnv("PATH", root + ";" + oldPath);
  else
    setEnv("PATH", root);

  // Writable per-user base
  const string appBase   = localAppDataDir() + "/sv-dashboard-gtk";
  const string cacheHome = appBase + "/cache";
  const string fcCache   = cacheHome + "/fontconfig";

  ensureDir(appBase);
  ensureDir(cacheHome);
  ensureDir(fcCache);

  // Some configs still probe these legacy locations (safe to create)
  ensureDir(appBase + "/.cache/fontconfig");
  ensureDir(appBase + "/.fontconfig");

  // Make fontconfig pick a writable cache directory (hard override)
  setEnv("HOME", appBase);
  setEnv("XDG_CACHE_HOME", cacheHome);
  setEnv("FC_CACHEDIR", fcCache);

  // Fontconfig: read-only config, writable cache
  setEnv("FONTCONFIG_PATH", root + "/etc/fonts");
  setEnv("FONTCONFIG_FILE", root + "/etc/fonts/fonts.conf");

  // App resources
  setEnv("SV_DASHBOARD_FONT_DIR", root + "/share/sv-dashboard-gtk/fonts");
  setEnv("GSETTINGS_SCHEMA_DIR", root + "/share/glib-2.0/schemas");
  setEnv("GDK_PIXBUF_MODULEDIR", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders");
  setEnv("GDK_PIXBUF_MODULE_FILE", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache");
#endif
}
