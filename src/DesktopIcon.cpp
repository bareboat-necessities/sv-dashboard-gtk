#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <cmath>

static constexpr int ICON_PX_BASE   = 56;
static constexpr int LABEL_PX_BASE  = 20;
static constexpr int SPACING_BASE   = 10;

static constexpr int ICON_BOX_BASE  = 88;  // square at scale=1
static constexpr int ICON_PAD_BASE  = 14;  // inner margins around glyph

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

  icon_box_.set_visible_window(true);
  icon_box_.set_halign(Gtk::ALIGN_CENTER);
  icon_box_.set_valign(Gtk::ALIGN_CENTER);
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
  const int icon_px  = std::max(8, (int)std::lround(ICON_PX_BASE  * s));
  const int label_px = std::max(6, (int)std::lround(LABEL_PX_BASE * s));

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

  Pango::FontDescription txt;
  txt.set_family("Sans");
  txt.set_size(label_px * Pango::SCALE);
  text_.override_font(txt);

  // Inner margins -> padding without CSS sizing surprises
  const int pad = std::max(1, (int)std::lround(ICON_PAD_BASE * s));
  icon_.set_margin_top(pad);
  icon_.set_margin_bottom(pad);
  icon_.set_margin_start(pad);
  icon_.set_margin_end(pad);

  const int box_px = std::max(icon_px + 2 * pad, (int)std::lround(ICON_BOX_BASE * s));
  icon_box_.set_size_request(box_px, box_px);
}

void DesktopIcon::set_ui_scale(double s, bool show_label) {
  box_.set_spacing(std::max(1, (int)std::lround(SPACING_BASE * s)));
  text_.set_visible(show_label);
  apply_fonts(s);
}
