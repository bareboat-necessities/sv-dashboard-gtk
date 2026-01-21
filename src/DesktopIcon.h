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

  // Used by Desktop/MainWindow scaling
  void set_ui_scale(double s, bool show_label);

  // Used by Desktop.cpp to assign bg color class
  void set_color_class(const std::string& cls);

private:
  static Glib::ustring to_utf8(char32_t cp);
  void apply_fonts(double s);

  Gtk::Box      box_{Gtk::ORIENTATION_VERTICAL};

  // Square background container (this fixes "rectangles" for wide glyphs)
  Gtk::EventBox icon_box_;
  Gtk::Label    icon_;
  Gtk::Label    text_;

  bool is_brand_ = false;
  std::string color_class_;
};
