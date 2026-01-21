#pragma once

#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <glibmm/ustring.h>
#include <pangomm/fontdescription.h>

#include <string>

#include "Icons.h"

class DesktopIcon : public Gtk::Button {
public:
  explicit DesktopIcon(const IconSpec& spec);

  void set_ui_scale(double s, bool show_label);
  void set_color_class(const std::string& cls);

private:
  static Glib::ustring to_utf8(char32_t cp);

  class IconCanvas : public Gtk::DrawingArea {
  public:
    IconCanvas();

    void set_glyph(const Glib::ustring& g);
    void set_font(const Pango::FontDescription& fd);
    void set_box_px(int px);

  protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

    // Hard-force a fixed square preferred size so boxes NEVER differ.
    void get_preferred_width_vfunc(int& min_w, int& nat_w) const override;
    void get_preferred_height_vfunc(int& min_h, int& nat_h) const override;

  private:
    Glib::ustring glyph_;
    Pango::FontDescription font_;

    int box_px_ = 112;
    int glyph_px_ = 56;

    void update_glyph_px_();
  };

  void apply_fonts(double s);

  Gtk::Box   box_{Gtk::ORIENTATION_VERTICAL};
  IconCanvas icon_box_;
  Gtk::Label text_;

  bool is_brand_ = false;
  std::string color_class_;
};
