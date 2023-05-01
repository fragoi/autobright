#include <cassert>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include <string>

#include <src/retry.h>
#include <src/logger.h>

using namespace std;
using namespace promise;
using namespace retry;

static Promise<void> test_retry_void(int &times) {
  return retry::retry([&] {
    cout << "Retry called: " << times << endl;
    if (times > 0) {
      times--;
      return rejected<void>(runtime_error("rejecting"));
    } else {
      return resolved();
    }
  });
}

static Promise<int> test_retry_int(int &times) {
  return retry::retry([&] {
    cout << "Retry called: " << times << endl;
    if (times > 0) {
      times--;
      return rejected<int>(runtime_error("rejecting"));
    } else {
      return resolved(1);
    }
  });
}

static void quitLoop(GMainLoop **loop) {
  g_main_loop_quit(*loop);
  g_main_loop_unref(*loop);
  *loop = nullptr;
}

template<typename T>
static void runLoop(Promise<T> promise, int &resolved, int &rejected) {
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  promise.then(
      [&](...) {
        resolved++;
        cout << "Resolved " << resolved << endl;
        quitLoop(&loop);
      },
      [&](exception_ptr ex) {
        rejected++;
        cout << "Rejected " << rejected << ": " << ex << endl;
        quitLoop(&loop);
      });
  if (loop)
    g_main_loop_run(loop);
}

static void test_void() {
  int times;
  int resolved = 0;
  int rejected = 0;

  for (int i = 0; i < 5; ++i) {
    times = i;
    runLoop(test_retry_void(times), resolved, rejected);
  }

  assert(resolved == 4);
  assert(rejected == 1);
}

static void test_int() {
  int times;
  int resolved = 0;
  int rejected = 0;

  for (int i = 0; i < 5; ++i) {
    times = i;
    runLoop(test_retry_int(times), resolved, rejected);
  }

  assert(resolved == 4);
  assert(rejected == 1);
}

int main() {
  test_void();
  test_int();
}
