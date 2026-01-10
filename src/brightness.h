#ifndef BRIGHTNESS_H_
#define BRIGHTNESS_H_

#include "gdbus.h"
#include "promise.h"
#include "signals.h"

struct IBrightnessProxy {
    signals::Signal<void()> brightnessChanged;

    virtual ~IBrightnessProxy() = default;
    virtual promise::Promise<void> connect() = 0;
    virtual promise::Promise<void> setBrightness(int) = 0;
    virtual int getBrightness() const = 0;
};

class BrightnessProxy: public IBrightnessProxy {
    friend class BrightnessProxyPrivate;

    gdbus::PGDBusProxy proxy;
    int brightness = -1;

  public:

    /**
     * Ping the service.
     * This ensures that the service actually exists.
     */
    static promise::Promise<void> pingService();

    promise::Promise<void> connect();
    promise::Promise<void> setBrightness(int);
    int getBrightness() const;
};

#endif /* BRIGHTNESS_H_ */
