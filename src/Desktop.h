#pragma once
#include <gtkmm.h>
#include <vector>
#include "Icons.h"

// A 5×3 grid of DesktopIcon tiles.
class Desktop final : public Gtk::Box {
public:
  explicit Desktop(const std::vector<IconSpec>& icons);

private:
  Gtk::Grid grid_;
};
