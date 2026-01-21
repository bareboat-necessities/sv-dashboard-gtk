#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <cmath>

static constexpr int ICON_PX_BASE       = 56;
static constexpr int LABEL_PX_BASE      = 20;
static constexpr int SPACING_BASE       = 10;

// This defines the *square* icon background size (independent of glyph width).
// Choose something that looks right for 56px icons.
static constexpr int ICON_BOX_PX_BASE   = 88;  // ~56 + margins inside the square

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

  // Label gets text color class
  icon_.get_style_context()->add_class("tile-icon");
  text_.get_style_context()->add_class("tile-label");

  // NEW: square container gets the background/color classes
  icon_box_.set_visible_window(true);     // required to draw background
  icon_box_.set_above_child(false);
  icon_box_.set_halign(Gtk::ALIGN_CENTER);
  icon_box_.set_valign(Gtk::ALIGN_CENTER);
  icon_box_.get_style_context()->add_class("tile-icon-box");
  if (spec.colorClass) {
    icon_box_.get_style_context()->add_class(spec.colorClass);
  }

  icon_box_.add(icon_);

  box_.pack_start(icon_box_, Gtk::PACK_SHRINK);
  box_.pack_start(text_, Gtk::PACK_SHRINK);
  add(box_);

  // Default scale (overridden by MainWindow)
  set_ui_scale(1.0, true);

  signal_clicked().connect([label = spec.label] {
    (void)label;
    // hook launcher action here
  });

  show_all_children();
}

void DesktopIcon::apply_fonts(double s) {
  const int icon_px  = std::max(10, (int)std::lround(ICON_PX_BASE  * s));
  const int label_px = std::max(6,  (int)std::lround(LABEL_PX_BASE * s));

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

  // Force a *square* icon background; independent of glyph width.
  // Ensure it never goes below the icon’s height (avoid clipping).
  const int box_px = std::max(
      (int)std::lround(ICON_BOX_PX_BASE * s),
      icon_px + std::max(8, (int)std::lround(18 * s))
  );
  icon_box_.set_size_request(box_px, box_px);
}

void DesktopIcon::set_ui_scale(double s, bool show_label) {
  const int sp = std::max(1, (int)std::lround(SPACING_BASE * s));
  box_.set_spacing(sp);

  text_.set_visible(show_label);

  apply_fonts(s);
}
