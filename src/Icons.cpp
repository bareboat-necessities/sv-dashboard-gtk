#include "Icons.h"

#include <yaml-cpp/yaml.h>
#include <glib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace {

struct GlyphSpec {
  char32_t codepoint{};
  bool isBrand{false};
};

const std::unordered_map<std::string, GlyphSpec>& glyph_map() {
  static const std::unordered_map<std::string, GlyphSpec> map = {
    {"chart",        {U'\uf5a0', false}},
    {"chart-area",   {U'\uf5a0', false}},
    {"dashboard",    {U'\uf624', false}},
    {"tachometer-alt", {U'\uf624', false}},
    {"autopilot",    {U'\uf1d8', false}},
    {"paper-plane",  {U'\uf1d8', false}},
    {"weather",      {U'\uf743', false}},
    {"cloud-sun-rain", {U'\uf743', false}},
    {"camera",       {U'\uf030', false}},
    {"race",         {U'\uf0ac', false}},
    {"flag-checkered", {U'\uf0ac', false}},
    {"multimedia",   {U'\uf001', false}},
    {"music",        {U'\uf001', false}},
    {"youtube",      {U'\uf167', true}},
    {"travel",       {U'\uf6ec', false}},
    {"route",        {U'\uf6ec', false}},
    {"internet",     {U'\uf268', true}},
    {"globe",        {U'\uf268', true}},
    {"email",        {U'\uf0e0', false}},
    {"envelope",     {U'\uf0e0', false}},
    {"radio",        {U'\uf8d7', false}},
    {"broadcast-tower", {U'\uf8d7', false}},
    {"sky",          {U'\uf005', false}},
    {"star",         {U'\uf005', false}},
    {"buoy",         {U'\uf13d', false}},
    {"anchor",       {U'\uf13d', false}},
    {"provisioning", {U'\uf2e7', false}},
    {"shopping-basket", {U'\uf2e7', false}},
    {"ship",         {U'\uf21a', false}},
    {"terminal",     {U'\uf120', false}},
    {"tasks",        {U'\uf0ae', false}},
    {"folder",       {U'\uf07b', false}},
    {"solar",        {U'\uf5ba', false}},
    {"solar-panel",  {U'\uf5ba', false}},
    {"drone",        {U'\uf072', false}},
    {"facebook",     {U'\uf39e', true}},
    {"vessel",       {U'\uf21a', false}},
    {"school",       {U'\uf549', false}},
    {"knot",         {U'\uf6f0', false}},
    {"colreg",       {U'\uf2c1', false}},
    {"book",         {U'\uf2c1', false}},
    {"chess",        {U'\uf439', false}},
  };
  return map;
}

GlyphSpec glyph_for_image(const std::string& name) {
  const auto& map = glyph_map();
  auto it = map.find(name);
  if (it != map.end()) {
    return it->second;
  }
  return {U'\uf128', false};
}

std::string slugify_color(const std::string& input) {
  std::string out;
  out.reserve(input.size());
  for (char c : input) {
    if (std::isalnum(static_cast<unsigned char>(c))) {
      out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    } else if (c == '#') {
      out.push_back('h');
      out.push_back('e');
      out.push_back('x');
      out.push_back('-');
    } else if (!out.empty() && out.back() != '-') {
      out.push_back('-');
    }
  }
  if (out.empty()) out = "default";
  return out;
}

std::string color_class_for(const std::string& color) {
  if (color.rfind("bg-", 0) == 0) {
    return color;
  }
  return "bg-" + slugify_color(color);
}

std::vector<std::string> read_args(const YAML::Node& obj) {
  std::vector<std::string> args;
  const auto args_node = obj["args"];
  if (!args_node || !args_node.IsSequence()) return args;
  args.reserve(args_node.size());
  for (const auto& node : args_node) {
    if (!node.IsScalar()) continue;
    const auto value = node.as<std::string>();
    if (!value.empty()) {
      args.emplace_back(value);
    }
  }
  return args;
}

std::string get_string_member(const YAML::Node& obj,
                              const char* key,
                              const char* fallback) {
  const auto node = obj[key];
  if (node && node.IsScalar()) {
    return node.as<std::string>();
  }
  return fallback ? fallback : "";
}

const std::unordered_map<std::string, std::string>& default_palette_map() {
  static const std::unordered_map<std::string, std::string> map = {
    {"bg-azure", "#007ACC"},
    {"bg-blue", "#1976D2"},
    {"bg-teal", "#009688"},
    {"bg-teal-light", "#26A69A"},
    {"bg-cyan", "#06B6D4"},
    {"bg-indigo", "#5C6BC0"},
    {"bg-gray", "#455A64"},
    {"bg-slate", "#556F7B"},
    {"bg-slate-dark", "#546E7A"},
    {"bg-purple", "#8E24AA"},
    {"bg-violet", "#7E22CE"},
    {"bg-red", "#DC2626"},
  };
  return map;
}

std::vector<IconSpec> read_page(const YAML::Node& root,
                                const char* key,
                                std::unordered_map<std::string, std::string>& palette) {
  std::vector<IconSpec> out;
  const auto arr = root[key];
  if (!arr || !arr.IsSequence()) return out;

  out.reserve(arr.size());

  for (const auto& obj : arr) {
    if (!obj || !obj.IsMap()) continue;

    const std::string title = get_string_member(obj, "title", "");
    const std::string fa = get_string_member(obj, "fa", "");
    const std::string bg = get_string_member(obj, "bg", "#455A64");
    std::string cmd = get_string_member(obj, "cmd", "");
    if (cmd.empty()) {
      cmd = get_string_member(obj, "command", "");
    }

    if (fa.empty()) {
      continue;
    }
    auto glyph = glyph_for_image(fa);
    std::string bg_value = bg;
    std::string class_name = color_class_for(bg_value);
    if (!bg_value.empty()) {
      if (bg_value.rfind("bg-", 0) == 0) {
        auto it = default_palette_map().find(bg_value);
        if (it != default_palette_map().end()) {
          palette[class_name] = it->second;
        }
      } else {
        palette[class_name] = bg_value;
      }
    }

    IconSpec spec;
    spec.codepoint = glyph.codepoint;
    spec.isBrand = glyph.isBrand;
    spec.label = title;
    spec.colorClass = class_name;
    spec.command = cmd;
    spec.args = read_args(obj);

    out.push_back(std::move(spec));
  }

  return out;
}

IconConfig default_icon_config() {
  IconConfig cfg;
  cfg.page1 = {
    { U'\uf5a0', "Freeboard",     false, "bg-azure", "", {} },
    { U'\uf005', "Sky",           false, "bg-indigo", "", {} },
    { U'\uf13d', "Moorings",      false, "bg-blue", "", {} },
    { U'\uf2e7', "Provisioning",  false, "bg-slate", "", {} },
    { U'\uf5a0', "AvNav",         false, "bg-azure", "", {} },

    { U'\uf21a', "Vessel",        false, "bg-blue", "", {} },
    { U'\uf013', "SignalK",       false, "bg-teal", "", {} },
    { U'\uf120', "Terminal",      false, "bg-slate-dark", "", {} },
    { U'\uf0ae', "Tasks",         false, "bg-slate-dark", "", {} },
    { U'\uf07b', "Files",         false, "bg-slate", "", {} },

    { U'\uf8d7', "Radio",         false, "bg-purple", "", {} },
    { U'\uf072', "Drones",        false, "bg-indigo", "", {} },
    { U'\uf030', "Web Cam",       false, "bg-gray", "", {} },
    { U'\uf39f', "Messenger",     true,  "bg-blue", "", {} },
    { U'\uf39e', "Social",        true,  "bg-blue", "", {} },
  };

  cfg.page2 = {
    { U'\uf5a0', "OpenCPN",       false, "bg-blue", "", {} },
    { U'\uf624', "KIP",           false, "bg-teal", "", {} },
    { U'\uf5ba', "Power",         false, "bg-teal-light", "", {} },
    { U'\uf743', "GRIB",          false, "bg-blue", "", {} },
    { U'\uf030', "Camera",        false, "bg-gray", "", {} },

    { U'\uf5a0', "qtVlm",         false, "bg-blue", "", {} },
    { U'\uf624', "Instruments",   false, "bg-teal", "", {} },
    { U'\uf1d8', "PyPilot",       false, "bg-cyan", "", {} },
    { U'\uf72e', "Windy",         false, "bg-indigo", "", {} },
    { U'\uf0e0', "Email",         false, "bg-blue", "", {} },

    { U'\uf001', "Music",         false, "bg-red", "", {} },
    { U'\uf167', "Video",         true,  "bg-red", "", {} },
    { U'\uf011', "Commands",      false, "bg-slate-dark", "", {} },
    { U'\uf76c', "T-Storms",      false, "bg-violet", "", {} },
    { U'\uf268', "Chrome",        true,  "bg-blue", "", {} },
  };

  cfg.palette.reserve(default_palette_map().size());
  for (const auto& entry : default_palette_map()) {
    cfg.palette.emplace_back(entry.first, entry.second);
  }
  return cfg;
}

} // namespace

std::string user_config_path() {
  const char* cfg_dir = g_get_user_config_dir();
  return std::string(cfg_dir ? cfg_dir : ".") + "/sv-dashboard-gtk/sv-dashboard.yaml";
}

std::string exe_dir() {
#ifdef _WIN32
  return ".";
#else
  std::string out = ".";
  std::array<char, 4096> buf{};
  const ssize_t len = readlink("/proc/self/exe", buf.data(), buf.size() - 1);
  if (len > 0) {
    buf[static_cast<size_t>(len)] = '\0';
    std::filesystem::path p(buf.data());
    out = p.parent_path().string();
  }
  return out;
#endif
}

std::vector<std::string> default_config_candidates() {
  std::vector<std::string> paths;
  const gchar* const* data_dirs = g_get_system_data_dirs();
  for (size_t i = 0; data_dirs && data_dirs[i]; ++i) {
    paths.emplace_back(std::string(data_dirs[i]) + "/sv-dashboard-gtk/sv-dashboard.yaml");
    paths.emplace_back(std::string(data_dirs[i]) + "/sv-dashboard-gtk/assets/sv-dashboard.yaml");
  }
  const std::string bin_dir = exe_dir();
  paths.emplace_back(bin_dir + "/../share/sv-dashboard-gtk/sv-dashboard.yaml");
  paths.emplace_back(bin_dir + "/share/sv-dashboard-gtk/sv-dashboard.yaml");
  paths.emplace_back(bin_dir + "/../assets/sv-dashboard.yaml");
  paths.emplace_back(bin_dir + "/assets/sv-dashboard.yaml");
  if (char* cwd = g_get_current_dir()) {
    paths.emplace_back(std::string(cwd) + "/assets/sv-dashboard.yaml");
    g_free(cwd);
  }
  return paths;
}

std::string find_default_config_path() {
  for (const auto& path : default_config_candidates()) {
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS)) {
      return path;
    }
  }
  return {};
}

void copy_default_config_if_missing(const std::string& source, const std::string& dest) {
  if (source.empty() || dest.empty()) return;
  if (g_file_test(dest.c_str(), G_FILE_TEST_EXISTS)) return;

  std::filesystem::path dest_path(dest);
  const std::string dir = dest_path.parent_path().string();
  if (!dir.empty()) {
    g_mkdir_with_parents(dir.c_str(), 0755);
  }

  gchar* contents = nullptr;
  gsize len = 0;
  GError* error = nullptr;
  if (!g_file_get_contents(source.c_str(), &contents, &len, &error)) {
    if (error) g_error_free(error);
    return;
  }

  if (!g_file_set_contents(dest.c_str(), contents, len, &error)) {
    if (error) g_error_free(error);
  }
  g_free(contents);
}

IconConfig load_icon_config() {
  const char* env_path = g_getenv("SV_DASHBOARD_CONFIG");
  std::string config_path;
  if (env_path && *env_path) {
    config_path = env_path;
  } else {
    config_path = user_config_path();
  }

  if (!g_file_test(config_path.c_str(), G_FILE_TEST_EXISTS)) {
    const std::string fallback = find_default_config_path();
    if (!fallback.empty()) {
      copy_default_config_if_missing(fallback, config_path);
      if (!g_file_test(config_path.c_str(), G_FILE_TEST_EXISTS)) {
        config_path = fallback;
      }
    }
  }

  if (!g_file_test(config_path.c_str(), G_FILE_TEST_EXISTS)) {
    return default_icon_config();
  }

  YAML::Node root;
  try {
    root = YAML::LoadFile(config_path);
  } catch (const YAML::Exception&) {
    return default_icon_config();
  }
  if (!root || !root.IsMap()) {
    return default_icon_config();
  }

  std::unordered_map<std::string, std::string> palette_map;
  IconConfig cfg;
  cfg.page1 = read_page(root, "commands1", palette_map);
  cfg.page2 = read_page(root, "commands2", palette_map);

  cfg.palette.reserve(palette_map.size());
  for (const auto& entry : palette_map) {
    cfg.palette.emplace_back(entry.first, entry.second);
  }

  if (cfg.page1.empty() && cfg.page2.empty()) {
    return default_icon_config();
  }

  return cfg;
}
