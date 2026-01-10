#ifndef SENSOR_H_
#define SENSOR_H_

#include "gdbus.h"
#include "promise.h"
#include "signals.h"

class SensorProxy {
    friend class SensorProxyPrivate;

  public:
    enum Unit {
      UNKNOWN,
      VENDOR,
      LUX
    };

  private:
    gdbus::PGDBusProxy proxy;
    double lightLevel = 0;
    Unit unit = UNKNOWN;

  public:

    /**
     * Ping the service.
     * This ensures that the service actually exists.
     */
    static promise::Promise<void> pingService();

    signals::Signal<void()> lightLevelChanged;

    promise::Promise<void> connect();
    double getLightLevel() const;
    Unit getUnit() const;
    bool hasUnit() const;
};

#endif /* SENSOR_H_ */
