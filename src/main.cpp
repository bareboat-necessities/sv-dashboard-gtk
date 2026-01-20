#include "MainApp.h"
#include "FontRegistry.h"
#include "RuntimeEnv.h"

#include <glib.h>
#include <iostream>

int main(int argc, char** argv) {
#ifdef _WIN32
  // Don’t let GLib try to autolaunch D-Bus on Windows.
  if (!g_getenv("GSETTINGS_BACKEND")) {
    RuntimeEnv::setEnv("GSETTINGS_BACKEND", "memory");
  }

  // If user runs from Cygwin/MSYS shells with conflicting env, ignore them.
  const char* vars_to_clear[] = {
      "FONTCONFIG_FILE",
      "FONTCONFIG_PATH",
      "FC_CACHEDIR",
      "XDG_CACHE_HOME",
      "HOME",
      nullptr
  };
  for (int i = 0; vars_to_clear[i]; ++i) {
    RuntimeEnv::unsetEnv(vars_to_clear[i]);
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
