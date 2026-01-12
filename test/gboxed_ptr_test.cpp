#include <cassert>
#include <iostream>
#include <functional>
#include <vector>

#include <src/gobjectmm.h>

using namespace std;

#define NEW_GOBJECT G_OBJECT(g_object_new(g_object_get_type(), NULL))

static bool pass_by_val(gboxed_ptr<GObject> ptr, guint expected) {
  return ptr->ref_count == expected;
}

static void unref(GObject *o) {
  g_object_unref(o);
}

static void test_get() {
  GObject *o = NEW_GOBJECT;
  gboxed_ptr<GObject> ptr(o, unref);

  assert(ptr.get() == o);
}

static void test_arrow() {
  GObject *o = NEW_GOBJECT;
  gboxed_ptr<GObject> ptr(o, unref);

  assert(ptr->ref_count == 1);
  assert(o->ref_count == 1);
}

static void test_move() {
  GObject *o = NEW_GOBJECT;

  assert(o->ref_count == 1);

  assert(pass_by_val(gboxed_ptr<GObject>(o, unref), 1));
}

static void test_null() {
  gboxed_ptr<GObject> ptr(NULL, unref);

  assert(ptr.get() == NULL);
  assert(!ptr);

  try {
    gboxed_ptr<GObject> ptr2(NEW_GOBJECT, NULL);
    assert(false); // should have throw exception
  } catch (...) {
    /* OK */
  }
}

static void test_bool() {
  gboxed_ptr<GObject> a(NEW_GOBJECT, unref);

  assert(a);

  gboxed_ptr<GObject> b(NULL, unref);

  assert(!b);
}

static void test_swap() {
  struct Data {
      using FSignature = void(*)(Data*);
      FSignature value;
  };

  static Data::FSignature f1 = [](Data *data) {
    assert(data->value == f1);
    delete data;
  };

  static Data::FSignature f2 = [](Data *data) {
    assert(data->value == f2);
    delete data;
  };

  Data *d1 = new Data { f1 };
  Data *d2 = new Data { f2 };

  gboxed_ptr<Data> ptr1(d1, f1);
  gboxed_ptr<Data> ptr2(d2, f2);

  assert(ptr1.get() == d1);
  assert(ptr2.get() == d2);

  swap(ptr1, ptr2);

  assert(ptr1.get() == d2);
  assert(ptr2.get() == d1);
}

int main() {
  test_get();
  test_arrow();
  test_move();
  test_null();
  test_bool();
  test_swap();

  cout << "OK" << endl;
}
