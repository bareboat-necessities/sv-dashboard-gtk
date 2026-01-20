#include "DesktopIcon.h"
#include "FontRegistry.h"

#include <pango/pango.h>

static constexpr int ICON_PX  = 56;
static constexpr int LABEL_PX = 20;

Glib::ustring DesktopIcon::to_utf8(char32_t cp) {
  // Convert a single Unicode codepoint to UTF-8.
  // Glib::ustring can take UTF-8 bytes.
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
  get_style_context()->add_class("tile");

  box_.set_spacing(10);
  box_.set_halign(Gtk::ALIGN_CENTER);
  box_.set_valign(Gtk::ALIGN_CENTER);

  icon_.set_halign(Gtk::ALIGN_CENTER);
  text_.set_halign(Gtk::ALIGN_CENTER);

  icon_.get_style_context()->add_class("tile-icon");
  text_.get_style_context()->add_class("tile-label");

  apply_fonts(spec);

  box_.pack_start(icon_, Gtk::PACK_SHRINK);
  box_.pack_start(text_, Gtk::PACK_SHRINK);
  add(box_);

  // Example: click handler via lambda (no-op placeholder)
  signal_clicked().connect([label = spec.label] {
    // hook your launcher action here
    (void)label;
  });
}

void DesktopIcon::apply_fonts(const IconSpec& spec) {
  // Pango font: FA6 Free (Solid weight), FA6 Brands (normal weight).
  Pango::FontDescription fa;
  if (spec.isBrand) {
    fa.set_family(FontRegistry::kFamilyBrands);
    fa.set_weight(Pango::WEIGHT_NORMAL);
  } else {
    fa.set_family(FontRegistry::kFamilyFree);
    fa.set_weight(Pango::WEIGHT_NORMAL); // 900-ish
  }
  fa.set_size(ICON_PX * Pango::SCALE);
  icon_.override_font(fa);

  Pango::FontDescription txt;
  txt.set_family("Sans");
  txt.set_size(LABEL_PX * Pango::SCALE);
  text_.override_font(txt);
}
