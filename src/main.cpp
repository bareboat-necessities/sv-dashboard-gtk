#include "MainApp.h"
#include "FontRegistry.h"
#include <glib.h>
#include <iostream>

int main(int argc, char** argv) {
#ifdef _WIN32
  // Avoid GLib trying to autolaunch a D-Bus session bus on Windows.
  g_setenv("GSETTINGS_BACKEND", "memory", TRUE);

  // If user runs from Cygwin/MSYS shells with weird fontconfig vars, ignore them.
  g_unsetenv("FONTCONFIG_FILE");
  g_unsetenv("FONTCONFIG_PATH");
#endif

  FontRegistry reg;
  if (!reg.registerBundledFonts()) {
    std::cerr << "Warning: FA fonts not registered; icons may fall back.\n";
  }

  auto app = MainApp::create();
  return app->run(argc, argv);
}
