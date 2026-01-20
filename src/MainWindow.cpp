#include "MainWindow.h"
#include "Desktop.h"
#include "Icons.h"
#include "FontRegistry.h"

#include <iostream>

static const char* CSS_TEXT =
  "window { background: #000000; }\n"
  ".tile { background: transparent; border: none; box-shadow: none; }\n"
  ".tile-icon, .tile-label, .nav { color: #d00000; }\n";

static Glib::ustring cp_to_utf8(char32_t cp) {
  gunichar gcp = static_cast<gunichar>(cp);
  gchar buf[8] = {0};
  const int len = g_unichar_to_utf8(gcp, buf);
  buf[len] = '\0';
  return Glib::ustring(buf);
}

static void set_nav_button_font(Gtk::Button& b) {
  b.set_relief(Gtk::RELIEF_NONE);
  b.get_style_context()->add_class("nav");

  Pango::FontDescription fd;
  fd.set_family(FontRegistry::kFamilyFree);
  fd.set_weight(Pango::WEIGHT_NORMAL);
  fd.set_size(48 * Pango::SCALE);

  // Button child is a Gtk::Label in gtkmm when using set_label()
  if (auto* child = dynamic_cast<Gtk::Label*>(b.get_child())) {
    child->override_font(fd);
  }
}

MainWindow::MainWindow() {
  set_title("BBN Launcher");
  set_default_size(1400, 800);

  apply_css();

  // Stack transition like your UI
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

  // Use lambdas for handlers
  btn_left_.signal_clicked().connect([this] { show_page("page1"); });
  btn_right_.signal_clicked().connect([this] { show_page("page2"); });

  root_.set_spacing(10);
  root_.pack_start(btn_left_, Gtk::PACK_SHRINK, 18);
  root_.pack_start(stack_, Gtk::PACK_EXPAND_WIDGET);
  root_.pack_start(btn_right_, Gtk::PACK_SHRINK, 18);

  add(root_);

  // Key handling (Left/Right, PageUp/PageDown)
  signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press), false);

  show_all();
  show_page("page1");
}

void MainWindow::apply_css() {
  auto provider = Gtk::CssProvider::create();
  provider->load_from_data(CSS_TEXT);

  auto screen = Gdk::Screen::get_default();
  Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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
    default:
      return false;
  }
}
