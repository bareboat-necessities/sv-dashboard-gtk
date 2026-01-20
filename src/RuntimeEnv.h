#pragma once
#include <string>

class RuntimeEnv {
public:
  // Call at the VERY start of main(), before Gtk::Application::create()
  static void setup();

private:
  static std::string exeDir();
  static void setEnv(const char* key, const std::string& val);
  static void ensureDir(const std::string& path);
  static std::string toForwardSlashes(std::string s);

#ifdef _WIN32
  static std::string localAppDataDir();
#endif
};
