#include <glib.h>
#include <iostream>

#include <src/idle-aware.h>

using namespace std;

int main() {
  IdleAware idle;

  idle.brightnessChanged << [&] {
    cout << "Brightness: " << idle.getBrightness() << endl;
  };

  idle.connect();

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
