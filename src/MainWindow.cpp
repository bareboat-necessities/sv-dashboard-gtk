#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>
#include <cmath>

static Glib::ustring cp_to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

static void set_nav_button_font(Gtk::Button& b) {
  b.set_relief(Gtk::RELIEF_NONE);
  b.set_can_focus(false);
  b.get_style_context()->add_class("nav");

  Pango::FontDescription fd;
  fd.set_family(FontRegistry::kFamilyFree);
  fd.set_weight(Pango::WEIGHT_HEAVY);
  fd.set_size(48 * Pango::SCALE);

  if (auto* child = dynamic_cast<Gtk::Label*>(b.get_child())) {
    child->override_font(fd);
  }
}

// Bigger/bolder scheme buttons
static void set_scheme_button_font(Gtk::Button& b) {
  b.set_relief(Gtk::RELIEF_NONE);
  b.set_can_focus(false);
  b.get_style_context()->add_class("scheme-btn");

  Pango::FontDescription fd;
  fd.set_family(FontRegistry::kFamilyFree);
  fd.set_weight(Pango::WEIGHT_HEAVY);
  fd.set_size(34 * Pango::SCALE); // bump to 38 if you want even bigger

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
  Glib::ustring css = R"css(
window, GtkWindow { background: #000000; }

.tile { background: transparent; border: none; box-shadow: none; padding: 0; }
.tile-icon, .tile-label, .nav { }

.tile-icon-box { }

.scheme-btn {
  background: transparent;
  border: none;
  box-shadow: none;
  padding: 6px 14px;   /* bigger hit target */
  margin-right: 6px;
}
.scheme-btn:focus { outline: none; }

.scheme-day   { color: #d4b000; }  /* gold */
.scheme-dusk  { color: #b0b0b0; }  /* gray */
.scheme-night { color: #d00000; }  /* red */

.scheme-btn { opacity: 0.65; }
.scheme-btn.active { opacity: 1.0; border-bottom: 2px solid currentColor; }
)css";

  if (s == Scheme::Day) {
    css += R"css(
.tile-icon, .tile-label, .nav { color: #f2f2f2; }

.tile-icon-box {
  color: #ffffff;
  border-radius: 16px;
  padding: 14px;
  background: #2b2b2b;
}

/* Day background colors (from your screenshots, approx) */
.tile-icon-box.bg-azure      { background: #007ACC; }
.tile-icon-box.bg-blue       { background: #1976D2; }
.tile-icon-box.bg-teal       { background: #009688; }
.tile-icon-box.bg-teal-light { background: #26A69A; }
.tile-icon-box.bg-cyan       { background: #06B6D4; }
.tile-icon-box.bg-indigo     { background: #5C6BC0; }
.tile-icon-box.bg-gray       { background: #455A64; }
.tile-icon-box.bg-slate      { background: #556F7B; }
.tile-icon-box.bg-slate-dark { background: #546E7A; }
.tile-icon-box.bg-purple     { background: #8E24AA; }
.tile-icon-box.bg-violet     { background: #7E22CE; }
.tile-icon-box.bg-red        { background: #DC2626; }
)css";
  } else if (s == Scheme::Dusk) {
    css += R"css(
.tile-icon { color: #e6e6e6; }
.tile-label, .nav { color: #c8c8c8; }

.tile-icon-box {
  background: transparent;
  padding: 14px;
  border-radius: 16px;
}
)css";
  } else {
    css += R"css(
.tile-icon, .tile-label, .nav { color: #d00000; }

.tile-icon-box {
  background: transparent;
  padding: 14px;
  border-radius: 16px;
}
)css";
  }

  return css;
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

MainWindow::MainWindow() {
  set_title("BBN Launcher");
  set_default_size(1400, 800);

  apply_css_provider_once();

  // Stack transition
  stack_.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  stack_.set_transition_duration(250);

  // Pages
  auto* p1 = Gtk::manage(new Desktop(PAGE1));
  auto* p2 = Gtk::manage(new Desktop(PAGE2));
  stack_.add(*p1, "page1");
  stack_.add(*p2, "page2");

  // Nav buttons
  btn_left_.set_label(cp_to_utf8(CHEV_LEFT));
  btn_right_.set_label(cp_to_utf8(CHEV_RIGHT));
  set_nav_button_font(btn_left_);
  set_nav_button_font(btn_right_);

  btn_left_.signal_clicked().connect([this] { show_page("page1"); });
  btn_right_.signal_clicked().connect([this] { show_page("page2"); });

  root_.set_spacing(10);
  root_.pack_start(btn_left_, Gtk::PACK_SHRINK, 18);
  root_.pack_start(stack_, Gtk::PACK_EXPAND_WIDGET);
  root_.pack_start(btn_right_, Gtk::PACK_SHRINK, 18);

  // Bottom-left scheme buttons (FontAwesome, bold)
  scheme_bar_.set_spacing(10);
  scheme_bar_.set_halign(Gtk::ALIGN_START);
  scheme_bar_.set_valign(Gtk::ALIGN_END);
  scheme_bar_.set_margin_start(22);
  scheme_bar_.set_margin_bottom(18);

  // fa-sun / fa-cloud-sun / fa-moon (solid)
  scheme_day_.set_label(cp_to_utf8(U'\uf185'));   // sun
  scheme_dusk_.set_label(cp_to_utf8(U'\uf6c4'));  // cloud-sun (if missing in your FA, swap to U'\uf0c2' (cloud))
  scheme_night_.set_label(cp_to_utf8(U'\uf186')); // moon

  set_scheme_button_font(scheme_day_);
  set_scheme_button_font(scheme_dusk_);
  set_scheme_button_font(scheme_night_);

  scheme_day_.get_style_context()->add_class("scheme-day");
  scheme_dusk_.get_style_context()->add_class("scheme-dusk");
  scheme_night_.get_style_context()->add_class("scheme-night");

  scheme_day_.signal_clicked().connect([this]{ set_scheme(Scheme::Day); });
  scheme_dusk_.signal_clicked().connect([this]{ set_scheme(Scheme::Dusk); });
  scheme_night_.signal_clicked().connect([this]{ set_scheme(Scheme::Night); });

  scheme_bar_.pack_start(scheme_day_, Gtk::PACK_SHRINK);
  scheme_bar_.pack_start(scheme_dusk_, Gtk::PACK_SHRINK);
  scheme_bar_.pack_start(scheme_night_, Gtk::PACK_SHRINK);

  // Overlay: main content + scheme bar pinned to bottom-left
  overlay_.add(root_);
  overlay_.add_overlay(scheme_bar_);
  add(overlay_);

  // Keys
  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);

  // NEW: Enable and listen for generic events so we can detect swipe/drag
  // ALL_EVENTS_MASK is the safest across gdkmm versions.
  add_events(Gdk::ALL_EVENTS_MASK);
  signal_event().connect(sigc::mem_fun(*this, &MainWindow::on_any_event), false);

  // Default scheme + show
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

    case GDK_KEY_1:
      set_scheme(Scheme::Day);
      return true;
    case GDK_KEY_2:
      set_scheme(Scheme::Dusk);
      return true;
    case GDK_KEY_3:
      set_scheme(Scheme::Night);
      return true;

    default:
      return false;
  }
}

void MainWindow::handle_swipe_delta(double dx, double dy, guint32 dt_ms) {
  // Require mostly-horizontal movement to avoid accidental page switches
  if (std::fabs(dx) < kSwipeMinPx) {
    // allow fast short swipe
    if (!(std::fabs(dx) >= kSwipeFastMinPx && dt_ms <= kSwipeFastMaxMs)) return;
  }
  if (std::fabs(dx) < std::fabs(dy) * 1.2) return;

  const auto name = stack_.get_visible_child_name();

  if (dx < 0) {
    // swipe left -> go to page2
    if (name == "page1") show_page("page2");
  } else {
    // swipe right -> go to page1
    if (name == "page2") show_page("page1");
  }
}

bool MainWindow::on_any_event(GdkEvent* e) {
  // Use root coords so it works regardless of which child widget got the event
  auto get_xy_root_time = [&](double& xr, double& yr, guint32& t) -> bool {
    switch (e->type) {
      case GDK_BUTTON_PRESS:
      case GDK_BUTTON_RELEASE:
        xr = e->button.x_root; yr = e->button.y_root; t = e->button.time; return true;
      case GDK_MOTION_NOTIFY:
        xr = e->motion.x_root; yr = e->motion.y_root; t = e->motion.time; return true;
      case GDK_TOUCH_BEGIN:
      case GDK_TOUCH_UPDATE:
      case GDK_TOUCH_END:
      case GDK_TOUCH_CANCEL:
        xr = e->touch.x_root; yr = e->touch.y_root; t = e->touch.time; return true;
      default:
        return false;
    }
  };

  double xr = 0.0, yr = 0.0;
  guint32 t = 0;
  if (!get_xy_root_time(xr, yr, t)) return false;

  // Start tracking on mouse press or touch begin
  if (e->type == GDK_BUTTON_PRESS) {
    if (e->button.button != 1) return false; // left mouse only
    swipe_tracking_ = true;
    swipe_locked_   = false;
    swipe_start_x_  = swipe_last_x_ = xr;
    swipe_start_y_  = swipe_last_y_ = yr;
    swipe_start_t_  = t;
    return false; // don’t block normal clicks yet
  }

  if (e->type == GDK_TOUCH_BEGIN) {
    swipe_tracking_ = true;
    swipe_locked_   = false;
    swipe_start_x_  = swipe_last_x_ = xr;
    swipe_start_y_  = swipe_last_y_ = yr;
    swipe_start_t_  = t;
    return false;
  }

  // Track motion
  if ((e->type == GDK_MOTION_NOTIFY || e->type == GDK_TOUCH_UPDATE) && swipe_tracking_) {
    swipe_last_x_ = xr;
    swipe_last_y_ = yr;

    const double dx = swipe_last_x_ - swipe_start_x_;
    const double dy = swipe_last_y_ - swipe_start_y_;

    // Once movement is clearly horizontal, "lock" swipe so icons are less likely to click
    if (!swipe_locked_) {
      if (std::fabs(dx) >= kSwipeLockPx && std::fabs(dx) > std::fabs(dy) * 1.1) {
        swipe_locked_ = true;
      }
    }

    // If locked, consume motion to reduce accidental child interactions
    return swipe_locked_;
  }

  // End gesture
  if ((e->type == GDK_BUTTON_RELEASE || e->type == GDK_TOUCH_END || e->type == GDK_TOUCH_CANCEL) && swipe_tracking_) {
    swipe_tracking_ = false;

    const double dx = xr - swipe_start_x_;
    const double dy = yr - swipe_start_y_;
    const guint32 dt_ms = (t >= swipe_start_t_) ? (t - swipe_start_t_) : 0;

    handle_swipe_delta(dx, dy, dt_ms);

    // If we locked swipe, consume the release to reduce accidental button activation
    const bool consumed = swipe_locked_;
    swipe_locked_ = false;
    return consumed;
  }

  return false;
}
