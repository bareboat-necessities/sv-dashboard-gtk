#pragma once

#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <glibmm/ustring.h>

#include "Icons.h"

class DesktopIcon : public Gtk::Button {
public:
  explicit DesktopIcon(const IconSpec& spec);

  // NEW: called by Desktop/MainWindow to scale fonts + optionally hide labels
  void set_ui_scale(double s, bool show_label);

private:
  static Glib::ustring to_utf8(char32_t cp);
  void apply_fonts(double s);

  Gtk::Box   box_{Gtk::ORIENTATION_VERTICAL};
  Gtk::Label icon_;
  Gtk::Label text_;

  bool is_brand_ = false;
};
