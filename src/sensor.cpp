#include <string>
#include <stdexcept>

#include "sensor.h"

using namespace std;
using namespace gdbus;
using namespace promise;
using namespace signals;

struct SensorProxyPrivate {
    static void setProxy(SensorProxy *self, PGDBusProxy proxy);
    static void setLightLevel(SensorProxy *self, double value);
    static void setUnit(SensorProxy *self, const string &value);
    static Promise<void> ensureUnit(SensorProxy *self);
};

struct ResolveOnUnit {
    SensorProxy *sensor;
    Signal<void()> *signal;
    Result<void> result;

    void operator()() {
      if (!sensor->hasUnit())
        return;

      signal->remove(*this);
      result.resolve();
    }
};

inline static void updateLightLevel(SensorProxy *self, GDBusProxy *proxy) {
  PUGVariant lightLevel(g_dbus_proxy_get_cached_property(proxy, "LightLevel"));
  if (lightLevel) {
    double value = g_variant_get_double(lightLevel.get());
    SensorProxyPrivate::setLightLevel(self, value);
  }
}

static void updateUnit(SensorProxy *self, GDBusProxy *proxy) {
  PUGVariant unit(g_dbus_proxy_get_cached_property(proxy, "LightLevelUnit"));
  if (unit) {
    string value = pgVariantGet<string>(unit);
    SensorProxyPrivate::setUnit(self, value);
  }
}

static void onPropertiesChanged(
    GDBusProxy *proxy,
    GVariant *changed_properties,
    const gchar *const*invalidated_properties,
    gpointer user_data) {
  SensorProxy *self = (SensorProxy*) user_data;
  if (!self->hasUnit()) {
    updateUnit(self, proxy);
  }
  updateLightLevel(self, proxy);
}

void SensorProxyPrivate::setProxy(SensorProxy *self, PGDBusProxy proxy) {
  if (self->proxy == proxy)
    return;

  if (self->proxy) {
    g_signal_handlers_disconnect_by_data(self->proxy.get(), self);
  }

  self->proxy = proxy;

  if (proxy) {
    g_signal_connect(
        proxy.get(),
        "g-properties-changed",
        G_CALLBACK(onPropertiesChanged),
        self);

    updateUnit(self, proxy.get());
    updateLightLevel(self, proxy.get());
  }
}

void SensorProxyPrivate::setLightLevel(SensorProxy *self, double value) {
  if (self->lightLevel == value)
    return;

  self->lightLevel = value;
  self->lightLevelChanged();
}

void SensorProxyPrivate::setUnit(SensorProxy *self, const string &value) {
  if (value == "lux") {
    self->unit = SensorProxy::LUX;
  } else if (value == "vendor") {
    self->unit = SensorProxy::VENDOR;
  } else {
    throw invalid_argument(value);
  }
}

Promise<void> SensorProxyPrivate::ensureUnit(SensorProxy *self) {
  if (self->hasUnit())
    return resolved();

  Result<void> result;
  Promise<void> promise = result;
  self->lightLevelChanged << ResolveOnUnit {
      self, &self->lightLevelChanged, result
  };
  return promise;
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
    return SensorProxyPrivate::ensureUnit(this);
  };
}

double SensorProxy::getLightLevel() const {
  return lightLevel;
}

SensorProxy::Unit SensorProxy::getUnit() const {
  return unit;
}

inline bool SensorProxy::hasUnit() const {
  return unit != UNKNOWN;
}
