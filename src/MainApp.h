#pragma once

#include <gtkmm/application.h>
#include <glibmm/refptr.h>

#include "FontRegistry.h"

class MainApp : public Gtk::Application {
public:
  static Glib::RefPtr<MainApp> create();

protected:
  MainApp();

  void on_startup() override;
  void on_activate() override;

private:
  // Keep registry alive for entire app lifetime
  FontRegistry font_registry_;
};
