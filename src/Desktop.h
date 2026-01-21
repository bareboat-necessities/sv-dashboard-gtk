#pragma once

#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <vector>

#include "Icons.h"

class DesktopIcon;

class Desktop : public Gtk::Box {
public:
  explicit Desktop(const std::vector<IconSpec>& icons);

  void set_ui_scale(double s, bool show_labels);

private:
  void apply_layout(double s);

  Gtk::Grid grid_;
  std::vector<DesktopIcon*> tiles_;

  double ui_scale_ = 1.0;
  bool show_labels_ = true;

  static constexpr int kMarginBase     = 40;
  static constexpr int kRowSpacingBase = 30;
  static constexpr int kColSpacingBase = 55;
};
