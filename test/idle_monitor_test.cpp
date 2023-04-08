#include <glib.h>
#include <iostream>

#include <src/idle-monitor.h>

using namespace std;

static void onIdleFunc() {
  cout << "I'm a static function and I'm idle" << endl;
}

int main() {
  IdleMonitorProxy proxy;

  auto onActive = [] {
    cout << "Active" << endl;
  };

  auto onIdle = [&] {
    cout << "Idle" << endl;
    proxy.addUserActiveWatch(onActive);
  };

  proxy.connect() << [&] {
    proxy.addIdleWatch(5000, onIdle);
    proxy.addIdleWatch(6000, &onIdleFunc);
  };

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
