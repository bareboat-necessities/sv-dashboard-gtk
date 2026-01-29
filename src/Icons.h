#pragma once
#include <string>
#include <utility>
#include <vector>

struct IconSpec {
  char32_t codepoint{};
  std::string label;
  bool isBrand{false};
  std::string colorClass;
  std::string command;
  std::vector<std::string> args;
};

struct IconConfig {
  std::vector<IconSpec> page1;
  std::vector<IconSpec> page2;
  std::vector<std::pair<std::string, std::string>> palette;
};

IconConfig load_icon_config();

inline constexpr int kCols = 5;
inline constexpr int kRows = 3;

// Nav chevrons
inline constexpr char32_t CHEV_LEFT  = U'\uf053';
inline constexpr char32_t CHEV_RIGHT = U'\uf054';
