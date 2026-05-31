#include "Icons.h"
#include "RuntimeEnv.h"

#include <glib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstring>
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

struct JsonValue {
  enum class Type { Null, Bool, Number, String, Object, Array };
  Type type{Type::Null};
  bool boolean{false};
  double number{0.0};
  std::string string;
  std::unordered_map<std::string, JsonValue> object;
  std::vector<JsonValue> array;
};

class JsonParser {
public:
  explicit JsonParser(std::string text) : text_(std::move(text)) {}

  bool parse(JsonValue& out, std::string& error) {
    skip_ws();
    if (!parse_value(out, error)) {
      return false;
    }
    skip_ws();
    if (pos_ != text_.size()) {
      error = "unexpected trailing content";
      return false;
    }
    return true;
  }

private:
  bool parse_value(JsonValue& out, std::string& error) {
    skip_ws();
    if (pos_ >= text_.size()) {
      error = "unexpected end of input";
      return false;
    }

    const char c = text_[pos_];
    if (c == '{') return parse_object(out, error);
    if (c == '[') return parse_array(out, error);
    if (c == '"') {
      out = JsonValue{};
      out.type = JsonValue::Type::String;
      return parse_string(out.string, error);
    }
    if (c == 't') return parse_literal("true", JsonValue::Type::Bool, out, error, true);
    if (c == 'f') return parse_literal("false", JsonValue::Type::Bool, out, error, false);
    if (c == 'n') return parse_literal("null", JsonValue::Type::Null, out, error, false);
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
      return parse_number(out, error);
    }

    error = "unexpected character";
    return false;
  }

  bool parse_object(JsonValue& out, std::string& error) {
    out = JsonValue{};
    out.type = JsonValue::Type::Object;
    ++pos_; // {
    skip_ws();
    if (consume('}')) return true;

    while (true) {
      skip_ws();
      std::string key;
      if (!parse_string(key, error)) {
        if (error.empty()) error = "expected object key string";
        return false;
      }
      skip_ws();
      if (!consume(':')) {
        error = "expected ':' after object key";
        return false;
      }
      JsonValue value;
      if (!parse_value(value, error)) return false;
      out.object[std::move(key)] = std::move(value);
      skip_ws();
      if (consume('}')) return true;
      if (!consume(',')) {
        error = "expected ',' or '}' in object";
        return false;
      }
    }
  }

  bool parse_array(JsonValue& out, std::string& error) {
    out = JsonValue{};
    out.type = JsonValue::Type::Array;
    ++pos_; // [
    skip_ws();
    if (consume(']')) return true;

    while (true) {
      JsonValue value;
      if (!parse_value(value, error)) return false;
      out.array.emplace_back(std::move(value));
      skip_ws();
      if (consume(']')) return true;
      if (!consume(',')) {
        error = "expected ',' or ']' in array";
        return false;
      }
    }
  }

  bool parse_string(std::string& out, std::string& error) {
    if (!consume('"')) {
      error = "expected string";
      return false;
    }

    out.clear();
    while (pos_ < text_.size()) {
      const char c = text_[pos_++];
      if (c == '"') return true;
      if (static_cast<unsigned char>(c) < 0x20) {
        error = "control character in string";
        return false;
      }
      if (c != '\\') {
        out.push_back(c);
        continue;
      }
      if (pos_ >= text_.size()) {
        error = "unterminated escape sequence";
        return false;
      }
      const char esc = text_[pos_++];
      switch (esc) {
        case '"': out.push_back('"'); break;
        case '\\': out.push_back('\\'); break;
        case '/': out.push_back('/'); break;
        case 'b': out.push_back('\b'); break;
        case 'f': out.push_back('\f'); break;
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        case 'u':
          if (!append_unicode_escape(out, error)) return false;
          break;
        default:
          error = "invalid escape sequence";
          return false;
      }
    }

    error = "unterminated string";
    return false;
  }

  bool append_unicode_escape(std::string& out, std::string& error) {
    if (pos_ + 4 > text_.size()) {
      error = "incomplete unicode escape";
      return false;
    }

    unsigned int code = 0;
    for (int i = 0; i < 4; ++i) {
      const char c = text_[pos_++];
      code <<= 4;
      if (c >= '0' && c <= '9') code += static_cast<unsigned int>(c - '0');
      else if (c >= 'a' && c <= 'f') code += static_cast<unsigned int>(c - 'a' + 10);
      else if (c >= 'A' && c <= 'F') code += static_cast<unsigned int>(c - 'A' + 10);
      else {
        error = "invalid unicode escape";
        return false;
      }
    }

    if (code <= 0x7F) {
      out.push_back(static_cast<char>(code));
    } else if (code <= 0x7FF) {
      out.push_back(static_cast<char>(0xC0 | (code >> 6)));
      out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xE0 | (code >> 12)));
      out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
    return true;
  }

  bool parse_number(JsonValue& out, std::string& error) {
    const size_t start = pos_;
    if (text_[pos_] == '-') ++pos_;
    if (pos_ >= text_.size()) {
      error = "incomplete number";
      return false;
    }
    if (text_[pos_] == '0') {
      ++pos_;
    } else if (std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
      while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    } else {
      error = "invalid number";
      return false;
    }
    if (pos_ < text_.size() && text_[pos_] == '.') {
      ++pos_;
      if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
        error = "invalid number fraction";
        return false;
      }
      while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }
    if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
      ++pos_;
      if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
      if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
        error = "invalid number exponent";
        return false;
      }
      while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }

    out = JsonValue{};
    out.type = JsonValue::Type::Number;
    try {
      out.number = std::stod(text_.substr(start, pos_ - start));
    } catch (...) {
      error = "invalid number";
      return false;
    }
    return true;
  }

  bool parse_literal(const char* literal,
                     JsonValue::Type type,
                     JsonValue& out,
                     std::string& error,
                     bool boolean) {
    const size_t len = std::strlen(literal);
    if (text_.compare(pos_, len, literal) != 0) {
      error = "invalid literal";
      return false;
    }
    pos_ += len;
    out = JsonValue{};
    out.type = type;
    out.boolean = boolean;
    return true;
  }

  bool consume(char expected) {
    if (pos_ < text_.size() && text_[pos_] == expected) {
      ++pos_;
      return true;
    }
    return false;
  }

  void skip_ws() {
    while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
      ++pos_;
    }
  }

  std::string text_;
  size_t pos_{0};
};

bool parse_json_file(const std::string& path, JsonValue& out) {
  std::ifstream in(path);
  if (!in) {
    g_warning("Unable to open config file '%s': %s", path.c_str(), std::strerror(errno));
    return false;
  }

  std::ostringstream buffer;
  buffer << in.rdbuf();
  std::string error;
  JsonParser parser(buffer.str());
  if (!parser.parse(out, error)) {
    g_warning("Unable to parse JSON config file '%s': %s", path.c_str(), error.c_str());
    return false;
  }
  return true;
}

const JsonValue* find_child(const JsonValue& node, const char* key) {
  if (node.type != JsonValue::Type::Object) return nullptr;
  auto it = node.object.find(key);
  if (it == node.object.end()) return nullptr;
  return &it->second;
}

std::vector<std::string> read_args(const JsonValue& obj) {
  std::vector<std::string> args;
  const auto* args_node = find_child(obj, "args");
  if (!args_node || args_node->type != JsonValue::Type::Array) return args;
  args.reserve(args_node->array.size());
  for (const auto& node : args_node->array) {
    if (node.type != JsonValue::Type::String) continue;
    if (!node.string.empty()) {
      args.emplace_back(node.string);
    }
  }
  return args;
}

std::string get_string_member(const JsonValue& obj,
                              const char* key,
                              const char* fallback) {
  const auto* node = find_child(obj, key);
  if (node && node->type == JsonValue::Type::String) {
    return node->string;
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

std::vector<IconSpec> read_page(const JsonValue& arr,
                                std::unordered_map<std::string, std::string>& palette) {
  std::vector<IconSpec> out;
  if (arr.type != JsonValue::Type::Array) return out;

  out.reserve(arr.array.size());

  for (const auto& obj : arr.array) {
    if (obj.type != JsonValue::Type::Object) continue;

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

std::vector<std::vector<IconSpec>> read_pages(const JsonValue& root,
                                              std::unordered_map<std::string, std::string>& palette) {
  std::vector<std::vector<IconSpec>> pages;
  const auto* node = find_child(root, "pages");
  if (!node || node->type != JsonValue::Type::Array) return pages;

  pages.reserve(node->array.size());
  for (const auto& page_node : node->array) {
    auto page = read_page(page_node, palette);
    if (!page.empty()) {
      pages.emplace_back(std::move(page));
    }
  }
  return pages;
}

IconConfig default_icon_config() {
  IconConfig cfg;
  cfg.pages = {
    {
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
    },
    {
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
    },
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
  return RuntimeEnv::localAppDataDir() + "/sv-dashboard-gtk/sv-dashboard.json";
#else
  const char* cfg_dir = g_get_user_config_dir();
  return std::string(cfg_dir ? cfg_dir : ".") + "/sv-dashboard-gtk/sv-dashboard.json";
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
    paths.emplace_back(std::string(data_dirs[i]) + "/sv-dashboard-gtk/sv-dashboard.json");
  }
  const std::string bin_dir = exe_dir();
  paths.emplace_back(bin_dir + "/../share/sv-dashboard-gtk/sv-dashboard.json");
  paths.emplace_back(bin_dir + "/share/sv-dashboard-gtk/sv-dashboard.json");
  if (char* cwd = g_get_current_dir()) {
    paths.emplace_back(std::string(cwd) + "/assets/sv-dashboard.json");
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
  const std::string user_config = user_config_path();
  std::string config_path;
  std::string fallback;
  std::vector<std::string> fallback_reasons;

  if (!g_file_test(user_config.c_str(), G_FILE_TEST_EXISTS)) {
    fallback_reasons.push_back("User config missing at " + user_config);
    fallback = find_default_config_path();
    if (!fallback.empty()) {
      copy_default_config_if_missing(fallback, user_config);
    } else {
      fallback_reasons.push_back("No default config found in system data directories or bundled assets");
    }
  }

  if (env_path && *env_path) {
    if (g_file_test(env_path, G_FILE_TEST_EXISTS)) {
      config_path = env_path;
    } else if (g_file_test(user_config.c_str(), G_FILE_TEST_EXISTS)) {
      fallback_reasons.push_back("SV_DASHBOARD_CONFIG was set but file does not exist at " +
                                 std::string(env_path));
      config_path = user_config;
    } else if (fallback.empty()) {
      fallback_reasons.push_back("SV_DASHBOARD_CONFIG was set but file does not exist at " +
                                 std::string(env_path));
      fallback = find_default_config_path();
      if (!fallback.empty()) {
        config_path = fallback;
      } else {
        fallback_reasons.push_back("No default config found in system data directories or bundled assets");
      }
    } else {
      fallback_reasons.push_back("SV_DASHBOARD_CONFIG was set but file does not exist at " +
                                 std::string(env_path));
      config_path = fallback;
    }
  } else if (g_file_test(user_config.c_str(), G_FILE_TEST_EXISTS)) {
    config_path = user_config;
  } else if (!fallback.empty()) {
    config_path = fallback;
  }

  if (!g_file_test(config_path.c_str(), G_FILE_TEST_EXISTS)) {
    std::ostringstream reason_stream;
    for (size_t i = 0; i < fallback_reasons.size(); ++i) {
      if (i > 0) reason_stream << "; ";
      reason_stream << fallback_reasons[i];
    }
    std::string reasons = reason_stream.str();
    if (reasons.empty()) {
      reasons = "No config file found at the expected locations";
    }
    g_warning("Falling back to built-in defaults: %s.", reasons.c_str());
    return default_icon_config();
  }

  g_message("Loading config file: %s", config_path.c_str());

  JsonValue root;
  if (!parse_json_file(config_path, root)) {
    g_warning("Failed to read or parse config file '%s'; using built-in defaults.",
              config_path.c_str());
    return default_icon_config();
  }
  if (root.type != JsonValue::Type::Object) {
    g_warning("Config file '%s' did not contain a JSON object; using built-in defaults.",
              config_path.c_str());
    return default_icon_config();
  }

  std::unordered_map<std::string, std::string> palette_map;
  IconConfig cfg;
  cfg.pages = read_pages(root, palette_map);

  cfg.palette.reserve(palette_map.size());
  for (const auto& entry : palette_map) {
    cfg.palette.emplace_back(entry.first, entry.second);
  }

  if (cfg.pages.empty()) {
    g_warning("Config file '%s' produced no pages; using built-in defaults.", config_path.c_str());
    return default_icon_config();
  }

  return cfg;
}
