#include <glib.h>
#include <cassert>
#include <iostream>

#include <src/idle-aware.h>

using namespace std;

int main() {
  bool emitted = false;
  exception_ptr exception;
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  IdleAware proxy;

  proxy.brightnessChanged << [&] {
    cout << "Brightness: " << proxy.getBrightness() << endl;
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
