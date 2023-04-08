#include <cassert>
#include <iostream>

#include <src/gobjectmm.h>

using namespace std;

#define NEW_GOBJECT G_OBJECT(g_object_new(g_object_get_type(), NULL))

static bool pass_by_val(gobject_ptr<GObject> ptr, guint expected) {
  return ptr.use_count() == expected;
}

static bool pass_by_ref(gobject_ptr<GObject> &ptr, guint expected) {
  return ptr.use_count() == expected;
}

static bool pass_by_ptr(gobject_ptr<GObject> *ptr, guint expected) {
  return (*ptr).use_count() == expected;
}

static void test_get() {
  GObject *o = NEW_GOBJECT;
  gobject_ptr<GObject> ptr(o);

  assert(ptr.get() == o);
}

static void test_arrow() {
  GObject *o = NEW_GOBJECT;
  gobject_ptr<GObject> ptr(o);

  assert(ptr->ref_count == 1);
  assert(o->ref_count == 1);
}

static void test_ref_count() {
  gobject_ptr<GObject> ptr(NEW_GOBJECT);

  assert(ptr.use_count() == 1);

  assert(pass_by_val(ptr, 2));
  assert(pass_by_ref(ptr, 1));
  assert(pass_by_ptr(&ptr, 1));

  assert(ptr.use_count() == 1);

  gobject_ptr<GObject> ptr2 = ptr;

  assert(ptr.use_count() == 2);
  assert(ptr2.use_count() == 2);

  assert(pass_by_val(ptr2, 3));
  assert(pass_by_ref(ptr2, 2));
  assert(pass_by_ptr(&ptr2, 2));

  assert(ptr.use_count() == 2);
  assert(ptr2.use_count() == 2);
}

static void test_finalize() {
  GObject *o = NEW_GOBJECT;

  assert(G_IS_OBJECT(o));

  /* pointer scope */
  {
    gobject_ptr<GObject> ptr(o);

    assert(ptr.use_count() == 1);

    assert(pass_by_val(ptr, 2));

    assert(ptr.use_count() == 1);
  }

  assert(!G_IS_OBJECT(o));
}

static void test_move() {
  GObject *o = NEW_GOBJECT;

  assert(o->ref_count == 1);

  assert(pass_by_val(gobject_ptr<GObject>(o), 1));

  assert(!G_IS_OBJECT(o));
}

static void test_assign() {
  gobject_ptr<GObject> a(NEW_GOBJECT);
  gobject_ptr<GObject> b = a;

  assert(a.use_count() == 2);
  assert(b.use_count() == 2);

  a = b;

  assert(a.use_count() == 2);
  assert(b.use_count() == 2);

  b = a;

  assert(a.use_count() == 2);
  assert(b.use_count() == 2);

  a = a; // @suppress("Assignment to itself")

  assert(a.use_count() == 2);
  assert(b.use_count() == 2);

  assert(a.get() == b.get());
}

static void test_null() {
  gobject_ptr<GObject> ptr;

  assert(ptr.get() == NULL);
  assert(!ptr);

  gobject_ptr<GObject> ptr2 = ptr;

  assert(ptr2.get() == NULL);
  assert(!ptr2);

  ptr = ptr2;

  assert(!ptr);
  assert(!ptr2);

  assert(ptr.get() == ptr2.get());
}

static void test_bool() {
  gobject_ptr<GObject> a(NEW_GOBJECT);

  assert(a);

  gobject_ptr<GObject> b;

  assert(!b);
}

static void test_swap() {
  GObject *a = NEW_GOBJECT;
  GObject *b = NEW_GOBJECT;
  gobject_ptr<GObject> ptrA(a);
  gobject_ptr<GObject> ptrB(b);

  assert(ptrA.get() == a);
  assert(ptrB.get() == b);

  swap(ptrA, ptrB);

  assert(ptrA.get() == b);
  assert(ptrB.get() == a);
}

int main() {
  test_get();
  test_arrow();
  test_ref_count();
  test_finalize();
  test_move();
  test_assign();
  test_null();
  test_bool();
  test_swap();

  cout << "OK" << endl;
}
