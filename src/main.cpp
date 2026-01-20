#include "MainApp.h"
#include "FontRegistry.h"
#include "RuntimeEnv.h"

#include <glib.h>
#include <iostream>

int main(int argc, char** argv) {
  RuntimeEnv::setup(); // MUST run before Gtk::Application::create() / any Pango usage

#ifdef _WIN32
  // Don’t let GLib try to talk to / autolaunch D-Bus on Windows.
  // Only set if user hasn't explicitly set it.
  if (!g_getenv("GSETTINGS_BACKEND")) {
    g_setenv("GSETTINGS_BACKEND", "memory", TRUE);
  }

  // If user runs from Cygwin/MSYS shells with conflicting env, ignore them.
  // RuntimeEnv::setup() will set correct bundle values.
  const char* vars_to_clear[] = {
      "FONTCONFIG_FILE",
      "FONTCONFIG_PATH",
      "FC_CACHEDIR",
      "XDG_CACHE_HOME",
      "HOME",
      nullptr
  };
  for (int i = 0; vars_to_clear[i]; ++i) {
    g_unsetenv(vars_to_clear[i]);
  }
#endif

  FontRegistry reg;
  if (!reg.registerBundledFonts()) {
    std::cerr << "Warning: FA fonts not registered; icons may fall back.\n";
  }

  auto app = MainApp::create();
  return app->run(argc, argv);
}
