#ifndef IDLE_MONITOR_H_
#define IDLE_MONITOR_H_

#include <stdexcept>

#include "gdbus.h"
#include "promise.h"
#include "signals.h"
#include "logger.h"

namespace idle {

  struct WatchFired: public signals::Signal<void(int)> {
      splist::List<P> handlers() {
        return this->Signal::handlers;
      }
  };

  struct WatchBase {
      WatchFired *signal = nullptr;
      int key = 0;
      long interval = 0;
  };

  template<typename Fn>
  struct Watch: public WatchBase {
      Fn handler;

      Watch(WatchFired *signal, int key, long interval, const Fn &handler) :
          handler(handler) {
        this->signal = signal;
        this->key = key;
        this->interval = interval;
      }

      void operator()(int key) {
        if (this->key != key)
          return;

        if (!interval)
          signal->remove(*this);

        handler();
      }
  };

}

class IdleMonitorProxy {
    friend class IdleMonitorProxyPrivate;

    static const Logger logger;

    gdbus::PGDBusProxy proxy;
    idle::WatchFired watchFired;

    promise::Promise<int> addIdleWatch(long interval);
    promise::Promise<int> addUserActiveWatch();

  public:
    promise::Promise<void> connect();
    promise::Promise<long> getIdleTime();

    template<typename Fn>
    promise::Promise<void*> addIdleWatch(long interval, const Fn &handler) {
      using Watch = idle::Watch<Fn>;
      return addIdleWatch(interval) << [=](int key) {
        if (!key)
          throw std::runtime_error("Service returned invalid key");

        LOGGER(logger) << "Added idle watch: " << key << std::endl;
        return watchFired << Watch(&watchFired, key, interval, handler);
      };
    }

    template<typename Fn>
    promise::Promise<void*> addUserActiveWatch(const Fn &handler) {
      using Watch = idle::Watch<Fn>;
      return addUserActiveWatch() << [=](int key) {
        if (!key)
          throw std::runtime_error("Service returned invalid key");

        LOGGER(logger) << "Added user active watch: " << key << std::endl;
        return watchFired << Watch(&watchFired, key, 0, handler);
      };
    }

    promise::Promise<void> removeWatch(void *id);
    promise::Promise<void> resetIdleTime();
    promise::Promise<void> removeAll();
    promise::Promise<void> refreshAll();
};

#endif /* IDLE_MONITOR_H_ */
