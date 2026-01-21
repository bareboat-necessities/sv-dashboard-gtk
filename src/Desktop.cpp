#include "Desktop.h"
#include "DesktopIcon.h"

#include <cmath>

static constexpr int MARGIN_BASE = 40;
static constexpr int ROW_SP_BASE = 30;
static constexpr int COL_SP_BASE = 55;

Desktop::Desktop(const std::vector<IconSpec>& icons)
: Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
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

  // Default (will be overridden by MainWindow on first size-allocate)
  set_ui_scale(1.0, true);

  show_all_children();
}

void Desktop::set_ui_scale(double s, bool show_labels) {
  // Avoid thrashing
  if (last_s_ > 0 && std::fabs(s - last_s_) < 0.02 && show_labels == last_show_labels_) {
    return;
  }
  last_s_ = s;
  last_show_labels_ = show_labels;

  const int m  = std::max(2, (int)std::lround(MARGIN_BASE * s));
  const int rs = std::max(1, (int)std::lround(ROW_SP_BASE * s));
  const int cs = std::max(1, (int)std::lround(COL_SP_BASE * s));

  set_margin_start(m);
  set_margin_end(m);
  set_margin_top(m);
  set_margin_bottom(m);

  grid_.set_row_spacing(rs);
  grid_.set_column_spacing(cs);

  // Forward scale to each icon
  for (auto* w : grid_.get_children()) {
    if (auto* icon = dynamic_cast<DesktopIcon*>(w)) {
      icon->set_ui_scale(s, show_labels);
    }
  }
}
