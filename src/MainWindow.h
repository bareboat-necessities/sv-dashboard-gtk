#pragma once
#include <gtkmm.h>
#include <memory>

class MainWindow final : public Gtk::Window {
public:
  MainWindow();

private:
  Gtk::Box root_{Gtk::ORIENTATION_HORIZONTAL};
  Gtk::Stack stack_;
  Gtk::Button btn_left_;
  Gtk::Button btn_right_;

  void apply_css();
  void show_page(const Glib::ustring& name);
  void refresh_nav();

  bool on_key_press(GdkEventKey* e);
};
