#include "Icons.h"
#include "RuntimeEnv.h"

#include <glib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
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

struct YamlNode {
  enum class Type { Null, Scalar, Map, Seq };
  Type type{Type::Null};
  std::string scalar;
  std::unordered_map<std::string, YamlNode> map;
  std::vector<YamlNode> seq;
};

struct YamlFrame {
  int indent;
  YamlNode* node;
};

std::string trim_copy(const std::string& value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }
  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }
  return value.substr(start, end - start);
}

std::string strip_quotes(const std::string& value) {
  if (value.size() >= 2) {
    const char first = value.front();
    const char last = value.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
      return value.substr(1, value.size() - 2);
    }
  }
  return value;
}

int line_indent(const std::string& line) {
  int count = 0;
  for (char c : line) {
    if (c == ' ') {
      ++count;
    } else {
      break;
    }
  }
  return count;
}

bool is_blank_or_comment(const std::string& line) {
  for (char c : line) {
    if (std::isspace(static_cast<unsigned char>(c))) continue;
    return c == '#';
  }
  return true;
}

std::string strip_inline_comment(const std::string& value) {
  bool in_single = false;
  bool in_double = false;
  for (size_t i = 0; i < value.size(); ++i) {
    char c = value[i];
    if (c == '"' && !in_single) {
      in_double = !in_double;
    } else if (c == '\'' && !in_double) {
      in_single = !in_single;
    } else if (c == '#' && !in_single && !in_double) {
      if (i == 0 || std::isspace(static_cast<unsigned char>(value[i - 1]))) {
        return trim_copy(value.substr(0, i));
      }
    }
  }
  return trim_copy(value);
}

bool find_next_content(const std::vector<std::string>& lines,
                       size_t start,
                       size_t& out_index,
                       int& out_indent,
                       std::string& out_content) {
  for (size_t i = start; i < lines.size(); ++i) {
    if (is_blank_or_comment(lines[i])) continue;
    out_index = i;
    out_indent = line_indent(lines[i]);
    out_content = trim_copy(lines[i].substr(static_cast<size_t>(out_indent)));
    return true;
  }
  return false;
}

bool parse_yaml_file(const std::string& path, YamlNode& out) {
  std::ifstream in(path);
  if (!in) {
    return false;
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    lines.push_back(line);
  }

  out = YamlNode{};
  out.type = YamlNode::Type::Map;
  std::vector<YamlFrame> stack;
  stack.push_back({-1, &out});

  for (size_t i = 0; i < lines.size(); ++i) {
    if (is_blank_or_comment(lines[i])) continue;

    int indent = line_indent(lines[i]);
    std::string content = trim_copy(lines[i].substr(static_cast<size_t>(indent)));
    if (content.empty()) continue;

    while (stack.size() > 1 && indent <= stack.back().indent) {
      stack.pop_back();
    }

    YamlNode* parent = stack.back().node;

    if (content.rfind("- ", 0) == 0) {
      if (parent->type != YamlNode::Type::Seq) {
        return false;
      }
      std::string item = trim_copy(content.substr(2));
      if (item.empty()) {
        YamlNode node;
        node.type = YamlNode::Type::Map;
        parent->seq.emplace_back(std::move(node));
        stack.push_back({indent, &parent->seq.back()});
        continue;
      }
      auto colon_pos = item.find(':');
      if (colon_pos == std::string::npos) {
        YamlNode node;
        node.type = YamlNode::Type::Scalar;
        node.scalar = strip_quotes(strip_inline_comment(item));
        parent->seq.emplace_back(std::move(node));
        continue;
      }
      std::string key = trim_copy(item.substr(0, colon_pos));
      std::string value = strip_inline_comment(item.substr(colon_pos + 1));
      YamlNode map_node;
      map_node.type = YamlNode::Type::Map;
      YamlNode value_node;
      if (value.empty()) {
        value_node.type = YamlNode::Type::Null;
      } else if (value == "[]") {
        value_node.type = YamlNode::Type::Seq;
      } else {
        value_node.type = YamlNode::Type::Scalar;
        value_node.scalar = strip_quotes(value);
      }
      map_node.map.emplace(key, std::move(value_node));
      parent->seq.emplace_back(std::move(map_node));
      if (value.empty()) {
        stack.push_back({indent, &parent->seq.back()});
      }
      continue;
    }

    if (parent->type != YamlNode::Type::Map) {
      return false;
    }

    auto colon_pos = content.find(':');
    if (colon_pos == std::string::npos) {
      return false;
    }
    std::string key = trim_copy(content.substr(0, colon_pos));
    std::string value = strip_inline_comment(content.substr(colon_pos + 1));

    if (value == "[]") {
      YamlNode node;
      node.type = YamlNode::Type::Seq;
      parent->map[key] = std::move(node);
      continue;
    }

    if (!value.empty()) {
      YamlNode node;
      node.type = YamlNode::Type::Scalar;
      node.scalar = strip_quotes(value);
      parent->map[key] = std::move(node);
      continue;
    }

    size_t next_index = 0;
    int next_indent = 0;
    std::string next_content;
    YamlNode node;
    if (find_next_content(lines, i + 1, next_index, next_indent, next_content) &&
        next_indent > indent && next_content.rfind("- ", 0) == 0) {
      node.type = YamlNode::Type::Seq;
    } else {
      node.type = YamlNode::Type::Map;
    }
    parent->map[key] = std::move(node);
    stack.push_back({indent, &parent->map[key]});
  }

  return true;
}

const YamlNode* find_child(const YamlNode& node, const char* key) {
  if (node.type != YamlNode::Type::Map) return nullptr;
  auto it = node.map.find(key);
  if (it == node.map.end()) return nullptr;
  return &it->second;
}

std::vector<std::string> read_args(const YamlNode& obj) {
  std::vector<std::string> args;
  const auto* args_node = find_child(obj, "args");
  if (!args_node || args_node->type != YamlNode::Type::Seq) return args;
  args.reserve(args_node->seq.size());
  for (const auto& node : args_node->seq) {
    if (node.type != YamlNode::Type::Scalar) continue;
    if (!node.scalar.empty()) {
      args.emplace_back(node.scalar);
    }
  }
  return args;
}

std::string get_string_member(const YamlNode& obj,
                              const char* key,
                              const char* fallback) {
  const auto* node = find_child(obj, key);
  if (node && node->type == YamlNode::Type::Scalar) {
    return node->scalar;
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

std::vector<IconSpec> read_page(const YamlNode& root,
                                const char* key,
                                std::unordered_map<std::string, std::string>& palette) {
  std::vector<IconSpec> out;
  const auto* arr = find_child(root, key);
  if (!arr || arr->type != YamlNode::Type::Seq) return out;

  out.reserve(arr->seq.size());

  for (const auto& obj : arr->seq) {
    if (obj.type != YamlNode::Type::Map) continue;

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
#ifdef _WIN32
  return RuntimeEnv::localAppDataDir() + "/sv-dashboard-gtk/sv-dashboard.yaml";
#else
  const char* cfg_dir = g_get_user_config_dir();
  return std::string(cfg_dir ? cfg_dir : ".") + "/sv-dashboard-gtk/sv-dashboard.yaml";
#endif
}

std::string exe_dir() {
#ifdef _WIN32
  return RuntimeEnv::exeDir();
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
  }
  const std::string bin_dir = exe_dir();
  paths.emplace_back(bin_dir + "/../share/sv-dashboard-gtk/sv-dashboard.yaml");
  paths.emplace_back(bin_dir + "/share/sv-dashboard-gtk/sv-dashboard.yaml");
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

  YamlNode root;
  if (!parse_yaml_file(config_path, root)) {
    return default_icon_config();
  }
  if (root.type != YamlNode::Type::Map) {
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
