#include <cassert>
#include <iostream>

#include <src/promise.h>

using namespace std;
using namespace promise;

struct Functor {
    static int constructed;
    static int copied;
    static int moved;
    static int destroyed;

    static int all_constructed() {
      return constructed + copied + moved;
    }

    bool resolved = false;
    bool rejected = false;

    Functor() {
      constructed++;
    }

    Functor(const Functor &other) {
      copied++;
    }

    Functor(Functor &&other) {
      moved++;
    }

    ~Functor() {
      destroyed++;
    }

    void operator()() {
      resolved = true;
    }

    void operator()(exception_ptr exception) {
      rejected = true;
    }
};

int Functor::constructed = 0;
int Functor::copied = 0;
int Functor::moved = 0;
int Functor::destroyed = 0;

#define DECLARE() \
  int __constructed = Functor::constructed; \
  int __copied = Functor::copied; \
  int __moved = Functor::moved; \
  int __destroyed = Functor::destroyed

#define ASSERTS(CONSTRUCTED, COPIED, MOVED, DESTROYED) \
  assert(Functor::constructed == __constructed + CONSTRUCTED); \
  assert(Functor::copied == __copied + COPIED); \
  assert(Functor::moved == __moved + MOVED); \
  assert(Functor::destroyed == __destroyed + DESTROYED)

static void test_then_lvalue() {
  DECLARE();

  {
    Result<void> r;
    Promise<void> p = r;

    Functor f;
    p.then(f);
  }

  ASSERTS(1, 1, 0, 2);
}

static void test_ref() {
  Result<void> r;
  Promise<void> p = r;

  Functor f;
  p.then(f);

  r.resolve();

  assert(!f.resolved);
}

static void test_lambda() {
  Result<void> r;
  Promise<void> p = r;

  Functor f;
  auto l = [f] {
  };

  p.then(l);
}

int main() {
  test_then_lvalue();
  test_ref();
  test_lambda();

  cout << "OK" << endl;
}
