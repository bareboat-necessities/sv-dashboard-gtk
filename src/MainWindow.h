#pragma once

#include <gtkmm.h>

class MainWindow : public Gtk::Window {
public:
  MainWindow();

private:
  enum class Scheme { Day, Dusk, Night };

  void apply_css_provider_once();
  Glib::ustring build_css(Scheme s) const;
  void set_scheme(Scheme s);
  void refresh_scheme_buttons();

  void show_page(const Glib::ustring& name);
  void refresh_nav();

  bool on_key_press(GdkEventKey* e);

  // Layout
  Gtk::Overlay overlay_;
  Gtk::Box     root_{Gtk::ORIENTATION_HORIZONTAL};

  Gtk::Stack   stack_;
  Gtk::Button  btn_left_;
  Gtk::Button  btn_right_;

  // Bottom-left scheme buttons
  Gtk::Box     scheme_bar_{Gtk::ORIENTATION_HORIZONTAL};
  Gtk::Button  scheme_day_;
  Gtk::Button  scheme_dusk_;
  Gtk::Button  scheme_night_;

  // One provider, reloaded on scheme change
  Glib::RefPtr<Gtk::CssProvider> css_provider_;

  Scheme scheme_ = Scheme::Day;
};
