#ifndef IDLE_AWARE_H_
#define IDLE_AWARE_H_

#include "brightness.h"
#include "idle-monitor.h"

class IdleAware: public IBrightnessProxy {
    friend class IdleAwarePrivate;

    enum Flags {
      NONE = 0,
      IDLE = 1 << 0,
      INACTIVE = 1 << 1,
      DISABLED = 1 << 2
    };

    BrightnessProxy proxy;
    int brightness = -1;
    int flags = NONE;

    IdleMonitorProxy idleMonitor;
    long idleInterval = 5000;
    long inactiveTimeout = 500;
    int inactiveCount = 0;

  public:
    IdleAware();
    ~IdleAware();
    promise::Promise<void> connect();
    promise::Promise<void> setBrightness(int value);
    int getBrightness() const;
};

#endif /* IDLE_AWARE_H_ */
