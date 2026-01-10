#ifndef IDLE_AWARE_H_
#define IDLE_AWARE_H_

#include "brightness.h"
#include "idle-monitor.h"
#include "debug-info.h"

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
    void *idleWatchId = 0;
    long inactiveTimeout = 500;
    int inactiveCount = 0;

  public:

    /**
     * Ping the service.
     * This ensures that the service actually exists.
     */
    static promise::Promise<void> pingService();

    IdleAware();
    ~IdleAware();

    promise::Promise<void> connect();
    promise::Promise<void> setBrightness(int);
    int getBrightness() const;

    void updateDebugInfo(DebugInfo*) const;
};

#endif /* IDLE_AWARE_H_ */
