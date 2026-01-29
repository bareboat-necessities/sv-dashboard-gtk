#include "Icons.h"

#include <json-glib/json-glib.h>
#include <glib.h>

#include <algorithm>
#include <cctype>
#include <unordered_map>

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

std::vector<std::string> read_args(JsonObject* obj) {
  std::vector<std::string> args;
  if (!json_object_has_member(obj, "args")) return args;
  auto* arr = json_object_get_array_member(obj, "args");
  if (!arr) return args;
  const guint n = json_array_get_length(arr);
  args.reserve(n);
  for (guint i = 0; i < n; ++i) {
    auto* node = json_array_get_element(arr, i);
    if (JSON_NODE_HOLDS_VALUE(node)) {
      args.emplace_back(json_node_get_string(node));
    }
  }
  return args;
}

const char* get_string_member(JsonObject* obj, const char* key, const char* fallback) {
  if (json_object_has_member(obj, key)) {
    return json_object_get_string_member(obj, key);
  }
  return fallback;
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

std::vector<IconSpec> read_page(JsonObject* root,
                                const char* key,
                                std::unordered_map<std::string, std::string>& palette) {
  std::vector<IconSpec> out;
  if (!json_object_has_member(root, key)) return out;

  auto* arr = json_object_get_array_member(root, key);
  if (!arr) return out;

  const guint n = json_array_get_length(arr);
  out.reserve(n);

  for (guint i = 0; i < n; ++i) {
    auto* node = json_array_get_element(arr, i);
    if (!JSON_NODE_HOLDS_OBJECT(node)) continue;
    auto* obj = json_node_get_object(node);
    if (!obj) continue;

    const char* title = get_string_member(obj, "title", "");
    const char* fa = get_string_member(obj, "fa", "");
    const char* bg = get_string_member(obj, "bg", "#455A64");
    const char* cmd = get_string_member(obj, "cmd", "");

    if (!fa || !*fa) {
      continue;
    }
    auto glyph = glyph_for_image(fa);
    std::string bg_value = bg ? bg : "#455A64";
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
    spec.label = title ? title : "";
    spec.colorClass = class_name;
    spec.command = cmd ? cmd : "";
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

IconConfig load_icon_config() {
  const char* env_path = g_getenv("SV_DASHBOARD_CONFIG");
  std::string config_path;
  if (env_path && *env_path) {
    config_path = env_path;
  } else {
    const char* cfg_dir = g_get_user_config_dir();
    config_path = std::string(cfg_dir ? cfg_dir : ".") + "/sv-dashboard-gtk/icons.json";
  }

  if (!g_file_test(config_path.c_str(), G_FILE_TEST_EXISTS)) {
    return default_icon_config();
  }

  GError* error = nullptr;
  JsonParser* parser = json_parser_new();
  gboolean ok = json_parser_load_from_file(parser, config_path.c_str(), &error);
  if (!ok || error) {
    if (error) g_error_free(error);
    g_object_unref(parser);
    return default_icon_config();
  }

  JsonNode* root_node = json_parser_get_root(parser);
  if (!JSON_NODE_HOLDS_OBJECT(root_node)) {
    g_object_unref(parser);
    return default_icon_config();
  }

  auto* root_obj = json_node_get_object(root_node);
  if (!root_obj) {
    g_object_unref(parser);
    return default_icon_config();
  }

  std::unordered_map<std::string, std::string> palette_map;
  IconConfig cfg;
  cfg.page1 = read_page(root_obj, "commands1", palette_map);
  cfg.page2 = read_page(root_obj, "commands2", palette_map);

  cfg.palette.reserve(palette_map.size());
  for (const auto& entry : palette_map) {
    cfg.palette.emplace_back(entry.first, entry.second);
  }

  g_object_unref(parser);

  if (cfg.page1.empty() && cfg.page2.empty()) {
    return default_icon_config();
  }

  return cfg;
}
