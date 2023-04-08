#include "settings.h"

struct SettingsPrivate {
    static void getOffset(Settings *self);
    static void setOffset(Settings *self);
};

void SettingsPrivate::getOffset(Settings *self) {
  int offset = g_settings_get_int(self->gsettings.get(), "offset");
  self->adapter->setOffset(offset);
}

void SettingsPrivate::setOffset(Settings *self) {
  int offset = self->adapter->getOffset();
  g_settings_set_int(self->gsettings.get(), "offset", offset);
}

static void onOffsetChanged(
    GSettings *settings,
    const gchar *key,
    gpointer user_data) {
  Settings *self = (Settings*) user_data;
  SettingsPrivate::getOffset(self);
}

Settings::Settings(Adapter *adapter, PGSettings gsettings) :
    adapter(adapter), gsettings(gsettings) {

  if (!gsettings)
    return;

  g_signal_connect(
      gsettings.get(),
      "changed::offset",
      G_CALLBACK(onOffsetChanged),
      this);

  SettingsPrivate::getOffset(this);

  ochid = adapter->offsetChanged << [=] {
    SettingsPrivate::setOffset(this);
  };
}

Settings::~Settings() {
  if (!gsettings)
    return;

  adapter->offsetChanged.remove(ochid);

  g_signal_handlers_disconnect_by_data(gsettings.get(), this);
}
