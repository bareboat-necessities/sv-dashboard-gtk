#pragma once

#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <vector>

#include "Icons.h"

class Desktop : public Gtk::Box {
public:
  explicit Desktop(const std::vector<IconSpec>& icons);

  // NEW: scales margins/spacing and forwards scale to icons
  void set_ui_scale(double s, bool show_labels);

private:
  Gtk::Grid grid_;
  double last_s_ = -1.0;
  bool last_show_labels_ = true;
};
