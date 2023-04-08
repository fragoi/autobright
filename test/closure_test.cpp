#include <cassert>
#include <iostream>
#include <thread>

#include <src/closure.h>

using namespace std;
using namespace closure;

static thread runThread(void (*callback)(void*), void *data) {
  return thread([=] {
    callback(data);
  });
}

int voidFunctionCalls = 0;

static void voidFunction() {
  voidFunctionCalls++;
}

static int sumFunction(int a, int b) {
  return a + b;
}

struct Foo {
    static int constructed;
    static int copied;
    static int moved;
    static int destroyed;

    static int all_constructed() {
      return constructed + copied + moved;
    }

    Foo() {
      constructed++;
    }

    Foo(const Foo &other) {
      copied++;
    }

    Foo(Foo &&other) {
      moved++;
    }

    ~Foo() {
      destroyed++;
    }
};

int Foo::constructed = 0;
int Foo::copied = 0;
int Foo::moved = 0;
int Foo::destroyed = 0;

static void test_void_function() {
  int prevCalls = voidFunctionCalls;

  Closure<void()> c = voidFunction;

  assert(voidFunctionCalls == prevCalls);
  c();
  assert(voidFunctionCalls == prevCalls + 1);
}

static void test_void_noncapture_lambda() {
  static int lambdaCalls = 0;
  int prevCalls = lambdaCalls;

  Closure<void()> c = [] {
    lambdaCalls++;
  };

  assert(lambdaCalls == prevCalls);
  c();
  assert(lambdaCalls == prevCalls + 1);
}

static void test_void_capture_lambda() {
  bool called = false;

  Closure<void()> c = [&called] {
    called = true;
  };

  assert(!called);
  c();
  assert(called);
}

static void test_nonvoid_function() {
  Closure<int(int, int)> c = sumFunction;

  int sum = c(1, 2);
  assert(sum == 3);
}

static void test_nonvoid_noncapture_lambda() {
  Closure<int(int, int)> c = [](int a, int b) {
    return a + b;
  };

  int sum = c(1, 2);
  assert(sum == 3);
}

static void test_nonvoid_capture_lambda() {
  bool called = false;

  Closure<int(int, int)> c = [&called](int a, int b) {
    called = true;
    return a + b;
  };

  int sum = c(1, 2);
  assert(sum == 3);
  assert(called);
}

static void test_foo_val_arg() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;

  Foo foo;

  Closure<void(Foo&)> c = [](Foo foo) {
  };

  c(foo);

  assert(Foo::constructed == constructed + 1);
  assert(Foo::copied == copied + 1);
  assert(Foo::moved == moved);
  assert(Foo::destroyed == destroyed + 1);
}

static void test_foo_const_ref_arg() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;

  Foo foo;

  Closure<void(Foo&)> c = [](const Foo &foo) {
  };

  c(foo);

  assert(Foo::constructed == constructed + 1);
  assert(Foo::copied == copied);
  assert(Foo::moved == moved);
  assert(Foo::destroyed == destroyed);
}

static void test_foo_ref_arg() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;

  Foo foo;

  Closure<void(Foo&)> c = [](Foo &foo) {
  };

  c(foo);

  assert(Foo::constructed == constructed + 1);
  assert(Foo::copied == copied);
  assert(Foo::moved == moved);
  assert(Foo::destroyed == destroyed);
}

static void test_foo_ptr_arg() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;

  Foo *foo = new Foo;

  Closure<void(Foo*)> c = [](Foo *foo) {
  };

  c(foo);

  delete foo;

  assert(Foo::constructed == constructed + 1);
  assert(Foo::copied == copied);
  assert(Foo::moved == moved);
  assert(Foo::destroyed == destroyed + 1);
}

static void test_foo_ret() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;

  Closure<Foo(void)> c = [] {
    return Foo();
  };

  Foo foo = c();

  assert(Foo::constructed == constructed + 1);
  assert(Foo::copied == copied);
  assert(Foo::moved == moved);
  assert(Foo::destroyed == destroyed);
}

static void test_ref_arg() {
  bool b = false;

  Closure<void(bool&)> c = [](bool &b) {
    b = true;
  };

  c(b);

  assert(b);
}

static void test_callback() {
  int constructed = Foo::constructed;
  int copied = Foo::copied;
  int moved = Foo::moved;
  int destroyed = Foo::destroyed;
  bool b = false;

  thread t;
  {
    Foo foo;
    Closure<void()> c = [foo, &b] {
      b = true;
    };
    t = runThread(c.callback(), c.detach());
  }
  t.join();

  assert(Foo::all_constructed() == Foo::destroyed);
  assert(Foo::constructed == constructed + 1);
  /* lambda */
  assert(Foo::copied == copied + 1);
  /* closure */
  assert(Foo::moved == moved + 1);
  assert(Foo::destroyed == destroyed + 3);
  assert(b);
}

int main() {
  test_void_function();
  test_void_noncapture_lambda();
  test_void_capture_lambda();

  test_nonvoid_function();
  test_nonvoid_noncapture_lambda();
  test_nonvoid_capture_lambda();

  test_foo_val_arg();
  test_foo_const_ref_arg();
  test_foo_ref_arg();
  test_foo_ptr_arg();
  test_foo_ret();
  test_ref_arg();

  test_callback();

  cout << "OK" << endl;
}
