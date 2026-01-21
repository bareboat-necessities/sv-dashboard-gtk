#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>   // for gtk_gesture_set_state (claim gesture)
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

  const int icon_pad     = std::max(1, (int)std::lround(14 * ui_scale_));
  const int icon_radius  = std::max(3, (int)std::lround(16 * ui_scale_));

  std::string css;
  css += "window, GtkWindow { background: #000000; }\n";
  css += ".tile { background: transparent; border: none; box-shadow: none; padding: 0; }\n";
  css += ".tile-icon, .tile-label, .nav { }\n";
  css += ".tile-icon-box { }\n";

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
    css += ".tile-icon-box { color:#ffffff; border-radius:" + itos(icon_radius) + "px; ";
    css += "padding:" + itos(icon_pad) + "px; background:#2b2b2b; }\n";

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
    css += ".tile-icon-box { background:transparent; padding:" + itos(icon_pad) + "px; ";
    css += "border-radius:" + itos(icon_radius) + "px; }\n";
  } else {
    css += ".tile-icon, .tile-label, .nav { color:#d00000; }\n";
    css += ".tile-icon-box { background:transparent; padding:" + itos(icon_pad) + "px; ";
    css += "border-radius:" + itos(icon_radius) + "px; }\n";
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
  s = std::max(0.18, std::min(1.0, s));

  const bool want_labels = (h >= 240) && (s >= 0.33);

  if (std::fabs(s - ui_scale_) < 0.02 && want_labels == show_labels_) return;

  ui_scale_ = s;
  show_labels_ = want_labels;

  root_.set_spacing(std::max(1, (int)std::lround(10 * ui_scale_)));

  const guint pad = (guint)std::max(0, (int)std::lround(18 * ui_scale_));
  root_.set_child_packing(btn_left_,  false, false, pad, Gtk::PACK_START);
  root_.set_child_packing(btn_right_, false, false, pad, Gtk::PACK_END);

  const int nav_px    = std::max(14, (int)std::lround(48 * ui_scale_));
  const int scheme_px = std::max(14, (int)std::lround(34 * ui_scale_));
  set_button_fa_font(btn_left_,  nav_px, true);
  set_button_fa_font(btn_right_, nav_px, true);
  set_button_fa_font(scheme_day_,   scheme_px, true);
  set_button_fa_font(scheme_dusk_,  scheme_px, true);
  set_button_fa_font(scheme_night_, scheme_px, true);

  scheme_bar_.set_spacing(std::max(1, (int)std::lround(10 * ui_scale_)));
  scheme_bar_.set_margin_start(std::max(2, (int)std::lround(22 * ui_scale_)));
  scheme_bar_.set_margin_bottom(std::max(2, (int)std::lround(18 * ui_scale_)));

  if (page1_) page1_->set_ui_scale(ui_scale_, show_labels_);
  if (page2_) page2_->set_ui_scale(ui_scale_, show_labels_);

  css_provider_->load_from_data(build_css(scheme_));
}

void MainWindow::on_size_allocate_custom(Gtk::Allocation& alloc) {
  apply_ui_scale(alloc.get_width(), alloc.get_height());
}

void MainWindow::handle_swipe_delta(double dx, double dy, guint32 dt_ms) {
  // must be mostly horizontal
  if (std::fabs(dx) < kSwipeMinPx) {
    if (!(std::fabs(dx) >= kSwipeFastMinPx && dt_ms <= kSwipeFastMaxMs)) return;
  }
  if (std::fabs(dx) < std::fabs(dy) * 1.2) return;

  const auto name = stack_.get_visible_child_name();
  if (dx < 0) {
    if (name == "page1") show_page("page2");
  } else {
    if (name == "page2") show_page("page1");
  }
}

void MainWindow::setup_gestures() {
  // Attach to stack_ so it sees events starting on any descendant (icons, grid, etc.)
  drag_ = Gtk::GestureDrag::create(stack_);
  drag_->set_touch_only(false); // enable mouse drag as well

  drag_->signal_drag_begin().connect([this](double /*x*/, double /*y*/) {
    drag_claimed_ = false;
    drag_t0_us_ = g_get_monotonic_time();
  });

  drag_->signal_drag_update().connect([this](double dx, double dy) {
    // Claim the gesture once we are confidently doing a horizontal drag.
    if (!drag_claimed_) {
      if (std::fabs(dx) >= kSwipeLockPx && std::fabs(dx) > std::fabs(dy) * 1.1) {
        drag_claimed_ = true;
        gtk_gesture_set_state(GTK_GESTURE(drag_->gobj()), GTK_EVENT_SEQUENCE_CLAIMED);
      }
    }
  });

  drag_->signal_drag_end().connect([this](double dx, double dy) {
    const gint64 t1_us = g_get_monotonic_time();
    const guint32 dt_ms = (t1_us > drag_t0_us_) ? (guint32)((t1_us - drag_t0_us_) / 1000) : 0;
    handle_swipe_delta(dx, dy, dt_ms);
    drag_claimed_ = false;
  });
}

MainWindow::MainWindow() {
  set_title("BBN Launcher");
  set_default_size(1400, 800);

  apply_css_provider_once();

  stack_.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  stack_.set_transition_duration(250);

  page1_ = Gtk::manage(new Desktop(PAGE1));
  page2_ = Gtk::manage(new Desktop(PAGE2));
  stack_.add(*page1_, "page1");
  stack_.add(*page2_, "page2");

  // Nav buttons
  btn_left_.set_label(cp_to_utf8(CHEV_LEFT));
  btn_right_.set_label(cp_to_utf8(CHEV_RIGHT));
  btn_left_.set_relief(Gtk::RELIEF_NONE);
  btn_right_.set_relief(Gtk::RELIEF_NONE);
  btn_left_.set_can_focus(false);
  btn_right_.set_can_focus(false);
  btn_left_.get_style_context()->add_class("nav");
  btn_right_.get_style_context()->add_class("nav");

  btn_left_.signal_clicked().connect([this] { show_page("page1"); });
  btn_right_.signal_clicked().connect([this] { show_page("page2"); });

  root_.pack_start(btn_left_, Gtk::PACK_SHRINK, 0);
  root_.pack_start(stack_, Gtk::PACK_EXPAND_WIDGET);
  root_.pack_start(btn_right_, Gtk::PACK_SHRINK, 0);

  // Theme buttons
  scheme_bar_.set_halign(Gtk::ALIGN_START);
  scheme_bar_.set_valign(Gtk::ALIGN_END);

  scheme_day_.set_label(cp_to_utf8(U'\uf185'));   // sun
  scheme_dusk_.set_label(cp_to_utf8(U'\uf6c4'));  // cloud-sun (if missing, use U'\uf0c2')
  scheme_night_.set_label(cp_to_utf8(U'\uf186')); // moon

  for (Gtk::Button* b : { &scheme_day_, &scheme_dusk_, &scheme_night_ }) {
    b->set_relief(Gtk::RELIEF_NONE);
    b->set_can_focus(false);
    b->get_style_context()->add_class("scheme-btn");
  }
  scheme_day_.get_style_context()->add_class("scheme-day");
  scheme_dusk_.get_style_context()->add_class("scheme-dusk");
  scheme_night_.get_style_context()->add_class("scheme-night");

  scheme_day_.signal_clicked().connect([this]{ set_scheme(Scheme::Day); });
  scheme_dusk_.signal_clicked().connect([this]{ set_scheme(Scheme::Dusk); });
  scheme_night_.signal_clicked().connect([this]{ set_scheme(Scheme::Night); });

  scheme_bar_.pack_start(scheme_day_, Gtk::PACK_SHRINK);
  scheme_bar_.pack_start(scheme_dusk_, Gtk::PACK_SHRINK);
  scheme_bar_.pack_start(scheme_night_, Gtk::PACK_SHRINK);

  overlay_.add(root_);
  overlay_.add_overlay(scheme_bar_);
  add(overlay_);

  // Keys + scaling
  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);
  signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::on_size_allocate_custom));

  // NEW: robust swipe/drag
  setup_gestures();

  set_scheme(Scheme::Day);

  show_all();
  show_page("page1");
}

void MainWindow::show_page(const Glib::ustring& name) {
  stack_.set_visible_child(name);
  refresh_nav();
}

void MainWindow::refresh_nav() {
  const auto name = stack_.get_visible_child_name();
  btn_left_.set_sensitive(name != "page1");
  btn_right_.set_sensitive(name != "page2");
}

bool MainWindow::on_key_press(GdkEventKey* e) {
  switch (e->keyval) {
    case GDK_KEY_Right:
    case GDK_KEY_Page_Down:
      show_page("page2");
      return true;
    case GDK_KEY_Left:
    case GDK_KEY_Page_Up:
      show_page("page1");
      return true;

    case GDK_KEY_1: set_scheme(Scheme::Day);  return true;
    case GDK_KEY_2: set_scheme(Scheme::Dusk); return true;
    case GDK_KEY_3: set_scheme(Scheme::Night);return true;

    default:
      return false;
  }
}
