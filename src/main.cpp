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

  gboolean opt_fullscreen = FALSE;
  gboolean opt_no_decorations = FALSE;

  GOptionEntry entries[] = {
      {"fullscreen", 'f', 0, G_OPTION_ARG_NONE, &opt_fullscreen,
       "Launch fullscreen", nullptr},
      {"no-decorations", 0, 0, G_OPTION_ARG_NONE, &opt_no_decorations,
       "Disable window decorations", nullptr},
      {nullptr}
  };

  GOptionContext* context = g_option_context_new(nullptr);
  g_option_context_add_main_entries(context, entries, nullptr);
  GError* error = nullptr;
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    std::cerr << "Failed to parse options: " << error->message << "\n";
    g_error_free(error);
  }
  g_option_context_free(context);

  const bool fullscreen = opt_fullscreen;
  const bool decorated = !opt_no_decorations;

  auto app = MainApp::create(fullscreen, decorated);
  return app->run(argc, argv);
}
