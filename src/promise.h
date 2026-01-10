#ifndef PROMISE_H_
#define PROMISE_H_

#include <list>
#include <memory>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <mutex>

#define PROMISE_LOG_EX promise::LogException(__PRETTY_FUNCTION__)

namespace _promise {

  using namespace std;

  using Mutex = mutex;
  using Lock = lock_guard<Mutex>;

  enum StateFlags {
    PENDING = 0,
    SUCCESS = 1 << 0,
    FAILURE = 1 << 1,
    STATUS_MASK = (1 << 2) - 1,
    HANDLED = 1 << 2
  };

  template<typename T>
  struct Functor;

  template<typename Ret, typename ...Args>
  struct Functor<Ret(Args...)> {
      virtual ~Functor() = default;
      virtual Ret operator()(Args...) = 0;
  };

  template<typename ...Args>
  struct TICallback {
      using type = Functor<void(const Args&...)>;
  };

  template<>
  struct TICallback<void> {
      using type = Functor<void()>;
  };

  template<typename ...Args>
  using ICallback = typename TICallback<Args...>::type;

  template<typename ...Args>
  using PCallback = unique_ptr<ICallback<Args...>>;

  template<typename T>
  class Callbacks {
    private:
      list<T> callbacks;

    public:
      virtual ~Callbacks() = default;
      virtual void run(const T &cb) = 0;

      void add(T &&cb) {
        callbacks.push_back(move(cb));
      }

      void clear() {
        callbacks.clear();
      }

      void flush() {
        while (!callbacks.empty()) {
          run(callbacks.front());
          callbacks.pop_front();
        }
      }
  };

  /* base template */
  template<typename T>
  class Future: public Callbacks<PCallback<T>> {
    private:
      T value;

    public:
      using Cb = PCallback<T>;

      void run(const Cb &cb) override {
        (*cb)(value);
      }

      template<typename V>
      void set(V &&value) {
        this->value = forward<V>(value);
      }

      const T& get() {
        return value;
      }
  };

  /* pointer specialization */
  template<typename T>
  class Future<T*> : public Callbacks<PCallback<T>> {
    private:
      T *value = nullptr;

      void ensureEmpty() {
        if (this->value)
          throw logic_error("Value already set");
      }

    public:
      using Cb = PCallback<T>;

      ~Future() {
        delete value;
      }

      void run(const Cb &cb) override {
        (*cb)(*value);
      }

      template<typename V>
      void set(V &&value) {
        ensureEmpty();
        this->value = new T(forward<V>(value));
      }
  };

  /* pointer to void specialization */
  template<>
  class Future<void*> : public Callbacks<PCallback<void>> {
    public:
      using Cb = PCallback<void>;

      void run(const Cb &cb) override {
        (*cb)();
      }

      void set() {
      }
  };

  template<typename T>
  class State {
      using RF = Future<T*>;
      using EF = Future<exception_ptr>;

      using RC = typename RF::Cb;
      using EC = typename EF::Cb;

      int flags = PENDING;

      Mutex mtx;

      RF resolved;
      EF rejected;

      int status() {
        return flags & STATUS_MASK;
      }

    public:
      ~State() {
        if ((flags & FAILURE) && !(flags & HANDLED)) {
          try {
            rethrow_exception(rejected.get());
          } catch (const exception &e) {
            cerr << "Uncaught promise exception: " << e.what() << endl;
          } catch (...) {
            cerr << "Uncaught promise exception" << endl;
          }
        }
      }

      template<typename ...Args>
      void resolve(Args &&...value) {
        {
          Lock lock(mtx);
          if (status() != PENDING)
            return;
          resolved.set(forward<Args>(value)...);
          flags |= SUCCESS;
        }
        rejected.clear();
        resolved.flush();
      }

      void reject(const exception_ptr &exception) {
        {
          Lock lock(mtx);
          if (status() != PENDING)
            return;
          rejected.set(exception);
          flags |= FAILURE;
        }
        resolved.clear();
        rejected.flush();
      }

      void whenResolved(RC &&cb) {
        {
          Lock lock(mtx);
          switch (status()) {
            case PENDING:
              resolved.add(move(cb));
              return;
            case SUCCESS:
              break;
            default:
              return;
          }
        }
        resolved.run(cb);
      }

      void whenRejected(EC &&cb) {
        {
          Lock lock(mtx);
          flags |= HANDLED;
          switch (status()) {
            case PENDING:
              rejected.add(move(cb));
              return;
            case FAILURE:
              break;
            default:
              return;
          }
        }
        rejected.run(cb);
      }
  };

  template<typename T>
  using PState = shared_ptr<State<T>>;

  template<typename T, typename ...Args>
  struct Resolver {
      inline static
      void resolve(const PState<T> &state, Args &&...args) {
        state->resolve(forward<Args>(args)...);
      }
  };

  template<typename ...Args>
  struct Resolver<void, Args...> {
      inline static
      void resolve(const PState<void> &state, Args &&...args) {
        state->resolve();
      }
  };

  template<typename T, typename Fn, typename Ret, typename ...Args>
  struct FnResolver {
      inline static
      void resolve(const PState<T> &state, Fn &fn, const Args &...args) {
        Resolver<T, Ret>::resolve(state, fn(args...));
      }
  };

  template<typename Fn, typename ...Args>
  struct FnResolver<void, Fn, void, Args...> {
      inline static
      void resolve(const PState<void> &state, Fn &fn, const Args &...args) {
        fn(args...);
        state->resolve();
      }
  };

  template<typename T>
  using FnVal = conditional_t<
  is_function<remove_reference_t<T>>::value, T, remove_reference_t<T>>;

  template<typename T, typename Fn, typename Ret, typename ...Args>
  class Callback: public ICallback<Args...> {

      static_assert(!is_void<Ret>::value || is_void<T>::value,
          "Function returns void but state is not void");

      using P = PState<T>;
      using F = FnVal<Fn>;
      using R = FnResolver<T, F, Ret, Args...>;

      P state;
      F fn;

    public:
      Callback(const P &state, Fn &&fn) :
          state(state), fn(forward<Fn>(fn)) {
      }

      void operator()(const Args &...args) override {
        try {
          R::resolve(state, fn, args...);
        } catch (...) {
          state->reject(current_exception());
        }
      }
  };

  template<typename U, typename Fn, typename R, typename T>
  struct TTCallback {
      using type = Callback<U, Fn, R, T>;
  };

  template<typename U, typename Fn, typename R>
  struct TTCallback<U, Fn, R, void> {
      using type = Callback<U, Fn, R>;
  };

  template<typename U, typename Fn, typename R, typename T>
  using TCallback = typename TTCallback<U, Fn, R, T>::type;

  template<typename T>
  class Then {
      using P = PState<T>;

      P state;

    public:
      Then(const P &state) : state(state) {
      }

      template<typename ...Args>
      void operator()(Args &&...args) {
        Resolver<T, Args...>::resolve(state, forward<Args>(args)...);
      }
  };

  template<typename T>
  class Grab {
      using P = PState<T>;

      P state;

    public:
      Grab(const P &state) : state(state) {
      }

      void operator()(const exception_ptr &exception) {
        state->reject(exception);
      }
  };

  template<typename T>
  class ResultBase {
    protected:
      using P = PState<T>;

      P state;

      void ensureState() const {
        if (!state)
          throw logic_error("No state");
      }

    public:
      void reject(const exception_ptr &exception) const {
        ensureState();
        state->reject(exception);
      }

      template<typename Ex>
      void reject(const Ex &ex) const {
        reject(make_exception_ptr(ex));
      }

      template<typename >
      friend class Promise;

      friend bool hasState(const ResultBase &result) {
        return (bool) result.state;
      }
  };

  /* base template */
  template<typename T>
  class Result: public ResultBase<T> {
      using B = ResultBase<T>;

    public:
      void resolve(const T &value) const {
        B::ensureState();
        B::state->resolve(value);
      }

      void resolve(T &&value) const {
        B::ensureState();
        B::state->resolve(move(value));
      }
  };

  /* void specialization */
  template<>
  class Result<void> : public ResultBase<void> {
      using B = ResultBase<void>;

    public:
      void resolve() const {
        B::ensureState();
        B::state->resolve();
      }
  };

  template<typename T>
  T rethrow(const exception_ptr &exception) {
    rethrow_exception(exception);
  }

  template<typename Fn, typename ...Args>
  struct TFnRet {
      using type = result_of_t<Fn(Args...)>;
  };

  template<typename Fn>
  struct TFnRet<Fn, void> {
      using type = result_of_t<Fn()>;
  };

  template<typename Fn, typename ...Args>
  using FnRet = typename TFnRet<Fn, Args...>::type;

  struct Undefined;

  template<typename Fn, typename Ret, typename ...Args>
  struct TFnRetOpt {
      using type = Ret;
  };

  template<typename Fn, typename ...Args>
  struct TFnRetOpt<Fn, Undefined, Args...> {
      using type = FnRet<Fn, Args...>;
  };

  template<typename Fn, typename Ret, typename ...Args>
  using FnRetOpt = typename TFnRetOpt<Fn, Ret, Args...>::type;

  template<typename T>
  class Promise;

  template<typename T>
  struct TTPromise {
      using type = T;
  };

  template<typename T>
  struct TTPromise<Promise<T>> {
      using type = typename TTPromise<T>::type;
  };

  template<typename T>
  using TPromise = typename TTPromise<T>::type;

  template<typename T>
  class Promise {
      using S = State<T>;
      using P = PState<T>;

      P state = P(new S);

      Promise() {
      }

      template<typename U, typename Fn>
      void _then(const Promise<U> &other, Fn &&fn) const;

      template<typename U, typename Fn>
      void _grab(const Promise<U> &other, Fn &&fn) const;

    public:
      Promise(Result<T> &result) {
        result.state = state;
      }

      template<typename E = void(Result<T>)>
      Promise(const E &executor) {
        Result<T> result;
        result.state = state;
        try {
          executor(result);
        } catch (...) {
          result.reject(current_exception());
        }
      }

      template<typename U>
      Promise(const Promise<U> &other) {
        /* this is not a copy constructor */
        static_assert(!is_same<T, U>::value);
        other.then(Then<T>(state), Grab<T>(state));
      }

      template<typename R = Undefined, typename Fn,
          typename U = TPromise<FnRetOpt<Fn, R, T>>>
      Promise<U> then(Fn &&fn) const {
        Promise<U> other;
        _then(other, forward<Fn>(fn));
        return other;
      }

      template<typename R = Undefined, typename Fn, typename Eh,
          typename U = TPromise<FnRetOpt<Fn, R, T>>>
      Promise<U> then(Fn &&fn, Eh &&eh) const {
        Promise<U> other;
        _then(other, forward<Fn>(fn));
        _grab(other, forward<Eh>(eh));
        return other;
      }

      template<typename R = Undefined, typename Eh,
          typename U = TPromise<FnRetOpt<Eh, R, exception_ptr>>>
      Promise<U> grab(Eh &&eh) const {
        Promise<U> other;
        _grab(other, forward<Eh>(eh));
        return other;
      }

      /**
       * Shortcut for "then" with re-throw exception handler.
       */
      template<typename Fn, typename U = TPromise<FnRet<Fn, T>>>
      Promise<U> operator<<(Fn &&fn) const {
        return then<U>(forward<Fn>(fn), rethrow<U>);
      }

      template<typename >
      friend class Promise;
  };

  template<typename T>
  template<typename U, typename Fn>
  void Promise<T>::_then(const Promise<U> &other, Fn &&fn) const {
    using R = FnRet<Fn, T>;
    PCallback<T> callback(
        new TCallback<U, Fn, R, T>(other.state, forward<Fn>(fn)));
    state->whenResolved(move(callback));
  }

  template<typename T>
  template<typename U, typename Fn>
  void Promise<T>::_grab(const Promise<U> &other, Fn &&fn) const {
    using E = exception_ptr;
    using R = FnRet<Fn, E>;
    PCallback<E> callback(
        new TCallback<U, Fn, R, E>(other.state, forward<Fn>(fn)));
    state->whenRejected(move(callback));
  }

  template<typename T, typename V>
  struct Resolver<T, Promise<V>> {
      inline static
      void resolve(const PState<T> &state, const Promise<V> &promise) {
        promise.then(Then<T>(state), Grab<T>(state));
      }
  };

  template<typename V>
  struct Resolver<void, Promise<V>> {
      inline static
      void resolve(const PState<void> &state, const Promise<V> &promise) {
        promise.then(Then<void>(state), Grab<void>(state));
      }
  };

  template<typename T, typename Fn>
  class Method {
      T target;
      Fn fn;

    public:
      Method(T &&target, Fn &&fn) :
          target(forward<T>(target)), fn(forward<Fn>(fn)) {
      }

      template<typename ...Args>
      auto operator()(const Args &...args) {
        return (target.*fn)(args...);
      }
  };

  template<typename T, typename Fn>
  Method<T, Fn> method(T &&target, Fn &&fn) {
    return Method<T, Fn>(forward<T>(target), forward<Fn>(fn));
  }

}

namespace promise {

  using _promise::Promise;
  using _promise::Result;
  using _promise::method;
  using _promise::rethrow;

  using std::exception_ptr;
  using std::rethrow_exception;

  template<typename T>
  Promise<T> resolved(T &&value) {
    Result<T> result;
    Promise<T> promise = result;
    result.resolve(std::forward<T>(value));
    return promise;
  }

  template<typename = void>
  Promise<void> resolved() {
    Result<void> result;
    Promise<void> promise = result;
    result.resolve();
    return promise;
  }

  template<typename T>
  Promise<T> rejected(const exception_ptr &exception) {
    Result<T> result;
    Promise<T> promise = result;
    result.reject(exception);
    return promise;
  }

  template<typename T, typename Ex>
  Promise<T> rejected(const Ex &exception) {
    Result<T> result;
    Promise<T> promise = result;
    result.reject(exception);
    return promise;
  }

  class LogException {
      const char *prefix;

    public:
      LogException(const char *prefix) : prefix(prefix) {
      }

      void operator()(const exception_ptr &ptr) {
        using namespace std;
        const char *msg;
        try {
          rethrow_exception(ptr);
        } catch (const exception &e) {
          msg = e.what();
        } catch (...) {
          msg = "unknown";
        }
        cerr << "[" << prefix << "]: " << msg << endl;
      }
  };

  class ResolveLatch {
      using R = Result<void>;

      static R* resultPtr(const R &result) {
        if (!hasState(result))
          throw std::invalid_argument("Result has no state");
        return new R(result);
      }

      static void deleter(R *result) {
        result->resolve();
        delete result;
      }

      std::shared_ptr<R> ptr;

    public:
      ResolveLatch(const Result<void> &result) :
          ptr(resultPtr(result), deleter) {
      }

      void operator()(...) {
      }
  };

  template<typename T, typename Eh>
  Promise<void> resolveAll(const std::list<Promise<T>> &list, const Eh &eh) {
    Result<void> result;
    Promise<void> promise = result;
    ResolveLatch rl = result;
    for (const Promise<T> &promise : list) {
      promise.then(rl, eh);
    }
    return promise;
  }

}

#endif /* PROMISE_H_ */
