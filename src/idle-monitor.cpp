#include <string.h>

#include "idle-monitor.h"

using namespace std;
using namespace gdbus;
using namespace promise;
using namespace idle;

static const Logger logger("[IdleMonitor]", Logger::DEBUG);

static const Method<unsigned long()> _getIdletime {
    "GetIdletime"
};
static const Method<unsigned int(unsigned long)> _addIdleWatch {
    "AddIdleWatch", "(t)"
};
static const Method<unsigned int()> _addUserActiveWatch {
    "AddUserActiveWatch"
};
static const Method<void(unsigned int)> _removeWatch {
    "RemoveWatch", "(u)"
};
static const Method<void()> _resetIdletime {
    "ResetIdletime"
};

const Logger IdleMonitorProxy::logger(::logger);

struct IdleMonitorProxyPrivate {
    static void setProxy(IdleMonitorProxy *self, PGDBusProxy proxy);
    static void onWatchFired(IdleMonitorProxy *self, int key);
    static WatchBase* findWatch(IdleMonitorProxy *self, void *id);
    static Promise<void> refreshKey(IdleMonitorProxy *self, void *id);
    static Promise<void> refreshKeys(
        IdleMonitorProxy *self,
        const list<void*> &ids);
    static bool compareAndSetKey(
        IdleMonitorProxy *self,
        void *id,
        int oldKey,
        int newKey);
};

static void onSignal(
    GDBusProxy *proxy,
    const gchar *sender_name,
    const gchar *signal_name,
    GVariant *parameters,
    gpointer user_data) {
  IdleMonitorProxy *self = (IdleMonitorProxy*) user_data;
  if (strcmp(signal_name, "WatchFired") == 0) {
    int key = gVariantGetChild<unsigned int>(parameters, 0);
    IdleMonitorProxyPrivate::onWatchFired(self, key);
  }
}

static void onOwnerChanged(
    GObject *object,
    GParamSpec *pspec,
    gpointer user_data) {
  GDBusProxy *proxy = G_DBUS_PROXY(object);
  // TODO: check pointer type is correct
  unique_ptr<char> owner(g_dbus_proxy_get_name_owner(proxy));
  LOGGER_DEBUG(logger) << "Owner changed: " << owner.get() << endl;
  if (!owner)
    return;

  IdleMonitorProxy *self = (IdleMonitorProxy*) user_data;
  self->refreshAll().grab(PROMISE_LOG_EX);
}

void IdleMonitorProxyPrivate::setProxy(
    IdleMonitorProxy *self,
    PGDBusProxy proxy) {
  if (self->proxy == proxy)
    return;

  if (self->proxy) {
    g_signal_handlers_disconnect_by_data(self->proxy.get(), self);
  }

  self->proxy = proxy;

  if (proxy) {
    g_signal_connect(
        proxy.get(),
        "g-signal",
        G_CALLBACK(onSignal),
        self);

    g_signal_connect(
        proxy.get(),
        "notify::g-name-owner",
        G_CALLBACK(onOwnerChanged),
        self);
  }
}

void IdleMonitorProxyPrivate::onWatchFired(
    IdleMonitorProxy *self,
    int key) {
  int handled = 0;
  self->watchFired(key, handled);
  if (handled != 1) {
    LOGGER_WARN(logger) << "watchFired handled: " << handled << endl;
  }
}

WatchBase* IdleMonitorProxyPrivate::findWatch(
    IdleMonitorProxy *self,
    void *id) {
  for (const WatchFired::P &handler : self->watchFired.handlers()) {
    if (handler->pdata() == id) {
      return (WatchBase*) handler->pdata();
    }
  }
  return nullptr;
}

Promise<void> IdleMonitorProxyPrivate::refreshKey(
    IdleMonitorProxy *self,
    void *id) {
  WatchBase *watch = findWatch(self, id);
  if (!watch)
    return rejected<void>(invalid_argument("Watch not found"));

  Promise<unsigned int> p = watch->interval ?
      _addIdleWatch(self->proxy, watch->interval) :
      _addUserActiveWatch(self->proxy);

  int oldKey = watch->key;

  return p << [=](int newKey) {
    if (compareAndSetKey(self, id, oldKey, newKey)) {
      LOGGER_INFO(logger) << "Refreshed "
          << (watch->interval ? "idle" : "user active")
          << " watch: " << oldKey << " -> " << newKey
          << endl;
    } else {
      LOGGER_ERROR(logger) << "Failed to refresh "
          << (watch->interval ? "idle" : "user active")
          << " watch: " << oldKey << " -> " << newKey
          << endl;
    }
  };
}

Promise<void> IdleMonitorProxyPrivate::refreshKeys(
    IdleMonitorProxy *self,
    const list<void*> &ids) {
  Result<void> result;
  Promise<void> promise = result;
  ResolveLatch rl = result;
  LogException eh = PROMISE_LOG_EX;
  for (void *id : ids) {
    refreshKey(self, id).then(rl, eh);
  }
  return promise;
}

bool IdleMonitorProxyPrivate::compareAndSetKey(
    IdleMonitorProxy *self,
    void *id,
    int oldKey,
    int newKey) {
  WatchBase *watch = findWatch(self, id);
  if (!watch)
    return false;

  if (watch->key != oldKey)
    return false;

  watch->key = newKey;
  return true;
}

Promise<void> IdleMonitorProxy::connect() {
  if (proxy)
    return resolved();

  Promise<PGDBusProxy> p = newForBus(
      G_BUS_TYPE_SESSION,
      "org.gnome.Mutter.IdleMonitor",
      "/org/gnome/Mutter/IdleMonitor/Core",
      "org.gnome.Mutter.IdleMonitor");

  return p << [=](PGDBusProxy proxy) {
    IdleMonitorProxyPrivate::setProxy(this, proxy);
  };
}

Promise<long> IdleMonitorProxy::getIdleTime() {
  return _getIdletime(proxy);
}

Promise<int> IdleMonitorProxy::addIdleWatch(long interval) {
  return _addIdleWatch(proxy, interval);
}

Promise<int> IdleMonitorProxy::addUserActiveWatch() {
  return _addUserActiveWatch(proxy);
}

Promise<void> IdleMonitorProxy::removeWatch(void *id) {
  WatchBase *watch = IdleMonitorProxyPrivate::findWatch(this, id);
  if (!watch)
    return resolved();

  int key = watch->key;

  watchFired.remove(id);

  Promise<void> p = _removeWatch(proxy, key);
  if (logger.isDebug()) {
    p.then([=] {
      logger.debug() << "Removed watch:" << key << endl;
    });
  }
  return p;
}

Promise<void> IdleMonitorProxy::resetIdleTime() {
  return _resetIdletime(proxy);
}

Promise<void> IdleMonitorProxy::removeAll() {
  Result<void> result;
  Promise<void> promise = result;
  ResolveLatch rl = result;
  LogException eh = PROMISE_LOG_EX;
  for (const WatchFired::P &handler : watchFired.handlers()) {
    removeWatch(handler->pdata()).then(rl, eh);
  }
  return promise;
}

Promise<void> IdleMonitorProxy::refreshAll() {
  list<void*> ids;
  Result<void> result;
  Promise<void> promise = result;
  ResolveLatch rl = result;
  LogException eh = PROMISE_LOG_EX;
  for (const WatchFired::P &handler : watchFired.handlers()) {
    ids.push_back(handler->pdata());
    WatchBase *watch = (WatchBase*) handler->pdata();
    _removeWatch(proxy, watch->key).then(rl, eh);
  }
  return promise << [=] {
    return IdleMonitorProxyPrivate::refreshKeys(this, ids);
  };
}
