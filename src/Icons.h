#pragma once
#include <vector>
#include <string>

struct IconSpec {
  char32_t codepoint{};
  std::string label;
  bool isBrand{false};
};

inline constexpr int kCols = 5;
inline constexpr int kRows = 3;

inline const std::vector<IconSpec> PAGE1 = {
  { U'\uf5a0', "Freeboard", false },   // map-marked-alt / map-location-ish
  { U'\uf005', "Sky", false },         // star
  { U'\uf13d', "Moorings", false },    // anchor
  { U'\uf2e7', "Provisioning", false },// utensils
  { U'\uf5a0', "AvNav", false },       // map

  { U'\uf6f2', "Vessel", false },      // sailboat (may vary in some FA6 builds)
  { U'\uf013', "SignalK", false },     // gear
  { U'\uf120', "Terminal", false },    // terminal
  { U'\uf0ae', "Tasks", false },       // tasks
  { U'\uf07b', "Files", false },       // folder

  { U'\uf8d7', "Radio", false },       // radio (some builds may differ)
  { U'\uf072', "Drones", false },      // plane
  { U'\uf030', "Web Cam", false },     // camera
  { U'\uf39f', "Messenger", true },    // fb messenger (brands)
  { U'\uf39e', "Social", true },       // facebook-f (brands)
};

inline const std::vector<IconSpec> PAGE2 = {
  { U'\uf5a0', "OpenCPN", false },
  { U'\uf624', "KIP", false },         // gauge
  { U'\uf5ba', "Power", false },       // solar-panel
  { U'\uf743', "GRIB", false },        // cloud-sun-rain
  { U'\uf030', "Camera", false },

  { U'\uf5a0', "qtVlm", false },
  { U'\uf624', "Instruments", false },
  { U'\uf1d8', "PyPilot", false },     // paper-plane
  { U'\uf72e', "Windy", false },       // wind
  { U'\uf0e0', "Email", false },       // envelope

  { U'\uf001', "Music", false },       // music
  { U'\uf167', "Video", true },        // youtube (brands)
  { U'\uf011', "Commands", false },    // power-off
  { U'\uf76c', "T-Storms", false },    // cloud-bolt (may vary)
  { U'\uf268', "Chrome", true },       // chrome (brands)
};

// Nav chevrons
inline constexpr char32_t CHEV_LEFT  = U'\uf053';
inline constexpr char32_t CHEV_RIGHT = U'\uf054';
