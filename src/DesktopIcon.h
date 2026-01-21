#pragma once

#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <glibmm/ustring.h>
#include <string>

#include "Icons.h"

class DesktopIcon : public Gtk::Button {
public:
  explicit DesktopIcon(const IconSpec& spec);

  void set_ui_scale(double s, bool show_label);
  void set_color_class(const std::string& cls);

private:
  static Glib::ustring to_utf8(char32_t cp);
  void apply_fonts(double s);

  Gtk::Box      box_{Gtk::ORIENTATION_VERTICAL};

  // Background is drawn on this fixed-size square
  Gtk::EventBox icon_box_;
  Gtk::Label    icon_;
  Gtk::Label    text_;

  bool is_brand_ = false;
  std::string color_class_;
};
