#include <glib.h>
#include <iostream>

#include <src/idle-aware.h>

using namespace std;

int main() {
  IdleAware proxy;
  proxy.brightnessChanged << [&] {
    cout << "Brightness: " << proxy.getBrightness() << endl;
  };
  proxy.connect() << [] {
    cout << "Connected" << endl;
  };
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
