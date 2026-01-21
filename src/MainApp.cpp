#include "MainApp.h"
#include "MainWindow.h"

#include <pangomm/cairofontmap.h>
#include <iostream>

Glib::RefPtr<MainApp> MainApp::create() {
  return Glib::RefPtr<MainApp>(new MainApp());
}

MainApp::MainApp()
: Gtk::Application("com.example.sv_dashboard")
{}

void MainApp::on_startup() {
  Gtk::Application::on_startup();

  // Register bundled fonts before UI is created
  if (!font_registry_.registerBundledFonts()) {
    std::cerr << "Failed to register bundled Font Awesome fonts.\n";
  }

  // Nudge Pango to refresh font map (helps when fonts are added at runtime)
  if (auto fm = Pango::CairoFontMap::get_default()) {
    fm->changed();
  }
}

void MainApp::on_activate() {
  auto* win = new MainWindow();
  add_window(*win);
  win->signal_hide().connect([win] { delete win; });
  win->present();
}
