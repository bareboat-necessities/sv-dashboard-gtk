#pragma once

#include <gtkmm.h>

class Desktop;

class MainWindow : public Gtk::Window {
public:
  MainWindow();

private:
  enum class Scheme { Day, Dusk, Night };

  void apply_css_provider_once();
  Glib::ustring build_css(Scheme s) const;
  void set_scheme(Scheme s);
  void refresh_scheme_buttons();

  void show_page(const Glib::ustring& name);
  void refresh_nav();

  bool on_key_press(GdkEventKey* e);

  // Scaling
  void on_overlay_size_allocate(Gtk::Allocation& alloc);
  void apply_ui_scale(int w, int h);

  // Swipe/drag
  void setup_gestures();
  void handle_swipe_delta(double dx, double dy, guint32 dt_ms);

  // Layout
  Gtk::Overlay overlay_;
  Gtk::Box     root_{Gtk::ORIENTATION_HORIZONTAL};

  Gtk::Stack   stack_;
  Gtk::EventBox swipe_box_;
  Gtk::Button  btn_left_;
  Gtk::Button  btn_right_;

  Desktop* page1_ = nullptr;
  Desktop* page2_ = nullptr;

  // Bottom-left scheme buttons
  Gtk::Box     scheme_bar_{Gtk::ORIENTATION_HORIZONTAL};
  Gtk::Button  scheme_day_;
  Gtk::Button  scheme_dusk_;
  Gtk::Button  scheme_night_;

  Glib::RefPtr<Gtk::CssProvider> css_provider_;

  Scheme scheme_ = Scheme::Day;

  // IMPORTANT: force first scale apply
  double ui_scale_ = -1.0;
  bool show_labels_ = true;

  // Gesture state
  Glib::RefPtr<Gtk::GestureDrag> drag_;
  bool   drag_claimed_ = false;
  gint64 drag_t0_us_ = 0;

  // thresholds
  static constexpr double  kSwipeLockPx      = 18.0;
  static constexpr double  kSwipeMinPx       = 120.0;
  static constexpr double  kSwipeFastMinPx   = 70.0;
  static constexpr guint32 kSwipeFastMaxMs   = 260;
};
