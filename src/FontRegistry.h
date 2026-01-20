#pragma once
#include <string>

class FontRegistry {
public:
  // Registers FA6 fonts via fontconfig. Returns true on success.
  bool registerBundledFonts();

  // Optional: allow overriding the font dir (e.g. for packaging/tests)
  void setFontDirOverride(std::string dir);

  // Common families (Pango names) used by FA6.
  static constexpr const char* kFamilyFree   = "Font Awesome 6 Free";
  static constexpr const char* kFamilyBrands = "Font Awesome 6 Brands";

private:
  std::string font_dir_override_;

  std::string findFontDir() const;
  static std::string exeDir();
};
