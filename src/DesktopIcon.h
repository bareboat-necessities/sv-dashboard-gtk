#pragma once
#include <gtkmm.h>
#include "Icons.h"

// A tile: big FA glyph + label, clickable (button).
class DesktopIcon final : public Gtk::Button {
public:
  explicit DesktopIcon(const IconSpec& spec);

private:
  Gtk::Box box_{Gtk::ORIENTATION_VERTICAL};
  Gtk::Label icon_;
  Gtk::Label text_;

  static Glib::ustring to_utf8(char32_t cp);

  void apply_fonts(const IconSpec& spec);
};
