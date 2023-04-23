#include <math.h>

#include "autobright.h"
#include "logger.h"

using namespace std;
using namespace promise;

static const Logger logger("[Autobright]", Logger::INFO);

struct AutobrightPrivate {
    static void onLightLevelChanged(Autobright *self);
};

inline static int normalizeLux(double lux) {
  return lux < 1 ? 0 : round(log10(lux) / 3 * 100);
}

void AutobrightPrivate::onLightLevelChanged(Autobright *self) {
  double lightLevel = self->sensor.getLightLevel();
  int normalized = normalizeLux(lightLevel);
  self->normalized = normalized;
  int filtered = self->filter.filter(normalized);
  self->filtered = filtered;
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

void Autobright::updateDebugInfo(DebugInfo *info) {
  info->lightLevel = sensor.getLightLevel();
  info->normalized = normalized;
  //info->pressure = filter.
  info->filtered = filtered;
  info->value = adapter.getValue();
  info->offset = adapter.getOffset();
  info->brightness = bright.getBrightness();
  //info->flags = bright.
}
