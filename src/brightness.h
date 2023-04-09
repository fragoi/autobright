#ifndef BRIGHTNESS_H_
#define BRIGHTNESS_H_

#include "gdbus.h"
#include "promise.h"
#include "signals.h"

struct IBrightnessProxy {
    signals::Signal<void()> brightnessChanged;

    virtual ~IBrightnessProxy() = default;
    virtual promise::Promise<void> connect() = 0;
    virtual promise::Promise<void> setBrightness(int value) = 0;
    virtual int getBrightness() const = 0;
};

class BrightnessProxy: public IBrightnessProxy {
    friend class BrightnessProxyPrivate;

    gdbus::PGDBusProxy proxy;
    int brightness = -1;

  public:
    promise::Promise<void> connect();
    promise::Promise<void> setBrightness(int value);
    int getBrightness() const;
};

#endif /* BRIGHTNESS_H_ */
