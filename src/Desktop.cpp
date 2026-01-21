#include "Desktop.h"
#include "DesktopIcon.h"

Desktop::Desktop(const std::vector<IconSpec>& icons)
: Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
  set_margin_start(40);
  set_margin_end(40);
  set_margin_top(40);
  set_margin_bottom(40);

  grid_.set_row_spacing(30);
  grid_.set_column_spacing(55);
  grid_.set_row_homogeneous(true);
  grid_.set_column_homogeneous(true);

  grid_.set_halign(Gtk::ALIGN_CENTER);
  grid_.set_valign(Gtk::ALIGN_CENTER);

  for (int i = 0; i < (int)icons.size(); ++i) {
    const int r = i / kCols;
    const int c = i % kCols;

    auto* tile = Gtk::manage(new DesktopIcon(icons.at(i)));
    grid_.attach(*tile, c, r, 1, 1);
  }

  pack_start(grid_, Gtk::PACK_EXPAND_WIDGET);

  // Helps when this is placed inside a Gtk::Stack
  show_all_children();
}
