#include <string>

#include "gdbus.h"
#include "closure.h"
#include "gexception.h"

using namespace std;
using namespace gdbus;
using namespace closure;

template<typename T, typename V>
static void finish(
    const Result<T> &result,
    const GException &error,
    V &&value) {
  if (error) {
    result.reject(error);
  } else {
    result.resolve(forward<T>(value));
  }
}

namespace gdbus {

  Promise<PGDBusProxy> newForBus(
      GBusType busType,
      const char *name,
      const char *path,
      const char *ifaceName,
      GDBusProxyFlags flags,
      GDBusInterfaceInfo *info,
      GCancellable *cancellable) {

    Result<PGDBusProxy> result;
    Promise<PGDBusProxy> promise = result;
    Closure<void(GObject*, GAsyncResult*)> c = [result](
        GObject *source,
        GAsyncResult *res) {
      GException error;
      PGDBusProxy proxy(g_dbus_proxy_new_for_bus_finish(res, error.get()));
      finish(result, error, proxy);
    };

    g_dbus_proxy_new_for_bus(
        busType,
        flags,
        info,
        name,
        path,
        ifaceName,
        cancellable,
        c.callback(),
        c.detach());

    return promise;
  }

  Promise<PUGVariant> call(
      PGDBusProxy proxy,
      const char *methodName,
      PUGVariant *parameters,
      GDBusCallFlags flags,
      int timeout,
      GCancellable *cancellable) {

    if (!proxy) {
      return rejected<PUGVariant>(invalid_argument("Proxy is null"));
    }

    Result<PUGVariant> result;
    Promise<PUGVariant> promise = result;
    Closure<void(GObject*, GAsyncResult*)> c = [result](
        GObject *source,
        GAsyncResult *res) {
      GException error;
      PUGVariant value(g_dbus_proxy_call_finish(
          (GDBusProxy*) source, res, error.get()));
      finish(result, error, move(value));
    };

    bool floating = parameters && *parameters
        && g_variant_is_floating(parameters->get());

    g_dbus_proxy_call(
        proxy.get(),
        methodName,
        parameters ? parameters->get() : NULL,
        flags,
        timeout,
        cancellable,
        c.callback(),
        c.detach());

    if (floating) {
      parameters->release();
    }

    return promise;
  }

  template<>
  void gVariantGet<void>(GVariant*) {
  }

  template<>
  int gVariantGet<int>(GVariant *value) {
    return g_variant_get_int32(value);
  }

  template<>
  unsigned int gVariantGet<unsigned int>(GVariant *value) {
    return g_variant_get_uint32(value);
  }

  template<>
  unsigned long gVariantGet<unsigned long>(GVariant *value) {
    return g_variant_get_uint64(value);
  }

  template<>
  string gVariantGet<string>(GVariant *value) {
    return g_variant_get_string(value, NULL);
  }

  template<>
  void pgVariantRet<void>(const PUGVariant &value) {
  }

}
