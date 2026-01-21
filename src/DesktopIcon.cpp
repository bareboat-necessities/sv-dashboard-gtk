#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <cmath>

static constexpr int LABEL_PX_BASE = 20;
static constexpr int SPACING_BASE  = 10;

// The ONLY thing that defines the icon background size (scale=1).
// All rounded boxes will be identical because this is fixed per-scale.
static constexpr int ICON_BOX_BASE = 88;

// Icon takes this fraction of the box (rest is “padding” inside the square).
static constexpr double ICON_FRACTION = 0.62;

Glib::ustring DesktopIcon::to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

DesktopIcon::DesktopIcon(const IconSpec& spec)
: icon_(to_utf8(spec.codepoint)),
  text_(spec.label),
  is_brand_(spec.isBrand)
{
  set_relief(Gtk::RELIEF_NONE);
  set_can_focus(false);
  get_style_context()->add_class("tile");

  box_.set_halign(Gtk::ALIGN_CENTER);
  box_.set_valign(Gtk::ALIGN_CENTER);

  icon_.set_halign(Gtk::ALIGN_CENTER);
  icon_.set_valign(Gtk::ALIGN_CENTER);
  text_.set_halign(Gtk::ALIGN_CENTER);
  text_.set_valign(Gtk::ALIGN_CENTER);

  icon_.get_style_context()->add_class("tile-icon");
  text_.get_style_context()->add_class("tile-label");

  // Square background container
  icon_box_.set_visible_window(true);
  icon_box_.set_halign(Gtk::ALIGN_CENTER);
  icon_box_.set_valign(Gtk::ALIGN_CENTER);
  icon_box_.set_hexpand(false);
  icon_box_.set_vexpand(false);
  icon_box_.get_style_context()->add_class("tile-icon-box");
  icon_box_.add(icon_);

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
}

void DesktopIcon::apply_fonts(double s) {
  // 1) Compute square box size ONLY from scale (identical for all tiles).
  const int box_px = std::max(18, (int)std::lround(ICON_BOX_BASE * s));
  icon_box_.set_size_request(box_px, box_px);

  // 2) Compute icon font size from the square (not from glyph metrics).
  const int icon_px = std::max(8, (int)std::lround(box_px * ICON_FRACTION));

  // Center the glyph; do NOT use margins that could change requested size.
  icon_.set_margin_top(0);
  icon_.set_margin_bottom(0);
  icon_.set_margin_start(0);
  icon_.set_margin_end(0);

  Pango::FontDescription fa;
  if (is_brand_) {
    fa.set_family(FontRegistry::kFamilyBrands);
    fa.set_weight(Pango::WEIGHT_NORMAL);
  } else {
    fa.set_family(FontRegistry::kFamilyFree);
    fa.set_weight(Pango::WEIGHT_HEAVY);
  }
  fa.set_size(icon_px * Pango::SCALE);
  icon_.override_font(fa);

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
