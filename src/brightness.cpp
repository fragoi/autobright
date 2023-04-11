#include "brightness.h"
#include "logger.h"

using namespace std;
using namespace gdbus;
using namespace promise;
using namespace signals;

static const Logger logger("[BrightnessProxy]");

static const Setter<int> brightnessSetter { "Brightness", "i" };

struct BrightnessProxyPrivate {
    static void setProxy(BrightnessProxy *self, PGDBusProxy proxy);
    static void setBrightness(BrightnessProxy *self, int value);
    static Promise<void> ensureBrightness(BrightnessProxy *self);
};

struct ResolveOnEmit {
    Signal<void()> *signal;
    Result<void> result;

    void operator()() {
      signal->remove(*this);
      result.resolve();
    }
};

inline static void updateBrightness(BrightnessProxy *self, GDBusProxy *proxy) {
  PUGVariant brightness(g_dbus_proxy_get_cached_property(proxy, "Brightness"));
  if (brightness) {
    int value = g_variant_get_int32(brightness.get());
    BrightnessProxyPrivate::setBrightness(self, value);
  }
}

static void onPropertiesChanged(
    GDBusProxy *proxy,
    GVariant *changed_properties,
    const gchar *const*invalidated_properties,
    gpointer user_data) {
  BrightnessProxy *self = (BrightnessProxy*) user_data;
  updateBrightness(self, proxy);
}

void BrightnessProxyPrivate::setProxy(
    BrightnessProxy *self,
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
        "g-properties-changed",
        G_CALLBACK(onPropertiesChanged),
        self);

    updateBrightness(self, proxy.get());
  }
}

void BrightnessProxyPrivate::setBrightness(BrightnessProxy *self, int value) {
  if (self->brightness == value)
    return;

  self->brightness = value;
  self->brightnessChanged();
}

Promise<void> BrightnessProxyPrivate::ensureBrightness(BrightnessProxy *self) {
  LOGGER(logger) << "Initial brightness: " << self->brightness << endl;

  if (self->brightness >= 0)
    return resolved();

  Result<void> result;
  Promise<void> promise = result;
  self->brightnessChanged << ResolveOnEmit {
      &self->brightnessChanged, result
  };
  return promise;
}

Promise<void> BrightnessProxy::connect() {
  if (proxy)
    return resolved();

  Promise<PGDBusProxy> p = newForBus(
      G_BUS_TYPE_SESSION,
      "org.gnome.SettingsDaemon.Power",
      "/org/gnome/SettingsDaemon/Power",
      "org.gnome.SettingsDaemon.Power.Screen");

  return p << [=](PGDBusProxy proxy) {
    BrightnessProxyPrivate::setProxy(this, proxy);
    return BrightnessProxyPrivate::ensureBrightness(this);
  };
}

Promise<void> BrightnessProxy::setBrightness(int value) {
  if (brightness == value)
    return resolved();
  return brightnessSetter(proxy, value);
}

int BrightnessProxy::getBrightness() const {
  return brightness;
}
