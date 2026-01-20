#pragma once
#include <gtkmm.h>

class MainApp final : public Gtk::Application {
public:
  static Glib::RefPtr<MainApp> create();

protected:
  MainApp();

  void on_activate() override;
};
