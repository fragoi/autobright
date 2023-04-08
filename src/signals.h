#ifndef SIGNALS_H_
#define SIGNALS_H_

#include <memory>
#include <utility>

#include "splist.h"

namespace _signals {

  using namespace std;

  template<typename T>
  struct IHandler;

  template<typename Ret, typename ...Args>
  struct IHandler<Ret(Args...)> {
      virtual ~IHandler() = default;
      virtual Ret operator()(const Args&...) = 0;
      virtual void* pdata() = 0;
  };

  template<typename T>
  using PHandler = unique_ptr<IHandler<T>>;

  template<typename Fn, typename Ret, typename ...Args>
  class Handler: public IHandler<Ret(Args...)> {
      Fn fn;

    public:
      Handler(Fn &&fn) : fn(forward<Fn>(fn)) {
      }

      Ret operator()(const Args &...args) override {
        return fn(args...);
      }

      void* pdata() override {
        return (void*) &fn;
      }
  };

  template<typename T>
  class Signal;

  template<typename Ret, typename ...Args>
  class Signal<Ret(Args...)> {
    public:
      using P = PHandler<Ret(Args...)>;

    protected:
      splist::List<P> handlers;

    public:
      Signal& operator=(const Signal&) = delete;
      Signal& operator=(Signal&&) = delete;

      void emit(const Args &...args) const {
        for (const P &handler : handlers) {
          (*handler)(args...);
        }
      }

      template<typename Fn>
      void* add(Fn &&fn) {
        P handler(new Handler<Fn, Ret, Args...>(forward<Fn>(fn)));
        void *pdata = handler->pdata();
        handlers.push_back(move(handler));
        return pdata;
      }

      void remove(void *pdata) {
        for (const P &handler : handlers) {
          if (handler->pdata() == pdata) {
            handlers.remove(handler);
            break;
          }
        }
      }

      template<typename Fn>
      void remove(const Fn &fn) {
        void *pdata = (void*) &fn;
        remove(pdata);
      }

      void operator()(const Args &...args) const {
        emit(args...);
      }

      template<typename Fn>
      void* operator<<(Fn &&fn) {
        return add(forward<Fn>(fn));
      }

      Signal& operator>>(void *pdata) {
        remove(pdata);
        return *this;
      }

      template<typename Fn>
      Signal& operator>>(const Fn &fn) {
        remove(fn);
        return *this;
      }
  };

}

namespace signals {

  using _signals::Signal;

}

#endif /* SIGNALS_H_ */
