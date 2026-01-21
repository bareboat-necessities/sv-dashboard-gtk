#pragma once

#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <vector>

#include "Icons.h"

class Desktop : public Gtk::Box {
public:
  explicit Desktop(const std::vector<IconSpec>& icons);

private:
  static constexpr int kCols = 5;   // 5 columns like your screenshots
  Gtk::Grid grid_;
};
