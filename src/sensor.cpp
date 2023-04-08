#include "sensor.h"

using namespace std;
using namespace gdbus;
using namespace promise;

struct SensorProxyPrivate {
    static void setProxy(SensorProxy *sensor, PGDBusProxy proxy);
    static void setLightLevel(SensorProxy *sensor, double value);
};

inline static void updateLightLevel(SensorProxy *sensor, GDBusProxy *proxy) {
  PUGVariant lightLevel(g_dbus_proxy_get_cached_property(proxy, "LightLevel"));
  if (lightLevel) {
    double value = g_variant_get_double(lightLevel.get());
    SensorProxyPrivate::setLightLevel(sensor, value);
  }
}

static void onPropertiesChanged(
    GDBusProxy *proxy,
    GVariant *changed_properties,
    const gchar *const*invalidated_properties,
    gpointer user_data) {
  SensorProxy *sensor = (SensorProxy*) user_data;
  updateLightLevel(sensor, proxy);
}

void SensorProxyPrivate::setProxy(SensorProxy *sensor, PGDBusProxy proxy) {
  if (sensor->proxy == proxy)
    return;

  if (sensor->proxy) {
    g_signal_handlers_disconnect_by_data(sensor->proxy.get(), sensor);
  }

  sensor->proxy = proxy;

  if (proxy) {
    g_signal_connect(
        proxy.get(),
        "g-properties-changed",
        G_CALLBACK(onPropertiesChanged),
        sensor);

    updateLightLevel(sensor, proxy.get());
  }
}

void SensorProxyPrivate::setLightLevel(SensorProxy *sensor, double value) {
  if (sensor->lightLevel == value)
    return;

  sensor->lightLevel = value;
  sensor->lightLevelChanged();
}

Promise<void> SensorProxy::connect() {
  if (this->proxy)
    return resolved();

  Promise<PGDBusProxy> p = newForBus(
      G_BUS_TYPE_SYSTEM,
      "net.hadess.SensorProxy",
      "/net/hadess/SensorProxy",
      "net.hadess.SensorProxy");

  return p << [=](PGDBusProxy proxy) {
    SensorProxyPrivate::setProxy(this, proxy);
  };
}

double SensorProxy::getLightLevel() const {
  return lightLevel;
}
