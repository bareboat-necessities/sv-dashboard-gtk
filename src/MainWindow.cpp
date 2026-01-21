#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>
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

  // Scale-dependent geometry
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

    // Day background colors (from your screenshots, approx)
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
    // Keep layout stable but remove colored squares
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
  // Base design size corresponds to your original layout.
  constexpr double base_w = 1400.0;
  constexpr double base_h = 800.0;

  double s = std::min(w / base_w, h / base_h);

  // Allow down to 320x200 comfortably. 320/1400=0.228, 200/800=0.25.
  // Clamp very low just to avoid zero/negative edge cases.
  s = std::max(0.18, std::min(1.0, s));

  // Hide labels when too small to be usable/readable.
  // At 320x200 this will typically hide labels (better than 5px text).
  const bool want_labels = (h >= 240) && (s >= 0.33);

  if (std::fabs(s - ui_scale_) < 0.02 && want_labels == show_labels_) return;

  ui_scale_ = s;
  show_labels_ = want_labels;

  // Scale box spacing
  root_.set_spacing(std::max(1, (int)std::lround(10 * ui_scale_)));

  // Scale nav padding via child packing
  const guint pad = (guint)std::max(0, (int)std::lround(18 * ui_scale_));
  root_.set_child_packing(btn_left_,  false, false, pad, Gtk::PACK_START);
  root_.set_child_packing(btn_right_, false, false, pad, Gtk::PACK_END);

  // Scale fonts for nav + scheme buttons (keep a sensible minimum)
  const int nav_px    = std::max(14, (int)std::lround(48 * ui_scale_));
  const int scheme_px = std::max(14, (int)std::lround(34 * ui_scale_));
  set_button_fa_font(btn_left_,  nav_px, true);
  set_button_fa_font(btn_right_, nav_px, true);
  set_button_fa_font(scheme_day_,   scheme_px, true);
  set_button_fa_font(scheme_dusk_,  scheme_px, true);
  set_button_fa_font(scheme_night_, scheme_px, true);

  // Scale scheme bar geometry
  scheme_bar_.set_spacing(std::max(1, (int)std::lround(10 * ui_scale_)));
  scheme_bar_.set_margin_start(std::max(2, (int)std::lround(22 * ui_scale_)));
  scheme_bar_.set_margin_bottom(std::max(2, (int)std::lround(18 * ui_scale_)));

  // Scale desktops
  if (page1_) page1_->set_ui_scale(ui_scale_, show_labels_);
  if (page2_) page2_->set_ui_scale(ui_scale_, show_labels_);

  // Reload CSS because Day square padding/radius is scale-dependent
  css_provider_->load_from_data(build_css(scheme_));
}

void MainWindow::on_size_allocate_custom(Gtk::Allocation& alloc) {
  apply_ui_scale(alloc.get_width(), alloc.get_height());
}

MainWindow::MainWindow() {
  set_title("BBN Launcher");
  set_default_size(1400, 800);

  apply_css_provider_once();

  // Stack transition
  stack_.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  stack_.set_transition_duration(250);

  // Pages
  page1_ = Gtk::manage(new Desktop(PAGE1));
  page2_ = Gtk::manage(new Desktop(PAGE2));
  stack_.add(*page1_, "page1");
  stack_.add(*page2_, "page2");

  // Nav buttons (FontAwesome chevrons)
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

  // Bottom-left scheme buttons (FontAwesome, bold)
  scheme_bar_.set_halign(Gtk::ALIGN_START);
  scheme_bar_.set_valign(Gtk::ALIGN_END);

  scheme_day_.set_label(cp_to_utf8(U'\uf185'));   // sun
  scheme_dusk_.set_label(cp_to_utf8(U'\uf6c4'));  // cloud-sun (if missing, swap to U'\uf0c2')
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

  // Overlay: main content + scheme bar pinned bottom-left
  overlay_.add(root_);
  overlay_.add_overlay(scheme_bar_);
  add(overlay_);

  // Input handlers
  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);

  add_events(Gdk::ALL_EVENTS_MASK);
  signal_event().connect(sigc::mem_fun(*this, &MainWindow::on_any_event), false);

  // NEW: scale on resize/allocate
  signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::on_size_allocate_custom));

  // Defaults
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

bool MainWindow::on_any_event(GdkEvent* e) {
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

  if (e->type == GDK_BUTTON_PRESS) {
    if (e->button.button != 1) return false;
    swipe_tracking_ = true;
    swipe_locked_   = false;
    swipe_start_x_  = swipe_last_x_ = xr;
    swipe_start_y_  = swipe_last_y_ = yr;
    swipe_start_t_  = t;
    return false;
  }

  if (e->type == GDK_TOUCH_BEGIN) {
    swipe_tracking_ = true;
    swipe_locked_   = false;
    swipe_start_x_  = swipe_last_x_ = xr;
    swipe_start_y_  = swipe_last_y_ = yr;
    swipe_start_t_  = t;
    return false;
  }

  if ((e->type == GDK_MOTION_NOTIFY || e->type == GDK_TOUCH_UPDATE) && swipe_tracking_) {
    swipe_last_x_ = xr;
    swipe_last_y_ = yr;

    const double dx = swipe_last_x_ - swipe_start_x_;
    const double dy = swipe_last_y_ - swipe_start_y_;

    if (!swipe_locked_) {
      if (std::fabs(dx) >= kSwipeLockPx && std::fabs(dx) > std::fabs(dy) * 1.1) {
        swipe_locked_ = true;
      }
    }
    return swipe_locked_;
  }

  if ((e->type == GDK_BUTTON_RELEASE || e->type == GDK_TOUCH_END || e->type == GDK_TOUCH_CANCEL) && swipe_tracking_) {
    swipe_tracking_ = false;

    const double dx = xr - swipe_start_x_;
    const double dy = yr - swipe_start_y_;
    const guint32 dt_ms = (t >= swipe_start_t_) ? (t - swipe_start_t_) : 0;

    handle_swipe_delta(dx, dy, dt_ms);

    const bool consumed = swipe_locked_;
    swipe_locked_ = false;
    return consumed;
  }

  return false;
}
