#include <math.h>

#include "filter.h"

struct PressureFilterPrivate {
    static void addValue(PressureFilter *self, int value);
    static int midValue(PressureFilter *self);
    static void reset(PressureFilter *self);
};

void PressureFilterPrivate::addValue(PressureFilter *self, int value) {
  int d = value - self->value;
  if (d > 0) {
    self->pressure += (d * d * 0.01) * (value * 0.01);
  } else if (d < 0) {
    self->pressure -= (d * d * 0.01) * (self->value * 0.01);
  }

  self->vpSum += value * 0.01;
  self->vpNum += 0.01;
}

int PressureFilterPrivate::midValue(PressureFilter *self) {
  return self->vpNum ? round(self->vpSum / self->vpNum) : 0;
}

void PressureFilterPrivate::reset(PressureFilter *self) {
  self->pressure = 0;
  self->vpSum = 0;
  self->vpNum = 0;
}

void PressureFilter::setValue(int value) {
  this->value = value;
}

int PressureFilter::filter(int v) {
  if (v > value && pressure >= 0) {
    PressureFilterPrivate::addValue(this, v);
    if (pressure >= 1) {
      value = PressureFilterPrivate::midValue(this);
      PressureFilterPrivate::reset(this);
    }
  } else if (v < value && pressure <= 0) {
    PressureFilterPrivate::addValue(this, v);
    if (pressure <= -1) {
      value = PressureFilterPrivate::midValue(this);
      PressureFilterPrivate::reset(this);
    }
  } else {
    PressureFilterPrivate::reset(this);
  }
  return value;
}

void PressureFilter::updateDebugInfo(DebugInfo *info) const {
  info->pressure = pressure;
  info->filtered = value;
}
