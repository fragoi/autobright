#ifndef SENSOR_H_
#define SENSOR_H_

#include "gdbus.h"
#include "promise.h"
#include "signals.h"

class SensorProxy {
    friend class SensorProxyPrivate;

    gdbus::PGDBusProxy proxy;
    double lightLevel = 0;

  public:
    signals::Signal<void()> lightLevelChanged;

    promise::Promise<void> connect();
    double getLightLevel() const;
};

#endif /* SENSOR_H_ */
