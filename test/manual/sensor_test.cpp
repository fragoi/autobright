#include <glib.h>
#include <iostream>

#include <src/sensor.h>

using namespace std;

int main() {
  SensorProxy proxy;
  proxy.lightLevelChanged << [&] {
    cout << "Light level: " << proxy.getLightLevel() << endl;
  };
  proxy.connect() << [] {
    cout << "Connected" << endl;
  };
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
