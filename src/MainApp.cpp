#include "MainApp.h"
#include "MainWindow.h"

#include <iostream>

Glib::RefPtr<MainApp> MainApp::create(bool fullscreen, bool decorated, bool skip_taskbar) {
  return Glib::RefPtr<MainApp>(new MainApp(fullscreen, decorated, skip_taskbar));
}

MainApp::MainApp(bool fullscreen, bool decorated, bool skip_taskbar)
: Gtk::Application("github.bbn.sv_dashboard"),
  fullscreen_(fullscreen),
  decorated_(decorated),
  skip_taskbar_(skip_taskbar)
{}

void MainApp::on_startup() {
  Gtk::Application::on_startup();

  // Register bundled fonts before UI is created
  if (!font_registry_.registerBundledFonts()) {
    std::cerr << "Failed to register bundled Font Awesome fonts.\n";
  }

  // NOTE: On some pangomm versions there is no Pango::CairoFontMap::get_default().
  // It's safe to omit the font-map refresh; restart or early registration usually suffices.
}

void MainApp::on_activate() {
  auto* win = new MainWindow(fullscreen_, decorated_, skip_taskbar_);
  add_window(*win);
  win->signal_hide().connect([win] { delete win; });
  win->present();
}
