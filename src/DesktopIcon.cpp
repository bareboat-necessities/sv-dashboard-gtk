#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <glib.h>
#include <pango/pangocairo.h>

#include <cmath>

// Under-icon label
static constexpr int LABEL_PX_BASE = 20;
static constexpr int SPACING_BASE  = 10;

// Bigger square tile (scale=1)
static constexpr int ICON_BOX_BASE = 120;

// Smaller glyph inside square => visible padding.
// 0.50 is a good starting point; lower => more padding.
static constexpr double ICON_FRACTION = 0.42;

Glib::ustring DesktopIcon::to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

// ---- IconCanvas ----

DesktopIcon::IconCanvas::IconCanvas() {
  set_halign(Gtk::ALIGN_CENTER);
  set_valign(Gtk::ALIGN_CENTER);
  set_hexpand(false);
  set_vexpand(false);
  get_style_context()->add_class("tile-icon-box");
}

void DesktopIcon::IconCanvas::set_glyph(const Glib::ustring& g) {
  glyph_ = g;
  queue_draw();
}

void DesktopIcon::IconCanvas::set_font(const Pango::FontDescription& fd) {
  font_ = fd;
  queue_draw();
}

void DesktopIcon::IconCanvas::set_box_px(int px) {
  box_px_ = std::max(12, px);
  update_glyph_px_();
  queue_resize(); // rerun size negotiation
  queue_draw();
}

void DesktopIcon::IconCanvas::update_glyph_px_() {
  glyph_px_ = std::max(6, (int)std::lround(box_px_ * ICON_FRACTION));
}

void DesktopIcon::IconCanvas::get_preferred_width_vfunc(int& min_w, int& nat_w) const {
  min_w = box_px_;
  nat_w = box_px_;
}

void DesktopIcon::IconCanvas::get_preferred_height_vfunc(int& min_h, int& nat_h) const {
  min_h = box_px_;
  nat_h = box_px_;
}

bool DesktopIcon::IconCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
  auto sc = get_style_context();
  const int w = get_allocated_width();
  const int h = get_allocated_height();

  // Background + rounded corners from CSS (.tile-icon-box + bg-*)
  sc->render_background(cr, 0, 0, w, h);

  if (glyph_.empty()) return true;

  // IMPORTANT: use NORMAL state color (fixes “color broken” when hovered/active)
  const auto fg = sc->get_color(Gtk::STATE_FLAG_NORMAL);
  cr->set_source_rgba(fg.get_red(), fg.get_green(), fg.get_blue(), fg.get_alpha());

  auto layout = create_pango_layout(glyph_);
  Pango::FontDescription fd = font_;
  fd.set_size(glyph_px_ * Pango::SCALE);
  layout->set_font_description(fd);

  int lw = 0, lh = 0;
  layout->get_pixel_size(lw, lh);

  const double x = (w - lw) * 0.5;
  const double y = (h - lh) * 0.5;

  cr->move_to(x, y);
  pango_cairo_show_layout(cr->cobj(), layout->gobj());
  return true;
}

// ---- DesktopIcon ----

DesktopIcon::DesktopIcon(const IconSpec& spec)
: text_(spec.label),
  is_brand_(spec.isBrand)
{
  set_relief(Gtk::RELIEF_NONE);
  set_can_focus(false);
  get_style_context()->add_class("tile");

  box_.set_halign(Gtk::ALIGN_CENTER);
  box_.set_valign(Gtk::ALIGN_CENTER);

  text_.set_halign(Gtk::ALIGN_CENTER);
  text_.set_valign(Gtk::ALIGN_CENTER);
  text_.get_style_context()->add_class("tile-label");

  icon_box_.set_glyph(to_utf8(spec.codepoint));

  box_.pack_start(icon_box_, Gtk::PACK_SHRINK);
  box_.pack_start(text_, Gtk::PACK_SHRINK);
  add(box_);

  set_ui_scale(1.0, true);
  show_all_children();
}

void DesktopIcon::set_color_class(const std::string& cls) {
  if (!color_class_.empty())
    icon_box_.get_style_context()->remove_class(color_class_);
  color_class_ = cls;
  if (!color_class_.empty())
    icon_box_.get_style_context()->add_class(color_class_);
  icon_box_.queue_draw();
}

void DesktopIcon::apply_fonts(double s) {
  const int box_px = std::max(12, (int)std::lround(ICON_BOX_BASE * s));
  icon_box_.set_box_px(box_px);

  Pango::FontDescription fa;
  if (is_brand_) {
    fa.set_family(FontRegistry::kFamilyBrands);
    fa.set_weight(Pango::WEIGHT_NORMAL);
  } else {
    fa.set_family(FontRegistry::kFamilyFree);
    fa.set_weight(Pango::WEIGHT_HEAVY);
  }
  icon_box_.set_font(fa);

  const int label_px = std::max(6, (int)std::lround(LABEL_PX_BASE * s));
  Pango::FontDescription txt;
  txt.set_family("Sans");
  txt.set_size(label_px * Pango::SCALE);
  text_.override_font(txt);
}

void DesktopIcon::set_ui_scale(double s, bool show_label) {
  box_.set_spacing(std::max(1, (int)std::lround(SPACING_BASE * s)));
  text_.set_visible(show_label);
  apply_fonts(s);
}
