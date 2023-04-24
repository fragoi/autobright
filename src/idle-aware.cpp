#include <string>
#include <glib.h>

#include "idle-aware.h"
#include "logger.h"

using namespace std;
using namespace promise;

static const Logger logger("[IdleAware]");

struct IdleAwarePrivate {
    static void onBrightnessChanged(IdleAware *self);
    static void onActive(IdleAware *self);
    static void onIdle(IdleAware *self);
    static Promise<void> updateAsync(IdleAware *self);
    static Promise<void> updateIdle(IdleAware *self);
    static Promise<void> updateInactive(IdleAware *self);
    static void updateBrightness(IdleAware *self);
    static string flagsToString(int flags);
};

static gboolean gOnActive(gpointer user_data) {
  IdleAware *self = (IdleAware*) user_data;
  IdleAwarePrivate::onActive(self);
  return FALSE;
}

inline static void timeoutOnActive(long interval, IdleAware *self) {
  g_timeout_add(interval, gOnActive, self);
}

void IdleAwarePrivate::onBrightnessChanged(IdleAware *self) {
  LOGGER(logger) << "Brightness changed: " << self->proxy.getBrightness()
      << ", current: " << self->brightness
      << ", flags: " << flagsToString(self->flags)
      << endl;

  if (self->flags == IdleAware::IDLE) {
    updateAsync(self).grab(PROMISE_LOG_EX);
  } else {
    updateBrightness(self);
  }
}

void IdleAwarePrivate::onActive(IdleAware *self) {
  self->flags = IdleAware::NONE;
  LOGGER(logger) << "Active, brightness: " << self->brightness << endl;
  self->proxy.setBrightness(self->brightness).grab(PROMISE_LOG_EX);
}

void IdleAwarePrivate::onIdle(IdleAware *self) {
  self->flags |= IdleAware::IDLE;
}

Promise<void> IdleAwarePrivate::updateAsync(IdleAware *self) {
  return updateIdle(self) << [=] {
    return updateInactive(self);
  } << [=] {
    updateBrightness(self);
  };
}

Promise<void> IdleAwarePrivate::updateIdle(IdleAware *self) {
  if (self->flags != IdleAware::IDLE)
    return resolved();

  return self->idleMonitor.getIdleTime() << [=](long idleTime) {
    if (idleTime < self->idleInterval) {
      self->flags = IdleAware::NONE;
    } else {
      LOGGER(logger) << "Idle, time: " << idleTime << endl;
    }
  };
}

Promise<void> IdleAwarePrivate::updateInactive(IdleAware *self) {
//  if (!(self->flags & IdleAware::IDLE)
//      || (self->flags & IdleAware::INACTIVE))
//    return resolved();
  if (self->flags != IdleAware::IDLE)
    return resolved();

  self->inactiveCount++;
  return self->idleMonitor.addUserActiveWatch([=] {
    timeoutOnActive(self->inactiveTimeout, self);
  }) << [=](...) {
    self->flags |= IdleAware::INACTIVE;
    LOGGER(logger) << "Inactive, count: " << self->inactiveCount << endl;
  };
}

void IdleAwarePrivate::updateBrightness(IdleAware *self) {
  if (self->flags & IdleAware::DISABLED)
    return;

  if ((self->flags & IdleAware::INACTIVE)
      && self->brightness != self->proxy.getBrightness()) {

    self->flags |= IdleAware::DISABLED;
    LOGGER(logger) << "Disabled" << endl;
    return;
  }

  self->brightness = self->proxy.getBrightness();
  self->brightnessChanged();
}

string IdleAwarePrivate::flagsToString(int flags) {
  if (!flags)
    return "NONE";
  string s;
  if (flags & IdleAware::IDLE)
    s += "IDLE,";
  if (flags & IdleAware::INACTIVE)
    s += "INACTIVE,";
  if (flags & IdleAware::DISABLED)
    s += "DISABLED,";
  s.pop_back();
  return s;
}

IdleAware::IdleAware() {
  proxy.brightnessChanged << [=] {
    IdleAwarePrivate::onBrightnessChanged(this);
  };
}

IdleAware::~IdleAware() {
  idleMonitor.removeAll().grab(PROMISE_LOG_EX);
}

Promise<void> IdleAware::connect() {
  return proxy.connect() << [=] {
    return idleMonitor.connect();
  } << [=] {
    return idleMonitor.addIdleWatch(idleInterval, [=] {
      IdleAwarePrivate::onIdle(this);
    });
  };
}

Promise<void> IdleAware::setBrightness(int value) {
  LOGGER(logger) << "Setting brightness: " << value
      << ", current: " << brightness
      << ", flags: " << IdleAwarePrivate::flagsToString(flags)
      << endl;

  brightness = value;

  if (flags & DISABLED)
    return resolved();

  return proxy.setBrightness(value);
}

int IdleAware::getBrightness() const {
  return proxy.getBrightness();
}

void IdleAware::updateDebugInfo(DebugInfo *info) const {
  info->flags = flags;
}
