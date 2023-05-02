#include <math.h>
#include <stdexcept>

#include "autobright.h"
#include "logger.h"

using namespace std;
using namespace promise;
using Unit = SensorProxy::Unit;

static const Logger logger("[Autobright]", Logger::DEFAULT);

struct AutobrightPrivate {
    static void onLightLevelChanged(Autobright *self);
};

inline static int normalizeLux(double lux) {
  return lux < 1 ? 0 : round(log10(lux) / 3 * 100);
}

inline static int normalizeLightLevel(double lightLevel, Unit unit) {
  switch (unit) {
    case Unit::VENDOR:
      return lightLevel;
    case Unit::LUX:
      return normalizeLux(lightLevel);
    default:
      throw invalid_argument("Unknown unit");
  }
}

void AutobrightPrivate::onLightLevelChanged(Autobright *self) {
  if (!self->sensor.hasUnit())
    return;

  double lightLevel = self->sensor.getLightLevel();
  Unit unit = self->sensor.getUnit();
  int normalized = normalizeLightLevel(lightLevel, unit);
  self->normalized = normalized;
  int filtered = self->filter.filter(normalized);
  self->adapter.setValue(filtered);

  LOGGER_DEBUG(logger) << "Light Level Changed: " << lightLevel
      << ", normalized: " << normalized
      << ", filtered: " << filtered
      << endl;
}

Autobright::Autobright(PGSettings gsettings) :
    bright(),
    adapter(&bright),
    settings(&adapter, gsettings),
    sensor(),
    filter(),
    lightLevelChanged(sensor.lightLevelChanged),
    brightnessChanged(bright.brightnessChanged) {

  llchid = sensor.lightLevelChanged << [=] {
    AutobrightPrivate::onLightLevelChanged(this);
  };
}

Autobright::~Autobright() {
  sensor.lightLevelChanged.remove(llchid);
}

Promise<void> Autobright::connect() {
  return bright.connect() << [=] {
    filter.setValue(adapter.getValue());
    return sensor.connect();
  };
}

void Autobright::updateDebugInfo(DebugInfo *info) const {
  info->lightLevel = sensor.getLightLevel();
  info->normalized = normalized;
  info->value = adapter.getValue();
  info->offset = adapter.getOffset();
  info->brightness = bright.getBrightness();
  filter.updateDebugInfo(info);
  bright.updateDebugInfo(info);
}
