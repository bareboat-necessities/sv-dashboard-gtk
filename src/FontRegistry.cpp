#include "FontRegistry.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <unistd.h>

#include <iostream>

void FontRegistry::setFontDirOverride(std::string dir) {
  font_dir_override_ = std::move(dir);
}

std::string FontRegistry::exeDir() {
  char buf[4096] = {0};
  const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (n <= 0) return {};
  buf[n] = '\0';

  char* d = g_path_get_dirname(buf);
  std::string out = d ? d : "";
  g_free(d);
  return out;
}

std::string FontRegistry::findFontDir() const {
  if (!font_dir_override_.empty()) return font_dir_override_;

  // 1) Dev: ./assets/fonts relative to CWD
  if (g_file_test("assets/fonts", G_FILE_TEST_IS_DIR)) return "assets/fonts";

  // 2) Installed: <exe_dir>/../share/sv-dashboard-gtk/fonts
  const auto ed = exeDir();
  if (!ed.empty()) {
    char* p = g_build_filename(ed.c_str(), "..", "share", "sv-dashboard-gtk", "fonts", nullptr);
    std::string candidate = p ? p : "";
    g_free(p);
    if (!candidate.empty() && g_file_test(candidate.c_str(), G_FILE_TEST_IS_DIR)) return candidate;
  }

  // 3) Common install locations
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
      << "Run ./scripts/fetch-fontawesome.sh or install to /usr/share/sv-dashboard-gtk/fonts\n";
    return false;
  }

  char* solid  = g_build_filename(dir.c_str(), "fa-solid-900.ttf", nullptr);
  char* brands = g_build_filename(dir.c_str(), "fa-brands-400.ttf", nullptr);

  const bool have_solid  = solid  && g_file_test(solid,  G_FILE_TEST_IS_REGULAR);
  const bool have_brands = brands && g_file_test(brands, G_FILE_TEST_IS_REGULAR);

  if (!have_solid || !have_brands) {
    std::cerr << "FontRegistry: missing required font files in " << dir << "\n";
    if (solid)  std::cerr << "  expected: " << solid  << "\n";
    if (brands) std::cerr << "  expected: " << brands << "\n";
    g_free(solid);
    g_free(brands);
    return false;
  }

  if (!FcInit()) {
    std::cerr << "FontRegistry: FcInit failed\n";
    g_free(solid);
    g_free(brands);
    return false;
  }

  FcConfig* cfg = FcConfigGetCurrent();
  if (!cfg) {
    std::cerr << "FontRegistry: FcConfigGetCurrent failed\n";
    g_free(solid);
    g_free(brands);
    return false;
  }

  const FcBool ok1 = FcConfigAppFontAddFile(cfg, reinterpret_cast<const FcChar8*>(solid));
  const FcBool ok2 = FcConfigAppFontAddFile(cfg, reinterpret_cast<const FcChar8*>(brands));

  FcConfigBuildFonts(cfg);

  g_free(solid);
  g_free(brands);

  if (!ok1 || !ok2) {
    std::cerr << "FontRegistry: failed adding one or more fonts via fontconfig\n";
    return false;
  }

  return true;
}
