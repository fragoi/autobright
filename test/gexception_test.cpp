#include <cassert>
#include <iostream>

#include <src/gexception.h>

using namespace std;

static const GQuark DOMAIN = g_quark_from_static_string("GException Test");

static void fill_gerror(GError **gerror) {
  *gerror = g_error_new(DOMAIN, 1, "A friendly test error");
}

static void unsafe_pattern() {
  GError *gerror = NULL;
  assert(!gerror);
  fill_gerror(&gerror);
  if (gerror)
    throw GException(gerror);
}

static void safe_pattern() {
  GException gerror;
  assert(!gerror);
  fill_gerror(gerror.get());
  if (gerror)
    throw gerror;
}

template<typename T>
static void test_throws(T fn) {
  try {
    fn();
    assert(false); // should have thrown
  } catch (GException &e) {
    /* OK */
    assert(e);
    assert(e->domain == DOMAIN);
    assert(e->code == 1);
  }
}

static void test_throw_null() {
  GException gerror;
  try {
    throw gerror;
    assert(false); // should have thrown
  } catch (GException &e) {
    assert(!e);
    assert(e.what());
  }
}

static void test_copy() {
  GException a;
  fill_gerror(a.get());
  GException b = a;
  assert(a != b);
  assert(strcmp(a->message, b->message) == 0);
}

static void test_move() {
  GException a;
  fill_gerror(a.get());
  GException b = move(a);
  assert(!a);
  assert(b);
  assert(b->message);
}

int main() {
  test_throws(unsafe_pattern);
  test_throws(safe_pattern);
  test_throw_null();
  test_copy();
  test_move();

  cout << "OK" << endl;
}
