#pragma once

#include <gtkmm/application.h>
#include <glibmm/refptr.h>

#include "FontRegistry.h"

class MainApp : public Gtk::Application {
public:
  static Glib::RefPtr<MainApp> create(bool fullscreen, bool decorated, bool skip_taskbar);

protected:
  MainApp(bool fullscreen, bool decorated, bool skip_taskbar);

  void on_startup() override;
  void on_activate() override;

private:
  // Keep registry alive for entire app lifetime
  FontRegistry font_registry_;
  bool fullscreen_ = false;
  bool decorated_ = true;
  bool skip_taskbar_ = false;
};
