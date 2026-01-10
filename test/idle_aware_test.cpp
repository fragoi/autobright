#include <glib.h>
#include <cassert>
#include <iostream>

#include <src/idle-aware.h>
#include <src/logger.h>

using namespace std;

int main() {
  bool emitted = false;
  bool cannotConnect = false;
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  IdleAware proxy;

  proxy.brightnessChanged << [&] {
    cout << "Brightness: " << proxy.getBrightness() << endl;
    emitted = true;
  };

  auto promise = IdleAware::pingService() << [&] {
    return proxy.connect();
  };

  promise.then([] {
    cout << "Connected" << endl;
  }, [&](exception_ptr ex) {
    cerr << "Cannot connect: " << ex << endl;
    cannotConnect = true;
  }) << [&] {
    g_main_loop_quit(loop);
  };

  g_main_loop_run(loop);

  if (cannotConnect) {
    /* skipped */
    return 77;
  }

  assert(emitted);

  cout << "OK" << endl;
}
