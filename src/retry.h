#ifndef RETRY_H_
#define RETRY_H_

#include <glib.h>
#include <utility>

#include "promise.h"
#include "closure.h"

namespace _retry {

  using namespace promise;
  using namespace closure;

  using _promise::FnRet;
  using _promise::TPromise;

  template<typename Fn>
  struct Retry {
      using T = TPromise<FnRet<Fn>>;

      Fn fn;
      long backoff = 1000;
      double factor = 2;
      int retries = 3;

      Promise<T> operator()() const {
        Result<T> result;
        Promise<T> promise = result;
        call(result);
        return promise;
      }

      operator bool() const {
        return retries > 0;
      }

    private:
      void call(Result<T> result) const;

      bool retry(Result<T> result) const;

      template<typename >
      friend class Reject;
  };

  template<typename T>
  struct Resolve {
      Result<T> result;

      void operator()(const T &value) {
        result.resolve(value);
      }
  };

  template<>
  struct Resolve<void> {
      Result<void> result;

      void operator()() {
        result.resolve();
      }
  };

  template<typename Fn>
  struct Reject {
      using T = typename Retry<Fn>::T;

      Result<T> result;
      Retry<Fn> retry;

      void operator()(const exception_ptr &ex) {
        if (retry) {
          retry.retry(result);
        } else {
          result.reject(ex);
        }
      }
  };

  template<typename Fn>
  void Retry<Fn>::call(Result<T> result) const {
    Promise<T> promise = fn();
    promise.then(Resolve<T> { result }, Reject<Fn> { result, *this });
  }

  template<typename Fn>
  bool Retry<Fn>::retry(Result<T> result) const {
    if (retries < 1)
      return false;

    Retry copy = *this;
    copy.retries--;
    copy.backoff *= factor;

    Closure < gboolean() > closure = [=] {
      copy.call(result);
      return FALSE;
    };
    g_timeout_add(backoff, closure.callback(), closure.detach());

    return true;
  }

}

namespace retry {

  template<typename Fn>
  promise::Promise<typename _retry::Retry<Fn>::T> retry(Fn &&fn,
      long backoff = 1000,
      double factor = 2,
      int retries = 3) {
    _retry::Retry<Fn> retry { std::forward<Fn>(fn), backoff, factor, retries };
    return retry();
  }

}

#endif /* RETRY_H_ */
