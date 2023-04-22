#ifndef AUTOBRIGHT_H_
#define AUTOBRIGHT_H_

#include "idle-aware.h"
#include "adapter.h"
#include "settings.h"
#include "sensor.h"
#include "filter.h"
#include "signals.h"
#include "gsettings.h"
#include "promise.h"

class Autobright {
    friend class AutobrightPrivate;

    using PGSettings = gsettings::PGSettings;

    IdleAware bright;
    Adapter adapter;
    Settings settings;
    SensorProxy sensor;
    PressureFilter filter;

    void *llchid = nullptr;
    int normalized = 0;
    int filtered = 0;

  public:
    signals::Signal<void()> &lightLevelChanged;
    signals::Signal<void()> &brightnessChanged;

    Autobright(PGSettings gsettings = PGSettings());
    ~Autobright();

    promise::Promise<void> connect();
};

#endif /* AUTOBRIGHT_H_ */
