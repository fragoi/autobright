#include <glib.h>
#include <iostream>

#include <src/idle-monitor.h>

using namespace std;

static void doNothing() {
}

int main() {
  bool cannotConnect = false;
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  exception_ptr exception;

  IdleMonitorProxy proxy;

  auto promise = IdleMonitorProxy::pingService() << [&] {
    return proxy.connect();
  };

  promise.grab([&](exception_ptr ex) {
    cerr << "Cannot connect: " << ex << endl;
    cannotConnect = true;
  });

  promise = promise << [&] {
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
  };

  promise.then([] {
    cout << "Done" << endl;
  }, [&](exception_ptr ex) {
    exception = ex;
  }) << [&] {
    g_main_loop_quit(loop);
  };

  g_main_loop_run(loop);

  if (cannotConnect) {
    /* skipped */
    return 77;
  } else if (exception) {
    rethrow_exception(exception);
  }

  cout << "OK" << endl;
}
