#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <glib.h>
#include <pango/pangocairo.h>   // <-- use C API instead of pangomm/cairo.h

#include <cmath>

// Text under icon
static constexpr int LABEL_PX_BASE = 20;
static constexpr int SPACING_BASE  = 10;

// Square icon background size at scale=1
static constexpr int ICON_BOX_BASE = 88;

// Icon size relative to square (smaller => more padding inside the box)
static constexpr double ICON_FRACTION = 0.58;

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
  box_px_ = std::max(10, px);
  set_size_request(box_px_, box_px_);
  update_glyph_px_();
  queue_draw();
}

void DesktopIcon::IconCanvas::update_glyph_px_() {
  glyph_px_ = std::max(6, (int)std::lround(box_px_ * ICON_FRACTION));
}

bool DesktopIcon::IconCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
  auto sc = get_style_context();

  const int w = get_allocated_width();
  const int h = get_allocated_height();

  // Draw CSS background (color + rounded corners come from CSS)
  sc->render_background(cr, 0, 0, w, h);

  if (glyph_.empty())
    return true;

  // Foreground color comes from CSS ("color:"), so Day/Dusk/Night works
  const auto fg = sc->get_color(sc->get_state());
  cr->set_source_rgba(fg.get_red(), fg.get_green(), fg.get_blue(), fg.get_alpha());

  // Create a Pango layout for the glyph
  auto layout = create_pango_layout(glyph_);
  Pango::FontDescription fd = font_;
  fd.set_size(glyph_px_ * Pango::SCALE);
  layout->set_font_description(fd);

  int lw = 0, lh = 0;
  layout->get_pixel_size(lw, lh);

  const double x = (w - lw) * 0.5;
  const double y = (h - lh) * 0.5;

  cr->move_to(x, y);

  // Render with PangoCairo C API (no pangomm/cairo.h needed)
  pango_cairo_show_layout(cr->cobj(), layout->gobj());

  return true;
}

// ---- DesktopIcon ----

DesktopIcon::DesktopIcon(const IconSpec& spec)
: icon_box_(),
  text_(spec.label),
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

  signal_clicked().connect([label = spec.label] { (void)label; });

  set_ui_scale(1.0, true);
  show_all_children();
}

void DesktopIcon::set_color_class(const std::string& cls) {
  if (!color_class_.empty()) {
    icon_box_.get_style_context()->remove_class(color_class_);
  }
  color_class_ = cls;
  if (!color_class_.empty()) {
    icon_box_.get_style_context()->add_class(color_class_);
  }
  icon_box_.queue_draw();
}

void DesktopIcon::apply_fonts(double s) {
  // EXACT square for every tile, every glyph
  const int box_px = std::max(12, (int)std::lround(ICON_BOX_BASE * s));
  icon_box_.set_box_px(box_px);

  // Font Awesome family/weight
  Pango::FontDescription fa;
  if (is_brand_) {
    fa.set_family(FontRegistry::kFamilyBrands);
    fa.set_weight(Pango::WEIGHT_NORMAL);
  } else {
    fa.set_family(FontRegistry::kFamilyFree);
    fa.set_weight(Pango::WEIGHT_HEAVY);
  }
  icon_box_.set_font(fa);

  // Label font
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
