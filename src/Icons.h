#pragma once
#include <vector>
#include <string>

// Day-theme icon square colors sampled from your screenshots (approx hex):
//   bg-azure      = #007ACC   (Freeboard, AvNav)
//   bg-blue       = #1976D2   (most blue tiles)
//   bg-teal       = #009688   (KIP, Instruments, SignalK)
//   bg-teal-light = #26A69A   (Power)
//   bg-cyan       = #06B6D4   (PyPilot)
//   bg-indigo     = #5C6BC0   (Sky, Windy, Drones)
//   bg-gray       = #455A64   (Camera, Web Cam)
//   bg-slate      = #556F7B   (Provisioning, Files)
//   bg-slate-dark = #546E7A   (Terminal, Tasks, Commands)
//   bg-purple     = #8E24AA   (Radio)
//   bg-violet     = #7E22CE   (T-Storms)
//   bg-red        = #DC2626   (Music, Video)
//
// Dusk/Night themes ignore these and render monochrome (gray or red).

struct IconSpec {
  char32_t codepoint{};
  std::string label;
  bool isBrand{false};

  // NEW: Day theme background class for the rounded square behind the glyph.
  // (Used by CSS, e.g. ".tile-icon-box.bg-blue { background: #1976D2; }")
  const char* colorClass{nullptr};
};

inline constexpr int kCols = 5;
inline constexpr int kRows = 3;

// PAGE1 (screenshot with Freeboard/Sky/Moorings/...)
inline const std::vector<IconSpec> PAGE1 = {
  { U'\uf5a0', "Freeboard",     false, "bg-azure"      }, // #007ACC
  { U'\uf005', "Sky",           false, "bg-indigo"     }, // #5C6BC0
  { U'\uf13d', "Moorings",      false, "bg-blue"       }, // #1976D2
  { U'\uf2e7', "Provisioning",  false, "bg-slate"      }, // #556F7B
  { U'\uf5a0', "AvNav",         false, "bg-azure"      }, // #007ACC

  { U'\uf6f2', "Vessel",        false, "bg-blue"       }, // #1976D2
  { U'\uf013', "SignalK",       false, "bg-teal"       }, // #009688
  { U'\uf120', "Terminal",      false, "bg-slate-dark" }, // #546E7A
  { U'\uf0ae', "Tasks",         false, "bg-slate-dark" }, // #546E7A
  { U'\uf07b', "Files",         false, "bg-slate"      }, // #556F7B

  { U'\uf8d7', "Radio",         false, "bg-purple"     }, // #8E24AA
  { U'\uf072', "Drones",        false, "bg-indigo"     }, // #5C6BC0
  { U'\uf030', "Web Cam",       false, "bg-gray"       }, // #455A64
  { U'\uf39f', "Messenger",     true,  "bg-blue"       }, // #1976D2
  { U'\uf39e', "Social",        true,  "bg-blue"       }, // #1976D2
};

// PAGE2 (screenshot with OpenCPN/KIP/Power/GRIB/Camera/...)
inline const std::vector<IconSpec> PAGE2 = {
  { U'\uf5a0', "OpenCPN",       false, "bg-blue"       }, // #1976D2
  { U'\uf624', "KIP",           false, "bg-teal"       }, // #009688
  { U'\uf5ba', "Power",         false, "bg-teal-light" }, // #26A69A
  { U'\uf743', "GRIB",          false, "bg-blue"       }, // #1976D2
  { U'\uf030', "Camera",        false, "bg-gray"       }, // #455A64

  { U'\uf5a0', "qtVlm",         false, "bg-blue"       }, // #1976D2
  { U'\uf624', "Instruments",   false, "bg-teal"       }, // #009688
  { U'\uf1d8', "PyPilot",       false, "bg-cyan"       }, // #06B6D4
  { U'\uf72e', "Windy",         false, "bg-indigo"     }, // #5C6BC0
  { U'\uf0e0', "Email",         false, "bg-blue"       }, // #1976D2

  { U'\uf001', "Music",         false, "bg-red"        }, // #DC2626
  { U'\uf167', "Video",         true,  "bg-red"        }, // #DC2626
  { U'\uf011', "Commands",      false, "bg-slate-dark" }, // #546E7A
  { U'\uf76c', "T-Storms",      false, "bg-violet"     }, // #7E22CE
  { U'\uf268', "Chrome",        true,  "bg-blue"       }, // #1976D2
};

// Nav chevrons
inline constexpr char32_t CHEV_LEFT  = U'\uf053';
inline constexpr char32_t CHEV_RIGHT = U'\uf054';
