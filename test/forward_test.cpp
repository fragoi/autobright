#include <cassert>
#include <iostream>
#include <utility>
#include <type_traits>

using namespace std;

struct Foo {
    static int constructed;
    static int copied;
    static int moved;
    static int destroyed;

    static int all_constructed() {
      return constructed + copied;
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

template<typename T>
static void fn0(T foo, int copied, int moved) {
  assert(Foo::copied == copied);
  assert(Foo::moved == moved);
}

template<typename T, typename ...Args>
static void fn1(Args ...args) {
  fn0<T>(forward<Args>(args)...);
}

static void test_copy() {
  Foo foo;
  fn1<Foo&&>(foo, Foo::copied + 1, Foo::moved);
}

static void test_move() {
  Foo foo;
  fn1<Foo&&>(move(foo), Foo::copied, Foo::moved + 1);
}

static void test_rval() {
  fn1<Foo&&>(Foo(), Foo::copied, Foo::moved);
}

static void test_ptr() {
  Foo *foo = new Foo;
  fn1<Foo*>(foo, Foo::copied, Foo::moved);
  delete foo;
}

static void test_ref() {
  Foo foo;
  fn1<Foo&, Foo&>(foo, Foo::copied, Foo::moved);
}

static void test_const_ref_copy() {
  Foo foo;
  fn1<const Foo&>(foo, Foo::copied + 1, Foo::moved);
}

static void test_const_ref_move() {
  Foo foo;
  fn1<const Foo&>(move(foo), Foo::copied, Foo::moved + 1);
}

static void test_const_ref_rval() {
  fn1<const Foo&>(Foo(), Foo::copied, Foo::moved);
}

template<typename T>
class Future {
  private:
    T *value = nullptr;

  public:
    ~Future() {
      delete value;
    }

    void set(const T &value, int copied, int moved) {
      this->value = new T(value);
      assert(Foo::copied == copied);
      assert(Foo::moved == moved);
    }

    void set(T &&value, int copied, int moved) {
      this->value = new T(move(value));
      assert(Foo::copied == copied);
      assert(Foo::moved == moved);
    }
};

template<>
class Future<void> {
  public:
    void set() {
    }
};

template<typename T, typename ...Args>
static void fnF(Args ...args) {
  Future<T> future;
  future.set(forward<Args>(args)...);
}

static void test_F_copy() {
  Foo foo;
  fnF<Foo>(foo, Foo::copied + 1, Foo::moved + 1);
}

static void test_F_move() {
  Foo foo;
  fnF<Foo>(move(foo), Foo::copied, Foo::moved + 2);
}

static void test_F_rval() {
  fnF<Foo>(Foo(), Foo::copied, Foo::moved + 1);
}

static void test_F_ptr() {
  Foo *foo = new Foo;
  fnF<Foo*>(foo, Foo::copied, Foo::moved);
  delete foo;
}

static void test_F_ref() {
  Foo foo;
  fnF<Foo, Foo&>(foo, Foo::copied + 1, Foo::moved);
}

static void test_F_const_ref_copy() {
  Foo foo;
  fnF<Foo, const Foo&>(foo, Foo::copied + 1, Foo::moved);
}

static void test_F_const_ref_move() {
  Foo foo;
  fnF<Foo, const Foo&>(move(foo), Foo::copied + 1, Foo::moved);
}

static void test_F_const_ref_rval() {
  fnF<Foo, const Foo&>(Foo(), Foo::copied + 1, Foo::moved);
}

static void test_F_void() {
  fnF<void>();
}

int main() {
  test_copy();
  test_move();
  test_rval();
  test_ptr();
  test_ref();
  test_const_ref_copy();
  test_const_ref_move();
  test_const_ref_rval();

  test_F_copy();
  test_F_move();
  test_F_rval();
  test_F_ptr();
  test_F_ref();
  test_F_const_ref_copy();
  test_F_const_ref_move();
  test_F_const_ref_rval();

  test_F_void();

  cout << "OK" << endl;
}
