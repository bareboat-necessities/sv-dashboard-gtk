#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>

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

  // Button child is a Gtk::Label in gtkmm when using set_label()
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
  // Shared base
  Glib::ustring css = R"css(
window, GtkWindow { background: #000000; }

.tile { background: transparent; border: none; box-shadow: none; padding: 0; }
.tile-icon, .tile-label, .nav { }

.tile-icon-box { } /* scheme will define bg/padding */

.scheme-btn {
  background: transparent;
  border: none;
  box-shadow: none;
  padding: 0 8px;
}
.scheme-btn label { font-size: 18px; }
.scheme-btn:focus { outline: none; }

.scheme-day   { color: #d4b000; }  /* gold */
.scheme-dusk  { color: #b0b0b0; }  /* gray */
.scheme-night { color: #d00000; }  /* red */

.scheme-btn { opacity: 0.65; }
.scheme-btn.active { opacity: 1.0; }
)css";

  if (s == Scheme::Day) {
    // Day theme: colored squares + white glyph + white text
    css += R"css(
.tile-icon, .tile-label, .nav { color: #f2f2f2; }

.tile-icon-box {
  color: #ffffff;
  border-radius: 16px;
  padding: 14px;
  background: #2b2b2b; /* fallback */
}

/* Day background colors (from your screenshots, approx) */
.tile-icon-box.bg-azure      { background: #007ACC; } /* Freeboard/AvNav */
.tile-icon-box.bg-blue       { background: #1976D2; } /* many blue tiles */
.tile-icon-box.bg-teal       { background: #009688; } /* KIP/Instruments/SignalK */
.tile-icon-box.bg-teal-light { background: #26A69A; } /* Power */
.tile-icon-box.bg-cyan       { background: #06B6D4; } /* PyPilot */
.tile-icon-box.bg-indigo     { background: #5C6BC0; } /* Sky/Windy/Drones */
.tile-icon-box.bg-gray       { background: #455A64; } /* Camera/Web Cam */
.tile-icon-box.bg-slate      { background: #556F7B; } /* Provisioning/Files */
.tile-icon-box.bg-slate-dark { background: #546E7A; } /* Terminal/Tasks/Commands */
.tile-icon-box.bg-purple     { background: #8E24AA; } /* Radio */
.tile-icon-box.bg-violet     { background: #7E22CE; } /* T-Storms */
.tile-icon-box.bg-red        { background: #DC2626; } /* Music/Video */
)css";
  } else if (s == Scheme::Dusk) {
    // Dusk: monochrome white/gray on dark background
    css += R"css(
.tile-icon { color: #e6e6e6; }
.tile-label, .nav { color: #c8c8c8; }

/* keep layout stable but no colored squares */
.tile-icon-box {
  background: transparent;
  padding: 14px;
  border-radius: 16px;
}
)css";
  } else {
    // Night: red monochrome on dark background
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

  // Bottom-left scheme buttons
  scheme_bar_.set_spacing(10);
  scheme_bar_.set_halign(Gtk::ALIGN_START);
  scheme_bar_.set_valign(Gtk::ALIGN_END);
  scheme_bar_.set_margin_start(22);
  scheme_bar_.set_margin_bottom(18);

  scheme_day_.set_label("☀");
  scheme_dusk_.set_label("☼");
  scheme_night_.set_label("☾");

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

  // Overlay: main content + scheme bar pinned to bottom-left
  overlay_.add(root_);
  overlay_.add_overlay(scheme_bar_);
  add(overlay_);

  // Key handling
  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);

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

    // Optional scheme hotkeys
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
