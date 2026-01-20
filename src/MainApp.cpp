#include "MainApp.h"
#include "MainWindow.h"
#include "FontRegistry.h"

#include <iostream>

Glib::RefPtr<MainApp> MainApp::create() {
  return Glib::RefPtr<MainApp>(new MainApp());
}

MainApp::MainApp()
: Gtk::Application("com.example.sv_dashboard")
{}

void MainApp::on_activate() {
  // Register bundled fonts before UI
  FontRegistry reg;
  if (!reg.registerBundledFonts()) {
    std::cerr << "Failed to register bundled Font Awesome fonts.\n";
    // We still start; icons may show as squares.
  }

  auto* win = new MainWindow();
  add_window(*win);
  win->signal_hide().connect([win] { delete win; });
  win->present();
}
