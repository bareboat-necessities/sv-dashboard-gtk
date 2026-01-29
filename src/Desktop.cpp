#include "Desktop.h"
#include "DesktopIcon.h"

#include <string>
#include <utility>
#include <cmath>
#include <vector>

#include <glib.h>

namespace {

std::vector<std::string> build_command_argv(const IconSpec& spec) {
  std::string cmd = spec.command;
  std::vector<std::string> args = spec.args;

  if (cmd == "onlyone") {
    if (args.empty()) return {};
    cmd = args.front();
    args.erase(args.begin());
  }

  if (cmd.empty()) return {};

  std::vector<std::string> argv;
  argv.reserve(1 + args.size());
  argv.push_back(cmd);
  argv.insert(argv.end(), args.begin(), args.end());
  return argv;
}

void launch_command(const IconSpec& spec) {
  auto argv_strings = build_command_argv(spec);
  if (argv_strings.empty()) return;

  std::vector<char*> argv;
  argv.reserve(argv_strings.size() + 1);
  for (auto& arg : argv_strings) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);

  GError* error = nullptr;
  g_spawn_async(nullptr,
                argv.data(),
                nullptr,
                G_SPAWN_SEARCH_PATH,
                nullptr,
                nullptr,
                nullptr,
                &error);
  if (error) {
    g_warning("Failed to launch command: %s", error->message);
    g_error_free(error);
  }
}

} // namespace

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

    const auto& spec = icons.at(i);
    auto* tile = Gtk::manage(new DesktopIcon(spec));
    tile->set_color_class(spec.colorClass);
    tile->signal_clicked().connect([spec] { launch_command(spec); });
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
