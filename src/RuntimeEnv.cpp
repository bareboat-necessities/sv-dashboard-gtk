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

#ifdef _WIN32
static std::string windowsDirUtf8() {
  wchar_t wwin[MAX_PATH];
  UINT n = GetWindowsDirectoryW(wwin, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return "C:\\Windows";
  std::filesystem::path p = std::filesystem::path(utf8_from_wide(wwin));
  return p.u8string(); // keep backslashes
}

static void writeTextFile(const std::string& path, const std::string& text) {
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
  FILE* f = nullptr;
  if (fopen_s(&f, path.c_str(), "wb") == 0 && f) {
    fwrite(text.data(), 1, text.size(), f);
    fclose(f);
  }
}

static bool canWriteHere(const std::string& dir) {
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  auto test = std::filesystem::path(dir) / ".__writetest";
  FILE* f = nullptr;
  if (fopen_s(&f, test.u8string().c_str(), "wb") != 0 || !f) return false;
  fputs("ok", f);
  fclose(f);
  std::filesystem::remove(test, ec);
  return true;
}
#endif

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
  // Keep PATH clean / prefer our bundled tools & DLLs
  const char* oldPath = std::getenv("PATH");
  setEnv("PATH", root + ";" + (oldPath ? std::string(oldPath) : std::string()));

  // Writable per-user base (works even if EXE is run from inside a zip)
  const std::string appBase   = localAppDataDir() + "/sv-dashboard-gtk";
  const std::string cacheHome = appBase + "/cache";
  ensureDir(cacheHome);                 // .../cache
  ensureDir(cacheHome + "/fontconfig"); // .../cache/fontconfig (optional but nice)

  setEnv("XDG_CACHE_HOME", cacheHome);

  // Optional: helps some stacks, harmless
  setEnv("HOME", appBase);

  // Point fontconfig at our bundled config
  setEnv("FONTCONFIG_PATH", root + "/etc/fonts");
  setEnv("FONTCONFIG_FILE", root + "/etc/fonts/fonts.conf");

  // App resource locations
  setEnv("SV_DASHBOARD_FONT_DIR", root + "/share/sv-dashboard-gtk/fonts");
  setEnv("GSETTINGS_SCHEMA_DIR", root + "/share/glib-2.0/schemas");
  setEnv("GDK_PIXBUF_MODULEDIR", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders");
  setEnv("GDK_PIXBUF_MODULE_FILE", root + "/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache");
#endif
}
