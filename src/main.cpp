#include "MainApp.h"
#include "FontRegistry.h"
#include "RuntimeEnv.h"

#include <glib.h>
#include <iostream>

int main(int argc, char** argv) {
#ifdef _WIN32
  // Don’t let GLib try to autolaunch D-Bus on Windows.
  if (!g_getenv("GSETTINGS_BACKEND")) {
    g_setenv("GSETTINGS_BACKEND", "memory", TRUE);
  }

  // If started from MSYS/Cygwin shells, only clear fontconfig *config* overrides.
  // DO NOT clear HOME/XDG_CACHE_HOME/FC_CACHEDIR: those are exactly what make fontconfig writable.
  const char* vars_to_clear[] = {
      "FONTCONFIG_FILE",
      "FONTCONFIG_PATH",
      nullptr
  };
  for (int i = 0; vars_to_clear[i]; ++i) {
    g_unsetenv(vars_to_clear[i]);
  }
#endif

  RuntimeEnv::setup(); // MUST run before Gtk::Application::create() / any Pango usage

  FontRegistry reg;
  if (!reg.registerBundledFonts()) {
    std::cerr << "Warning: FA fonts not registered; icons may fall back.\n";
  }

  auto app = MainApp::create();
  return app->run(argc, argv);
}
