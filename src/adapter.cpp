#include "adapter.h"
#include "promise.h"
#include "logger.h"

using namespace std;

static const Logger logger("[Adapter]");

struct AdapterPrivate {
    static bool setOffset(Adapter *self, int value);
    static bool setValue(Adapter *self, int value);
    static void setBrightness(Adapter *self);
    static void onBrightnessChanged(Adapter *self);
};

inline static int clamp(int value, int min, int max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

bool AdapterPrivate::setOffset(Adapter *self, int value) {
  value = clamp(value, -100, 100);

  if (self->offset == value)
    return false;

  self->offset = value;
  self->offsetChanged();
  return true;
}

bool AdapterPrivate::setValue(Adapter *self, int value) {
  value = clamp(value, 0 - self->offset, 100 - self->offset);

  if (self->value == value)
    return false;

  self->value = value;
  self->valueChanged();
  return true;
}

void AdapterPrivate::setBrightness(Adapter *self) {
  int brightness = self->value + self->offset;
  LOGGER(logger) << "Setting brightness: " << brightness
      << ", value: " << self->value
      << ", offset: " << self->offset
      << endl;
  self->proxy->setBrightness(brightness).grab(PROMISE_LOG_EX);
}

void AdapterPrivate::onBrightnessChanged(Adapter *self) {
  int brightness = self->proxy->getBrightness();
  bool changed;
  if (self->value != Adapter::NO_VALUE) {
    changed = setOffset(self, brightness - self->value);
  } else {
    changed = setValue(self, brightness - self->offset);
  }
  changed && LOGGER(logger) << "Brightness changed: " << brightness
      << ", value: " << self->value
      << ", offset: " << self->offset
      << endl;
}

Adapter::Adapter(IBrightnessProxy *proxy) : proxy(proxy) {
  bchid = proxy->brightnessChanged << [=] {
    AdapterPrivate::onBrightnessChanged(this);
  };
}

Adapter::~Adapter() {
  proxy->brightnessChanged.remove(bchid);
}

int Adapter::getOffset() const {
  return offset;
}

void Adapter::setOffset(int value) {
  if (this->offset != value && AdapterPrivate::setOffset(this, value)) {
    if (this->value != NO_VALUE) {
      /* offset is changed so ensure value is still in range */
      AdapterPrivate::setValue(this, this->value);
      AdapterPrivate::setBrightness(this);
    }
  }
}

int Adapter::getValue() const {
  return value;
}

void Adapter::setValue(int value) {
  if (this->value != value && AdapterPrivate::setValue(this, value)) {
    AdapterPrivate::setBrightness(this);
  }
}
