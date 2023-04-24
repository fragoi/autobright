#include "filter.h"

void PressureFilter::setValue(int value) {
  this->value = value;
}

int PressureFilter::filter(int v) {
  int d = v - value;
  if (d > 0 && pressure >= 0) {
    pressure += (d * d * 0.01) * (v * 0.01);
    if (pressure >= 1) {
      value = v;
      pressure = 0;
    }
  } else if (d < 0 && pressure <= 0) {
    pressure -= (d * d * 0.01) * (value * 0.01);
    if (pressure <= -1) {
      value = v;
      pressure = 0;
    }
  } else {
    pressure = 0;
  }
  return value;
}

void PressureFilter::updateDebugInfo(DebugInfo *info) const {
  info->pressure = pressure;
  info->filtered = value;
}
