#include <glib.h>
#include <iostream>

#include <src/idle-monitor.h>

using namespace std;

static void doNothing() {
}

int main() {
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  exception_ptr exception;

  IdleMonitorProxy proxy;

  auto promise = proxy.connect() << [&] {
    return proxy.addIdleWatch(1000, &doNothing) << [](void *id) {
      cout << "Added idle watch: " << id << endl;
    };
  } << [&] {
    return proxy.addIdleWatch(2000, &doNothing) << [](void *id) {
      cout << "Added idle watch: " << id << endl;
    };
  } << [&] {
    return proxy.addUserActiveWatch(&doNothing) << [](void *id) {
      cout << "Added user active watch: " << id << endl;
    };
  } << [&] {
    return proxy.refreshAll() << [] {
      cout << "Refreshed" << endl;
    };
  } << [&] {
    cout << "Bye" << endl;
    g_main_loop_quit(loop);
  };

  promise.grab([&](exception_ptr ex) {
    exception = ex;
    g_main_loop_quit(loop);
  });

  g_main_loop_run(loop);

  if (exception)
    rethrow_exception(exception);

  cout << "OK" << endl;
}
