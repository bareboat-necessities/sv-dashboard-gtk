#include "MainApp.h"

int main(int argc, char** argv) {
  auto app = MainApp::create();
  return app->run(argc, argv);
}
