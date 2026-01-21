#include "DesktopIcon.h"
#include "FontRegistry.h"

static constexpr int ICON_PX  = 56;
static constexpr int LABEL_PX = 20;

Glib::ustring DesktopIcon::to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

DesktopIcon::DesktopIcon(const IconSpec& spec)
: icon_(to_utf8(spec.codepoint)),
  text_(spec.label)
{
  set_relief(Gtk::RELIEF_NONE);
  set_can_focus(false); // kiosk feel
  get_style_context()->add_class("tile");

  box_.set_spacing(10);
  box_.set_halign(Gtk::ALIGN_CENTER);
  box_.set_valign(Gtk::ALIGN_CENTER);

  icon_.set_halign(Gtk::ALIGN_CENTER);
  text_.set_halign(Gtk::ALIGN_CENTER);

  icon_.get_style_context()->add_class("tile-icon");
  text_.get_style_context()->add_class("tile-label");

  // NEW: enables Day theme to draw rounded colored square behind glyph
  icon_.get_style_context()->add_class("tile-icon-box");
  if (spec.colorClass) {
    icon_.get_style_context()->add_class(spec.colorClass);
  }

  apply_fonts(spec);

  box_.pack_start(icon_, Gtk::PACK_SHRINK);
  box_.pack_start(text_, Gtk::PACK_SHRINK);
  add(box_);

  signal_clicked().connect([label = spec.label] {
    (void)label;
    // hook your launcher action here
  });

  show_all_children();
}

void DesktopIcon::apply_fonts(const IconSpec& spec) {
  Pango::FontDescription fa;
  if (spec.isBrand) {
    fa.set_family(FontRegistry::kFamilyBrands);
    fa.set_weight(Pango::WEIGHT_NORMAL);
  } else {
    fa.set_family(FontRegistry::kFamilyFree);
    fa.set_weight(Pango::WEIGHT_HEAVY);
  }
  fa.set_size(ICON_PX * Pango::SCALE);
  icon_.override_font(fa);

  Pango::FontDescription txt;
  txt.set_family("Sans");
  txt.set_size(LABEL_PX * Pango::SCALE);
  text_.override_font(txt);
}
