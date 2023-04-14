#include <glib.h>
#include <cassert>
#include <iostream>

#include <src/sensor.h>

using namespace std;

int main() {
  bool emitted = false;
  exception_ptr exception;
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  SensorProxy proxy;

  proxy.lightLevelChanged << [&] {
    cout << "Light level: " << proxy.getLightLevel() << endl;
    emitted = true;
  };

  proxy.connect().then([] {
    cout << "Connected" << endl;
  }, [&](exception_ptr ex) {
    exception = ex;
  }) << [&] {
    g_main_loop_quit(loop);
  };

  g_main_loop_run(loop);

  if (exception)
    rethrow_exception(exception);

  assert(emitted);

  cout << "OK" << endl;
}
