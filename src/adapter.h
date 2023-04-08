#ifndef ADAPTER_H_
#define ADAPTER_H_

#include "brightness.h"
#include "signals.h"

class Adapter {
    friend class AdapterPrivate;

    static const int NO_VALUE = -200;

    IBrightnessProxy *proxy;
    void *bchid = nullptr;
    int offset = 0;
    int value = NO_VALUE;

  public:
    signals::Signal<void()> offsetChanged;
    signals::Signal<void()> valueChanged;

    Adapter(IBrightnessProxy *proxy);
    ~Adapter();
    int getOffset() const;
    void setOffset(int value);
    int getValue() const;
    void setValue(int value);
};

#endif /* ADAPTER_H_ */
