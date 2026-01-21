#include "MainApp.h"
#include "MainWindow.h"

#include <iostream>

Glib::RefPtr<MainApp> MainApp::create() {
  return Glib::RefPtr<MainApp>(new MainApp());
}

MainApp::MainApp()
: Gtk::Application("github.bbn.sv_dashboard")
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
  auto* win = new MainWindow();
  add_window(*win);
  win->signal_hide().connect([win] { delete win; });
  win->present();
}
