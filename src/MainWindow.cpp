#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <cmath>
#include <string>

static Glib::ustring cp_to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

static void set_button_fa_font(Gtk::Button& b, int px, bool heavy = true) {
  Pango::FontDescription fd;
  fd.set_family(FontRegistry::kFamilyFree);
  fd.set_weight(heavy ? Pango::WEIGHT_HEAVY : Pango::WEIGHT_NORMAL);
  fd.set_size(px * Pango::SCALE);

  if (auto* child = dynamic_cast<Gtk::Label*>(b.get_child())) {
    child->override_font(fd);
  }
}

void MainWindow::apply_css_provider_once() {
  css_provider_ = Gtk::CssProvider::create();
  auto screen = Gdk::Screen::get_default();
  Gtk::StyleContext::add_provider_for_screen(
      screen, css_provider_, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

Glib::ustring MainWindow::build_css(Scheme s) const {
  auto itos = [](int v) { return std::to_string(v); };

  const int scheme_pad_v = std::max(1, (int)std::lround(6  * ui_scale_));
  const int scheme_pad_h = std::max(2, (int)std::lround(14 * ui_scale_));
  const int scheme_mr    = std::max(0, (int)std::lround(6  * ui_scale_));

  // Icon square corner radius scales
  const int icon_radius  = std::max(3, (int)std::lround(16 * ui_scale_));

  std::string css;
  css += "window, GtkWindow { background: #000000; }\n";
  css += ".tile { background: transparent; border: none; box-shadow: none; padding: 0; }\n";
  css += ".tile-icon, .tile-label, .nav { }\n";

  css += ".scheme-btn { background: transparent; border: none; box-shadow: none; ";
  css += "padding: " + itos(scheme_pad_v) + "px " + itos(scheme_pad_h) + "px; ";
  css += "margin-right: " + itos(scheme_mr) + "px; }\n";
  css += ".scheme-btn:focus { outline: none; }\n";

  css += ".scheme-day   { color: #d4b000; }\n";
  css += ".scheme-dusk  { color: #b0b0b0; }\n";
  css += ".scheme-night { color: #d00000; }\n";
  css += ".scheme-btn { opacity: 0.65; }\n";
  css += ".scheme-btn.active { opacity: 1.0; border-bottom: 2px solid currentColor; }\n";

  if (s == Scheme::Day) {
    css += ".tile-icon, .tile-label, .nav { color: #f2f2f2; }\n";

    // IMPORTANT: no padding here; size is enforced in C++ (square).
    css += ".tile-icon-box { color:#ffffff; border-radius:" + itos(icon_radius) + "px; ";
    css += "background:#2b2b2b; }\n";

    css += ".tile-icon-box.bg-azure      { background:#007ACC; }\n";
    css += ".tile-icon-box.bg-blue       { background:#1976D2; }\n";
    css += ".tile-icon-box.bg-teal       { background:#009688; }\n";
    css += ".tile-icon-box.bg-teal-light { background:#26A69A; }\n";
    css += ".tile-icon-box.bg-cyan       { background:#06B6D4; }\n";
    css += ".tile-icon-box.bg-indigo     { background:#5C6BC0; }\n";
    css += ".tile-icon-box.bg-gray       { background:#455A64; }\n";
    css += ".tile-icon-box.bg-slate      { background:#556F7B; }\n";
    css += ".tile-icon-box.bg-slate-dark { background:#546E7A; }\n";
    css += ".tile-icon-box.bg-purple     { background:#8E24AA; }\n";
    css += ".tile-icon-box.bg-violet     { background:#7E22CE; }\n";
    css += ".tile-icon-box.bg-red        { background:#DC2626; }\n";
  } else if (s == Scheme::Dusk) {
    css += ".tile-icon { color:#e6e6e6; }\n";
    css += ".tile-label, .nav { color:#c8c8c8; }\n";
    css += ".tile-icon-box { background:transparent; border-radius:" + itos(icon_radius) + "px; }\n";
  } else {
    css += ".tile-icon, .tile-label, .nav { color:#d00000; }\n";
    css += ".tile-icon-box { background:transparent; border-radius:" + itos(icon_radius) + "px; }\n";
  }

  return Glib::ustring(css);
}

void MainWindow::refresh_scheme_buttons() {
  auto set_active = [](Gtk::Button& b, bool on) {
    auto sc = b.get_style_context();
    if (on) sc->add_class("active");
    else    sc->remove_class("active");
  };
  set_active(scheme_day_,   scheme_ == Scheme::Day);
  set_active(scheme_dusk_,  scheme_ == Scheme::Dusk);
  set_active(scheme_night_, scheme_ == Scheme::Night);
}

void MainWindow::set_scheme(Scheme s) {
  scheme_ = s;
  css_provider_->load_from_data(build_css(scheme_));
  refresh_scheme_buttons();
}

void MainWindow::apply_ui_scale(int w, int h) {
  constexpr double base_w = 1400.0;
  constexpr double base_h = 800.0;

  double s = std::min(w / base_w, h / base_h);
  s = std::max(0.12, std::min(1.0, s));

  const bool want_labels = (h >= 260) && (s >= 0.33);

  if (std::fabs(s - ui_scale_) < 0.02 && want_labels == show_labels_) return;

  ui_scale_ = s;
  show_labels_ = want_labels;

  root_.set_spacing(std::max(0, (int)std::lround(8 * ui_scale_)));

  const bool tiny = (w <= 420);
  btn_left_.set_visible(!tiny);
  btn_right_.set_visible(!tiny);

  const int pad = std::max(0, (int)std::lround(14 * ui_scale_));
  btn_left_.set_margin_start(pad);
  btn_left_.set_margin_end(pad);
  btn_right_.set_margin_start(pad);
  btn_right_.set_margin_end(pad);

  const int nav_px = std::max(12, (int)std::lround(48 * ui_scale_));

  // IMPORTANT: allow them to shrink (previously clamped too high).
  const int scheme_px = std::max(10, (int)std::lround(36 * ui_scale_));

  set_button_fa_font(btn_left_,  nav_px, true);
  set_button_fa_font(btn_right_, nav_px, true);

  set_button_fa_font(scheme_day_,   scheme_px, true);
  set_button_fa_font(scheme_dusk_,  scheme_px, true);
  set_button_fa_font(scheme_night_, scheme_px, true);

  scheme_bar_.set_spacing(std::max(0, (int)std::lround(8 * ui_scale_)));
  scheme_bar_.set_margin_start(std::max(2, (int)std::lround(14 * ui_scale_)));
  scheme_bar_.set_margin_bottom(std::max(2, (int)std::lround(12 * ui_scale_)));

  if (page1_) page1_->set_ui_scale(ui_scale_, show_labels_);
  if (page2_) page2_->set_ui_scale(ui_scale_, show_labels_);

  css_provider_->load_from_data(build_css(scheme_));

  queue_resize();
}

// --- the rest of your MainWindow.cpp remains unchanged ---
// (gestures, constructor wiring, show_page, refresh_nav, on_key_press, etc.)
