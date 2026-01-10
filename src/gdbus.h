#ifndef GDBUS_H_
#define GDBUS_H_

#include <gio/gio.h>

#include "gobjectmm.h"
#include "promise.h"

namespace gdbus {

  using namespace promise;

  using PGDBusProxy = gobject_ptr<GDBusProxy>;

  using PUGVariant = gunique_ptr<GVariant, g_variant_unref>;

  using PSGVariant = gshared_ptr<GVariant, g_variant_unref>;

  Promise<PGDBusProxy> newForBus(
      GBusType busType,
      const char *name,
      const char *path,
      const char *ifaceName,
      GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE,
      GDBusInterfaceInfo *info = NULL,
      GCancellable *cancellable = NULL);

  Promise<PUGVariant> call(
      PGDBusProxy proxy,
      const char *methodName,
      PUGVariant *parameters,
      GDBusCallFlags flags = G_DBUS_CALL_FLAGS_NONE,
      int timeout = -1,
      GCancellable *cancellable = NULL);

  Promise<void> ping(
      PGDBusProxy proxy,
      GDBusCallFlags flags = G_DBUS_CALL_FLAGS_NONE,
      int timeout = -1,
      GCancellable *cancellable = NULL);

  template<typename T>
  T gVariantGet(GVariant*);

  template<typename T>
  T gVariantGetChild(GVariant *container, int index) {
    PUGVariant child(g_variant_get_child_value(container, index));
    return gVariantGet<T>(child.get());
  }

  template<typename T>
  T pgVariantGet(const PUGVariant &value) {
    return gVariantGet<T>(value.get());
  }

  template<typename T>
  T pgVariantRet(const PUGVariant &value) {
    return gVariantGetChild<T>(value.get(), 0);
  }

  template<typename T>
  T pgVariantRetUnpack(const PUGVariant &value) {
    PUGVariant child(g_variant_get_child_value(value.get(), 0));
    PUGVariant variant(g_variant_get_variant(child.get()));
    return gVariantGet<T>(variant.get());
  }

  template<typename T>
  struct Method;

  template<typename Ret, typename ...Args>
  struct Method<Ret(Args...)> {
      const char *methodName;
      const char *paramsTypeString;
      GDBusCallFlags flags = G_DBUS_CALL_FLAGS_NONE;
      int timeout = -1;
      GCancellable *cancellable = NULL;

      Promise<Ret> operator()(PGDBusProxy proxy, Args ...args) const {
        PUGVariant parameters(paramsTypeString ?
            g_variant_new(paramsTypeString, args...) :
            NULL);
        Promise<PUGVariant> promise = call(
            proxy,
            methodName,
            &parameters,
            flags,
            timeout,
            cancellable);
        return promise.then(pgVariantRet<Ret>, rethrow<Ret>);
      }
  };

  template<typename T>
  struct Getter {
      const char *propertyName;
      GDBusCallFlags flags = G_DBUS_CALL_FLAGS_NONE;
      int timeout = -1;
      GCancellable *cancellable = NULL;

      Promise<T> operator()(PGDBusProxy proxy) const {
        const char *iname = g_dbus_proxy_get_interface_name(proxy.get());
        PUGVariant parameters(g_variant_new("(ss)", iname, propertyName));
        Promise<PUGVariant> promise = call(
            proxy,
            "org.freedesktop.DBus.Properties.Get",
            &parameters,
            flags,
            timeout,
            cancellable);
        return promise.then(pgVariantRetUnpack<T>, rethrow<T>);
      }
  };

  template<typename T>
  struct Setter {
      const char *propertyName;
      const char *propertyType;
      GDBusCallFlags flags = G_DBUS_CALL_FLAGS_NONE;
      int timeout = -1;
      GCancellable *cancellable = NULL;

      Promise<void> operator()(PGDBusProxy proxy, T value) const {
        const char *iname = g_dbus_proxy_get_interface_name(proxy.get());
        PUGVariant parameters(g_variant_new("(ssv)", iname, propertyName,
            g_variant_new(propertyType, value)));
        Promise<PUGVariant> promise = call(
            proxy,
            "org.freedesktop.DBus.Properties.Set",
            &parameters,
            flags,
            timeout,
            cancellable);
        return promise.then(pgVariantRet<void>, rethrow<void>);
      }
  };

}

#endif /* GDBUS_H_ */
