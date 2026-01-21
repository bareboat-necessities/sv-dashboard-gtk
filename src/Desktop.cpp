#include "Desktop.h"
#include "DesktopIcon.h"

#include <unordered_map>
#include <string>
#include <cmath>

static const char* color_for_label(const std::string& label) {
  // Palette classes must exist in MainWindow CSS (Day scheme).
  static const std::unordered_map<std::string, const char*> m = {
    {"Freeboard",    "bg-azure"},
    {"Sky",          "bg-cyan"},
    {"Moorings",     "bg-indigo"},
    {"Provisioning", "bg-teal"},
    {"AvNav",        "bg-purple"},

    {"Vessel",       "bg-blue"},
    {"SignalK",      "bg-slate"},
    {"Terminal",     "bg-gray"},
    {"Tasks",        "bg-slate-dark"},
    {"Files",        "bg-gray"},

    {"Radio",        "bg-indigo"},
    {"Drones",       "bg-cyan"},
    {"Web Cam",      "bg-blue"},
    {"Messenger",    "bg-purple"},
    {"Social",       "bg-red"},

    {"OpenCPN",      "bg-azure"},
    {"KIP",          "bg-indigo"},
    {"Power",        "bg-teal-light"},
    {"GRIB",         "bg-cyan"},
    {"Camera",       "bg-blue"},

    {"qtVlm",        "bg-azure"},
    {"Instruments",  "bg-indigo"},
    {"PyPilot",      "bg-teal"},
    {"Windy",        "bg-cyan"},
    {"Email",        "bg-red"},

    {"Music",        "bg-purple"},
    {"Video",        "bg-red"},
    {"Commands",     "bg-slate"},
    {"T-Storms",     "bg-indigo"},
    {"Chrome",       "bg-blue"},
  };

  auto it = m.find(label);
  return (it != m.end()) ? it->second : "bg-gray";
}

Desktop::Desktop(const std::vector<IconSpec>& icons)
: Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
  grid_.set_row_homogeneous(true);
  grid_.set_column_homogeneous(true);
  grid_.set_halign(Gtk::ALIGN_CENTER);
  grid_.set_valign(Gtk::ALIGN_CENTER);

  tiles_.reserve(icons.size());

  for (int i = 0; i < (int)icons.size(); ++i) {
    const int r = i / kCols;
    const int c = i % kCols;

    auto* tile = Gtk::manage(new DesktopIcon(icons.at(i)));
    tile->set_color_class(color_for_label(icons.at(i).label));
    tiles_.push_back(tile);

    grid_.attach(*tile, c, r, 1, 1);
  }

  pack_start(grid_, Gtk::PACK_EXPAND_WIDGET);

  set_ui_scale(1.0, true);
}

void Desktop::apply_layout(double s) {
  const int m = std::max(2, (int)std::lround(kMarginBase * s));
  set_margin_start(m);
  set_margin_end(m);
  set_margin_top(m);
  set_margin_bottom(m);

  grid_.set_row_spacing(std::max(0, (int)std::lround(kRowSpacingBase * s)));
  grid_.set_column_spacing(std::max(0, (int)std::lround(kColSpacingBase * s)));
}

void Desktop::set_ui_scale(double s, bool show_labels) {
  ui_scale_ = s;
  show_labels_ = show_labels;

  apply_layout(s);

  for (auto* t : tiles_) {
    t->set_ui_scale(s, show_labels_);
  }
}
