#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>   // gtk_gesture_set_state
#include <cmath>
#include <string>
#include <vector>

static Glib::ustring cp_to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

// GTK3 often wraps button labels (Alignment -> Label). This finds the label reliably.
static Gtk::Label* find_label(Gtk::Widget* w) {
  if (!w) return nullptr;
  if (auto* l = dynamic_cast<Gtk::Label*>(w)) return l;

  if (auto* c = dynamic_cast<Gtk::Container*>(w)) {
    auto kids = c->get_children();
    for (auto* k : kids) {
      if (auto* r = find_label(k)) return r;
    }
  }
  return nullptr;
}

static void set_button_fa_font(Gtk::Button& b, int px, bool heavy = true) {
  Pango::FontDescription fd;
  fd.set_family(FontRegistry::kFamilyFree);
  fd.set_weight(heavy ? Pango::WEIGHT_HEAVY : Pango::WEIGHT_NORMAL);
  fd.set_size(px * Pango::SCALE);

  if (auto* child = find_label(b.get_child())) {
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

  const int icon_radius  = std::max(3, (int)std::lround(16 * ui_scale_));

  std::string css;
  css += "window, GtkWindow { background: #000000; }\n";
  css += ".tile { background: transparent; border: none; box-shadow: none; padding: 0; }\n";

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
    css += ".tile-label, .nav { color: #f2f2f2; }\n";

    // Icon color + background are on tile-icon-box now
    css += ".tile-icon-box { background:#2b2b2b; color:#ffffff; border-radius:" + itos(icon_radius) + "px; }\n";

    for (const auto& entry : palette_) {
      css += ".tile-icon-box." + entry.first + " { background:" + entry.second + "; }\n";
    }

  } else if (s == Scheme::Dusk) {
    css += ".tile-label, .nav { color:#c8c8c8; }\n";
    // FIX: icon glyph is drawn inside tile-icon-box
    css += ".tile-icon-box { color:#e6e6e6; background:transparent; border-radius:" + itos(icon_radius) + "px; }\n";

  } else {
    css += ".tile-label, .nav { color:#d00000; }\n";
    // FIX: icon glyph is drawn inside tile-icon-box
    css += ".tile-icon-box { color:#d00000; background:transparent; border-radius:" + itos(icon_radius) + "px; }\n";
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
  s = std::max(0.06, std::min(1.0, s));

  const bool want_labels = (h >= 260) && (s >= 0.33);

  if (std::fabs(s - ui_scale_) < 0.005 && want_labels == show_labels_) return;

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

  const int nav_px    = std::max(10, (int)std::lround(48 * ui_scale_));
  const int scheme_px = std::max(6,  (int)std::lround(34 * ui_scale_));

  set_button_fa_font(btn_left_,  nav_px, true);
  set_button_fa_font(btn_right_, nav_px, true);
  set_button_fa_font(scheme_day_,   scheme_px, true);
  set_button_fa_font(scheme_dusk_,  scheme_px, true);
  set_button_fa_font(scheme_night_, scheme_px, true);

  scheme_bar_.set_spacing(std::max(0, (int)std::lround(8 * ui_scale_)));
  scheme_bar_.set_margin_start(std::max(2, (int)std::lround(14 * ui_scale_)));
  scheme_bar_.set_margin_bottom(std::max(2, (int)std::lround(12 * ui_scale_)));

  for (auto* page : pages_) {
    if (page) {
      page->set_ui_scale(ui_scale_, show_labels_);
    }
  }

  css_provider_->load_from_data(build_css(scheme_));

  queue_resize();
}

void MainWindow::on_overlay_size_allocate(Gtk::Allocation& alloc) {
  apply_ui_scale(alloc.get_width(), alloc.get_height());
}

void MainWindow::handle_swipe_delta(double dx, double dy, guint32 dt_ms) {
  if (std::fabs(dx) < kSwipeMinPx) {
    if (!(std::fabs(dx) >= kSwipeFastMinPx && dt_ms <= kSwipeFastMaxMs)) return;
  }
  if (std::fabs(dx) < std::fabs(dy) * 1.2) return;

  if (pages_.size() < 2) return;
  if (dx < 0) {
    if (current_page_index_ + 1 < pages_.size()) {
      show_page_index(current_page_index_ + 1);
    }
  } else {
    if (current_page_index_ > 0) {
      show_page_index(current_page_index_ - 1);
    }
  }
}

void MainWindow::setup_gestures() {
  drag_ = Gtk::GestureDrag::create(stack_);
  drag_->set_touch_only(false);
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(drag_->gobj()),
                                             GTK_PHASE_CAPTURE);

  drag_->signal_drag_begin().connect([this](double, double) {
    drag_claimed_ = false;
    drag_t0_us_ = g_get_monotonic_time();
  });

  drag_->signal_drag_update().connect([this](double dx, double dy) {
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

MainWindow::MainWindow(bool fullscreen, bool decorated, bool skip_taskbar) {
  set_title("BBN Launcher");
  set_default_size(1400, 800);
  set_decorated(decorated);
  if (skip_taskbar) {
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
  }
  if (fullscreen) {
    this->fullscreen();
  }

  apply_css_provider_once();

  stack_.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  stack_.set_transition_duration(250);

  auto config = load_icon_config();
  palette_ = config.palette;
  pages_.reserve(config.pages.size());
  page_names_.reserve(config.pages.size());
  for (size_t i = 0; i < config.pages.size(); ++i) {
    auto* page = Gtk::manage(new Desktop(config.pages[i]));
    pages_.push_back(page);
    Glib::ustring name = "page" + std::to_string(i + 1);
    page_names_.push_back(name);
    stack_.add(*page, name);
  }

  btn_left_.set_label(cp_to_utf8(CHEV_LEFT));
  btn_right_.set_label(cp_to_utf8(CHEV_RIGHT));
  btn_left_.set_relief(Gtk::RELIEF_NONE);
  btn_right_.set_relief(Gtk::RELIEF_NONE);
  btn_left_.set_can_focus(false);
  btn_right_.set_can_focus(false);
  btn_left_.get_style_context()->add_class("nav");
  btn_right_.get_style_context()->add_class("nav");
  btn_left_.set_size_request(1, 1);
  btn_right_.set_size_request(1, 1);

  btn_left_.signal_clicked().connect([this] {
    if (current_page_index_ > 0) {
      show_page_index(current_page_index_ - 1);
    }
  });
  btn_right_.signal_clicked().connect([this] {
    if (current_page_index_ + 1 < pages_.size()) {
      show_page_index(current_page_index_ + 1);
    }
  });

  root_.pack_start(btn_left_, Gtk::PACK_SHRINK, 0);
  root_.pack_start(stack_, Gtk::PACK_EXPAND_WIDGET);
  root_.pack_start(btn_right_, Gtk::PACK_SHRINK, 0);

  scheme_bar_.set_halign(Gtk::ALIGN_START);
  scheme_bar_.set_valign(Gtk::ALIGN_END);

  scheme_day_.set_label(cp_to_utf8(U'\uf185'));   // sun
  scheme_dusk_.set_label(cp_to_utf8(U'\uf6c4'));  // cloud-sun
  scheme_night_.set_label(cp_to_utf8(U'\uf186')); // moon

  for (Gtk::Button* b : { &scheme_day_, &scheme_dusk_, &scheme_night_ }) {
    b->set_relief(Gtk::RELIEF_NONE);
    b->set_can_focus(false);
    b->set_size_request(1, 1);
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

  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);

  overlay_.signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::on_overlay_size_allocate));

  setup_gestures();

  apply_ui_scale(1400, 800);
  set_scheme(Scheme::Day);

  show_all();
  show_page_index(0);

  signal_realize().connect([this] {
    auto a = overlay_.get_allocation();
    apply_ui_scale(a.get_width(), a.get_height());
  });
}

void MainWindow::show_page_index(size_t index) {
  if (index >= page_names_.size()) return;
  current_page_index_ = index;
  stack_.set_visible_child(page_names_[index]);
  refresh_nav();
}

void MainWindow::show_page(const Glib::ustring& name) {
  stack_.set_visible_child(name);
  for (size_t i = 0; i < page_names_.size(); ++i) {
    if (page_names_[i] == name) {
      current_page_index_ = i;
      break;
    }
  }
  refresh_nav();
}

void MainWindow::refresh_nav() {
  if (page_names_.empty()) {
    btn_left_.set_sensitive(false);
    btn_right_.set_sensitive(false);
    return;
  }
  btn_left_.set_sensitive(current_page_index_ > 0);
  btn_right_.set_sensitive(current_page_index_ + 1 < page_names_.size());
}

bool MainWindow::on_key_press(GdkEventKey* e) {
  switch (e->keyval) {
    case GDK_KEY_Right:
    case GDK_KEY_Page_Down:
      if (current_page_index_ + 1 < pages_.size()) {
        show_page_index(current_page_index_ + 1);
      }
      return true;
    case GDK_KEY_Left:
    case GDK_KEY_Page_Up:
      if (current_page_index_ > 0) {
        show_page_index(current_page_index_ - 1);
      }
      return true;

    case GDK_KEY_1: set_scheme(Scheme::Day);   return true;
    case GDK_KEY_2: set_scheme(Scheme::Dusk);  return true;
    case GDK_KEY_3: set_scheme(Scheme::Night); return true;

    default:
      return false;
  }
}
