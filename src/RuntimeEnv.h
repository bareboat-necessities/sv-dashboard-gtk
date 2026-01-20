#pragma once
#include <string>

struct RuntimeEnv {
  static std::string toForwardSlashes(std::string s);
  static void ensureDir(const std::string& path);

  static void setEnv(const char* key, const std::string& val);
  static void unsetEnv(const char* key);

  static std::string exeDir();

#ifdef _WIN32
  static std::string localAppDataDir();
#endif

  static void setup();
};
