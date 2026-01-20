#pragma once

class RuntimeEnv {
public:
  // Safe no-op on non-Windows.
  static void setup();
};
