#include <cassert>
#include <iostream>

#include <src/signals.h>

using namespace std;
using namespace signals;

struct SomeClass {
    Signal<void()> foo;
    Signal<int(double)> bar;
};

static void sfoo() {

}

static int sbar(double d) {
  return 1;
}

int main() {
  bool b;
  SomeClass sc;

  sc.foo << sfoo;

  auto id1 = sc.foo << [] {
  };

  sc.foo << [&b] {
    b = true;
  };

  sc.foo();

  sc.bar << sbar;

  sc.bar << [](double d) {
    return 1;
  };

  sc.bar << [&b](double d) {
    b = true;
    return 1;
  };

  sc.bar(10);

  sc.foo >> id1 >> sfoo;

  cout << "OK" << endl;
}
