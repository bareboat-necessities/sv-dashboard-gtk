#pragma once

#include <gtkmm.h>

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

  // NEW: one handler catches mouse + touch events for swipe/drag
  bool on_any_event(GdkEvent* e);
  void handle_swipe_delta(double dx, double dy, guint32 dt_ms);

  // Layout
  Gtk::Overlay overlay_;
  Gtk::Box     root_{Gtk::ORIENTATION_HORIZONTAL};

  Gtk::Stack   stack_;
  Gtk::Button  btn_left_;
  Gtk::Button  btn_right_;

  // Bottom-left scheme buttons
  Gtk::Box     scheme_bar_{Gtk::ORIENTATION_HORIZONTAL};
  Gtk::Button  scheme_day_;
  Gtk::Button  scheme_dusk_;
  Gtk::Button  scheme_night_;

  // One provider, reloaded on scheme change
  Glib::RefPtr<Gtk::CssProvider> css_provider_;

  Scheme scheme_ = Scheme::Day;

  // ---- Swipe state ----
  bool    swipe_tracking_ = false;
  bool    swipe_locked_   = false;   // once we detect a real drag, we "own" the gesture
  double  swipe_start_x_  = 0.0;
  double  swipe_start_y_  = 0.0;
  double  swipe_last_x_   = 0.0;
  double  swipe_last_y_   = 0.0;
  guint32 swipe_start_t_  = 0;

  // thresholds
  static constexpr double  kSwipeLockPx      = 18.0;   // start treating as drag
  static constexpr double  kSwipeMinPx       = 120.0;  // switch pages if dx beyond this
  static constexpr double  kSwipeFastMinPx   = 70.0;   // or smaller + fast
  static constexpr guint32 kSwipeFastMaxMs   = 260;    // fast swipe time window
};
