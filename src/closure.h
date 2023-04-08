#ifndef CLOSURE_H_
#define CLOSURE_H_

#include <utility>
#include <stdexcept>
#include <memory>

namespace _closure {

  using namespace std;

  template<typename T>
  using FnVal = conditional_t<
  is_function<remove_reference_t<T>>::value, T, remove_reference_t<T>>;

  template<typename Ret, typename ...Args>
  struct ICallback {
      using Cb = Ret(*)(Args..., void*);
      virtual ~ICallback() = default;
      virtual Ret operator()(Args...) = 0;
      virtual Cb callback() = 0;
  };

  template<typename Fn, typename Ret, typename ...Args>
  class Callback: public ICallback<Ret, Args...> {
      using T = Callback<Fn, Ret, Args...>;
      using I = ICallback<Ret, Args...>;
      using Cb = typename I::Cb;
      using F = FnVal<Fn>;

      static Ret _callback(Args ...args, void *user_data) {
        T *p = (T*) user_data;
        unique_ptr<T> ptr(p);
        return ptr->fn(args...);
      }

      F fn;

    public:
      Callback(Fn &&fn) : fn(forward<Fn>(fn)) {
      }

      Ret operator()(Args ...args) override {
        return fn(args...);
      }

      Cb callback() override {
        return _callback;
      }
  };

  template<typename T>
  class Closure;

  template<typename Ret, typename ...Args>
  class Closure<Ret(Args...)> {
      using I = ICallback<Ret, Args...>;
      using Cb = typename I::Cb;

      I *ptr;
      Cb _callback;

    public:
      template<typename Fn>
      Closure(Fn &&fn) :
          ptr(new Callback<Fn, Ret, Args...>(forward<Fn>(fn))),
          _callback(ptr->callback()) {
      }

      ~Closure() noexcept {
        delete ptr;
      }

      Closure(const Closure&) = delete;
      Closure& operator=(const Closure&) = delete;

      Closure(Closure &&other) noexcept {
        swap(*this, other);
      }

      Closure& operator=(Closure &&other) noexcept {
        swap(*this, other);
        return *this;
      }

      Ret operator()(Args ...args) {
        if (!ptr)
          throw logic_error("Detached");
        return (*ptr)(args...);
      }

      void* detach() noexcept {
        I *tmp = ptr;
        ptr = nullptr;
        return tmp;
      }

      Cb callback() noexcept {
        return _callback;
      }

      friend void swap(Closure &a, Closure &b) noexcept {
        using std::swap;
        swap(a.ptr, b.ptr);
        swap(a._callback, b._callback);
      }
  };

}

namespace closure {

  using _closure::Closure;

}

#endif /* CLOSURE_H_ */
