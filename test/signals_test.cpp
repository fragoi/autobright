#include <cassert>
#include <iostream>

#include <src/signals.h>

using namespace std;
using namespace signals;

struct DtorHandler {
    bool &destoyed;

    ~DtorHandler() {
      destoyed = true;
    }

    void operator()() {
    }
};

struct EmitHandler {
    int count = 0;

    void operator()() {
      count++;
    }
};

struct SuicideHandler {
    Signal<void()> &signal;
    bool &dead;

    int count = 0;

    void operator()() {
      count++;
      if (count == 2) {
        signal >> *this;
        dead = true;
      }
    }
};

static void test_empty_emit() {
  Signal<void()> signal;
  signal();
}

static void test_add_and_emit() {
  Signal<void()> signal;

  EmitHandler handler;

  signal << handler;

  assert(handler.count == 0);

  signal();

  assert(handler.count == 1);
}

static void test_remove_by_ptr() {
  Signal<void()> signal;

  bool destroyed = false;
  auto p = signal << DtorHandler { destroyed };

  assert(p);

  signal >> p;

  assert(destroyed);
}

static void test_remove_by_ref() {
  Signal<void()> signal;

  bool destroyed = false;
  {
    DtorHandler handler { destroyed };

    auto p = signal << handler;

    assert(p);

    signal >> handler;
  }

  assert(destroyed);
}

static void test_emit_and_remove() {
  Signal<void()> signal;

  EmitHandler handler;

  auto p = signal << handler;

  signal();

  signal >> p;

  signal();

  assert(handler.count == 1);
}

static void test_suicide() {
  Signal<void()> signal;

  bool dead = false;
  signal << SuicideHandler { signal, dead };

  signal();
  signal();

  assert(dead);
}

static void test_suicide_in_chain() {
  Signal<void()> signal;

  bool dead = false;
  EmitHandler h1;
  EmitHandler h2;

  signal << h1;
  signal << SuicideHandler { signal, dead };
  signal << h2;

  signal();
  signal();

  assert(dead);
  assert(h1.count == 2);
  assert(h2.count == 2);
}

int main() {
  test_empty_emit();
  test_add_and_emit();
  test_remove_by_ptr();
  test_remove_by_ref();
  test_emit_and_remove();
  test_suicide();
  test_suicide_in_chain();

  cout << "OK" << endl;
}
