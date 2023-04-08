#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "adapter.h"
#include "gsettings.h"

class Settings {
    friend class SettingsPrivate;

    using PGSettings = gsettings::PGSettings;

    Adapter *adapter;
    PGSettings gsettings;
    void *ochid;

  public:
    Settings(Adapter *adapter, PGSettings gsettings);
    ~Settings();
};

#endif /* SETTINGS_H_ */
