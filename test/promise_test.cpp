#include <cassert>
#include <iostream>
#include <thread>
#include <string.h>
#include <list>

#include <src/promise.h>

using namespace std;
using namespace promise;

int executorFunctionCalls = 0;
static void executorFunction(Result<void> result) {
  executorFunctionCalls++;
  result.resolve();
}

int thenFunctionCalls = 0;
static void thenFunction() {
  thenFunctionCalls++;
}

int grabFunctionCalls = 0;
static void grabFunction(exception_ptr e) {
  grabFunctionCalls++;
}

static runtime_error newException() {
  return runtime_error("Promise Test");
}

struct Functor {
    bool operator()() {
      return true;
    }

    bool operator()(exception_ptr exception) {
      return true;
    }
};

struct Object {
    bool resolved = false;
    bool rejected = false;

    void resolve() {
      resolved = true;
    }

    void reject(exception_ptr exception) {
      rejected = true;
    }
};

template<typename T>
static Promise<T> promisify(thread &t, T value) {
  return [&](Result<T> result) {
    t = thread([=] {
      result.resolve(value);
    });
  };
}

static Promise<void> promisify(thread &t) {
  return [&](Result<void> result) {
    t = thread([=] {
      result.resolve();
    });
  };
}

static void test_executor_with_function() {
  int prevCalls = executorFunctionCalls;

  Promise<void> p = executorFunction;

  assert(executorFunctionCalls == prevCalls + 1);
}

static void test_executor_with_noncapture_lambda() {
  static int lambdaCalls = 0;
  int prevCalls = lambdaCalls;

  Promise<void> p = [](Result<void> result) {
    lambdaCalls++;
    result.resolve();
  };

  assert(lambdaCalls == prevCalls + 1);
}

static void test_executor_with_capture_lambda() {
  bool called = false;

  Promise<void> p = [&called](Result<void> result) {
    called = true;
    result.resolve();
  };

  assert(called);
}

static void test_result_copy() {
  Result<void> result;

  try {
    result.resolve();
    assert(false); // should have thrown
  } catch (logic_error &e) {
    assert(e.what());
  }

  Promise<void> p = [&result](Result<void> r) {
    result = r;
  };

  result.resolve();
}

static void test_result_non_void() {
  Result<int> result;
  Promise<int> p = result;
  result.resolve(10);
}

static void test_then_with_function() {
  int prevCalls = thenFunctionCalls;

  Result<void> result;
  Promise<void> p = result;

  p.then<void>(thenFunction);

  assert(thenFunctionCalls == prevCalls);

  result.resolve();

  assert(thenFunctionCalls == prevCalls + 1);
}

static void test_then_with_noncapture_lambda() {
  static int lambdaCalls = 0;
  int prevCalls = lambdaCalls;

  Result<void> result;
  Promise<void> p = result;

  p.then<void>([] {
    lambdaCalls++;
  });

  assert(lambdaCalls == prevCalls);

  result.resolve();

  assert(lambdaCalls == prevCalls + 1);
}

static void test_then_with_capture_lambda() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  p.then<void>([&called] {
    called = true;
  });

  assert(!called);

  result.resolve();

  assert(called);
}

static void test_then_with_functor() {
  Result<void> result;
  Promise<void> promise = result;

  bool resolved = false;

  promise.then(Functor()).then([&resolved](bool b) {
    resolved = b;
  });

  result.resolve();

  assert(resolved);
}

static void test_then_with_method() {
  Result<void> result;
  Promise<void> promise = result;

  Object obj;
  promise.then(method(obj, &Object::resolve));

  result.resolve();

  assert(obj.resolved);
}

static void test_grab_with_function() {
  int prevCalls = grabFunctionCalls;

  Result<void> result;
  Promise<void> p = result;

  p.grab<void>(grabFunction);

  assert(grabFunctionCalls == prevCalls);

  result.reject(newException());

  assert(grabFunctionCalls == prevCalls + 1);
}

static void test_grab_with_noncapture_lambda() {
  static int lambdaCalls = 0;
  int prevCalls = lambdaCalls;

  Result<void> result;
  Promise<void> p = result;

  p.grab<void>([](exception_ptr) {
    lambdaCalls++;
  });

  assert(lambdaCalls == prevCalls);

  result.reject(newException());

  assert(lambdaCalls == prevCalls + 1);
}

static void test_grab_with_capture_lambda() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  p.grab<void>([&called](exception_ptr) {
    called = true;
  });

  assert(!called);

  result.reject(newException());

  assert(called);
}

static void test_grab_with_functor() {
  Result<void> result;
  Promise<void> promise = result;

  bool rejected = false;

  promise.grab(Functor()).then([&](bool b) {
    rejected = b;
  });

  result.reject(newException());

  assert(rejected);
}

static void test_grab_with_method() {
  Result<void> result;
  Promise<void> promise = result;

  Object obj;
  promise.grab(method(obj, &Object::reject));

  result.reject(newException());

  assert(obj.rejected);
}

static void test_then_with_reject() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  p.then([&called] {
    called = true;
  });

  /* avoid report uncaught exception */
  p.grab(grabFunction);

  result.reject(newException());

  assert(!called);
}

static void test_then_after_resolved() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  result.resolve();

  assert(!called);

  p.then([&called] {
    called = true;
  });

  assert(called);
}

static void test_then_after_rejected() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  result.reject(newException());

  p.then([&called] {
    called = true;
  });

  /* avoid report uncaught exception */
  p.grab(grabFunction);

  assert(!called);
}

static void test_grab_with_resolve() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  p.grab([&called](exception_ptr) {
    called = true;
  });

  result.resolve();

  assert(!called);
}

static void test_grab_after_rejected() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  result.reject(newException());

  assert(!called);

  p.grab([&called](exception_ptr) {
    called = true;
  });

  assert(called);
}

static void test_grab_after_resolved() {
  bool called = false;

  Result<void> result;
  Promise<void> p = result;

  result.resolve();

  p.grab([&called](exception_ptr) {
    called = true;
  });

  assert(!called);
}

static void test_reject_after_resolved() {
  bool then = false;
  bool grab = false;

  Result<void> result;
  Promise<void> p = result;

  p.then([&then] {
    then = true;
  });

  p.grab([&grab](exception_ptr) {
    grab = true;
  });

  result.resolve();
  result.reject(newException());

  assert(then);
  assert(!grab);
}

static void test_resolve_after_rejected() {
  bool then = false;
  bool grab = false;

  Result<void> result;
  Promise<void> p = result;

  p.then([&then] {
    then = true;
  });

  p.grab([&grab](exception_ptr) {
    grab = true;
  });

  result.reject(newException());
  result.resolve();

  assert(!then);
  assert(grab);
}

static void test_simple_chain() {
  Promise<void>(executorFunction).then(thenFunction);
}

static void test_scope() {
  bool b1 = false, b2 = false;
  Result<void> result;

  {
    Promise<void>(result).then([&b1] {
      b1 = true;
    }).then([&b2] {
      b2 = true;
    });
  }

  result.resolve();
  assert(b1);
  assert(b2);
}

static void test_chain() {
  Result<void> result;
  Promise<void> root = result;
  bool called[12] = { false, };

#define CALL(i) &called = called[i]
#define CALLED called = true

  root.then([CALL(0)] {
    // called
    CALLED;
  }).then([CALL(1)] {
    // called
    CALLED;
  });

  root.grab([CALL(2)](auto) {
    // not called
    CALLED;
  }).then([CALL(3)] {
    // not called
    CALLED;
  });

  auto l1 = root.then([CALL(4)] {
    // called
    CALLED;
  });

  auto l2 = l1.then([CALL(5)] {
    // called
    CALLED;
    throw runtime_error("Test error");
  });

  l2.then([CALL(6)] {
    // not called
    CALLED;
  });

  l2.grab([CALL(7)](auto) {
    // called
    CALLED;
  }).then([CALL(8)] {
    // called
    CALLED;
    return 5;
  }).then([CALL(9)](int i) {
    // called
    CALLED;
    assert(i == 5);
  });

  l1.grab([CALL(10)](auto) {
    // not called
    CALLED;
  });

  l1.then([CALL(11)] {
    // called
    CALLED;
  });

#undef CALL
#undef CALLED

  result.resolve();

  assert(called[0]);
  assert(called[1]);
  assert(!called[2]);
  assert(!called[3]);
  assert(called[4]);
  assert(called[5]);
  assert(!called[6]);
  assert(called[7]);
  assert(called[8]);
  assert(called[9]);
  assert(!called[10]);
  assert(called[11]);
}

static void test_thread() {
  bool called = false;

  thread t;

  Promise<void> p = [&t](Result<void> result) {
    t = thread([=] {
      result.resolve();
    });
  };

  p.then([&called] {
    called = true;
  });

  t.join();

  assert(called);
}

static void test_unique_ptr() {
  using Ptr = unique_ptr<int>;

  Ptr ptr(new int(10));

  Result<Ptr> result;
  Promise<Ptr> promise = result;

  promise.then([](const Ptr &ptr) {
    return *ptr;
  }).then([](int i) {
    assert(i == 10);
  });

  // rvalue
  result.resolve(move(ptr));
}

static void test_shared_ptr() {
  using Ptr = shared_ptr<int>;

  Ptr ptr(new int(10));

  Result<Ptr> result;
  Promise<Ptr> promise = result;

  promise.then([](const Ptr &ptr) {
    return *ptr;
  }).then([](int i) {
    assert(i == 10);
  });

  // lvalue
  result.resolve(ptr);
}

static void test_resolved_lvalue() {
  bool called = false;

  Promise<int> p = resolved(10);

  p.then([&called](int i) {
    assert(i == 10);
    called = true;
  });

  assert(called);
}

static void test_resolved_rvalue() {
  using Ptr = unique_ptr<int>;

  bool called = false;

  Ptr ptr(new int(10));

  Promise<Ptr> p = resolved(move(ptr));

  p.then([&called](const Ptr &ptr) {
    assert(*ptr == 10);
    called = true;
  });

  assert(called);
}

static void test_resolved_void() {
  bool called = false;

  Promise<void> p = resolved();

  p.then([&called] {
    called = true;
  });

  assert(called);
}

static void test_rejected_ptr() {
  bool called = false;

  Promise<void> p = rejected<void>(make_exception_ptr(newException()));

  p.grab([&called](auto) {
    called = true;
  });

  assert(called);
}

static void test_rejected_ex() {
  bool called = false;

  Promise<void> p = rejected<void>(newException());

  p.grab([&called](auto) {
    called = true;
  });

  assert(called);
}

static void test_rethrow_exception() {
  static const char *msg = "Hello exception";
  Result<int> result;
  Promise<int> promise = result;
  promise.then([](int i) {
    assert(false); // should not be called
    return 1;
  },
  rethrow<int>).grab([](exception_ptr ex) {
    try {
      rethrow_exception(ex);
      assert(false);
    } catch (exception &e) {
      assert(strcmp(e.what(), msg) == 0);
    }
  });

  result.reject(runtime_error(msg));
}

static void test_log_exception() {
  Result<void> result;
  Promise<void> promise = result;

  promise.grab(PROMISE_LOG_EX);

  result.reject(runtime_error("This error should be logged"));
}

static void test_flatten() {
  thread t1, t2, t3;
  bool c1 = false, c2 = false, c3 = false;

  promisify(t1, 10).then([&](int value) {
    c1 = true;
    assert(value == 10);
    return promisify(t2, 20);
  }).then([&](int value) {
    c2 = true;
    assert(value == 20);
    return promisify(t3);
  }).then([&] {
    c3 = true;
  });

  t1.join();
  t2.join();
  t3.join();

  assert(c1);
  assert(c2);
  assert(c3);
}

static void test_voidify() {
  bool called = false;

  Promise<void> promise = [](Result<void> result) {
    result.resolve();
    return 10;
  };

  auto p1 = promise.then<void>([] {
    return 10;
  });

  p1.then([&called] {
    called = true;
  });

  assert(called);
}

static void test_fancy_conversions() {
  char c;

  Promise<void>([](auto result) {
    result.resolve();
  }).then<unsigned int>([] {
    return 'a';
  }).then([&c](long l) {
    c = l;
  });

  assert(c == 'a');
}

static void test_chain_constructor() {
  list<int> called;
  auto cb = [&called](int id) {
    return [&called, id](auto) {
      called.push_back(id);
    };
  };

  Result<char> result;

  Promise<char> p1 = result;
  p1 << cb(1);

  Promise<char> p2 = p1; // copy constructor
  p2 << cb(2);

  Promise<int> p3 = p2; // chain constructor (type is different)
  p3 << cb(3);

  p2 << cb(4); // runs after cb(5)

  p3 << cb(5); // runs before cb(4)

  result.resolve('a');

  list<int> expected { 1, 2, 3, 5, 4 };
  assert(called == expected);
}

int main() {
  test_executor_with_function();
  test_executor_with_noncapture_lambda();
  test_executor_with_capture_lambda();

  test_result_copy();
  test_result_non_void();

  test_then_with_function();
  test_then_with_noncapture_lambda();
  test_then_with_capture_lambda();
  test_then_with_functor();
  test_then_with_method();

  test_grab_with_function();
  test_grab_with_noncapture_lambda();
  test_grab_with_capture_lambda();
  test_grab_with_functor();
  test_grab_with_method();

  test_then_with_reject();
  test_then_after_resolved();
  test_then_after_rejected();

  test_grab_with_resolve();
  test_grab_after_rejected();
  test_grab_after_resolved();

  test_reject_after_resolved();
  test_resolve_after_rejected();

  test_simple_chain();
  test_scope();
  test_chain();

  test_thread();

  test_unique_ptr();
  test_shared_ptr();

  test_resolved_lvalue();
  test_resolved_rvalue();
  test_resolved_void();
  test_rejected_ptr();
  test_rejected_ex();

  test_rethrow_exception();
  test_log_exception();

  test_flatten();
  test_voidify();
  test_fancy_conversions();
  test_chain_constructor();

  cout << "OK" << endl;
}
