#include <gio/gio.h>

#include "autobright-service.h"
#include "debug-info.h"
#include "gexception.h"
#include "logger.h"

using namespace std;

static const Logger logger("[AutobrightService]", Logger::DEBUG);

struct AutobrightServicePrivate {
    static void connectMethods(AutobrightService *self);
    static void onBusAcquired(
        AutobrightService *self,
        GDBusConnection *connection);
    static void onNameAquired(AutobrightService *self);
    static void onNameLost(AutobrightService *self);
    static void quit(AutobrightService *self, int status);
    static void enableDebug(AutobrightService *self);
    static void disableDebug(AutobrightService *self);
    static void updateDebug(AutobrightService *self);
};

static gboolean handleQuit(
    AutobrightDebug *object,
    GDBusMethodInvocation *invocation,
    gpointer user_data) {
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::quit(self, EXIT_SUCCESS);
  autobright_debug_complete_quit(object, invocation);
  return TRUE;
}

static gboolean handleEnable(
    AutobrightDebug *object,
    GDBusMethodInvocation *invocation,
    gpointer user_data) {
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::enableDebug(self);
  autobright_debug_complete_enable(object, invocation);
  return TRUE;
}

static gboolean handleDisable(
    AutobrightDebug *object,
    GDBusMethodInvocation *invocation,
    gpointer user_data) {
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::disableDebug(self);
  autobright_debug_complete_disable(object, invocation);
  return TRUE;
}

static void onBusAcquired(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] bus acquired, connection: "
      << g_dbus_connection_get_unique_name(connection) << endl;
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::onBusAcquired(self, connection);
}

static void onNameAquired(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] name acquired" << endl;
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::onNameAquired(self);
}

static void onNameLost(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] name lost" << endl;
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::onNameLost(self);
}

static void copyDebugInfo(DebugInfo *info, AutobrightDebug *debug) {
  autobright_debug_set_light_level(debug, info->lightLevel);
  autobright_debug_set_normalized(debug, info->normalized);
  autobright_debug_set_pressure(debug, info->pressure);
  autobright_debug_set_filtered(debug, info->filtered);
  autobright_debug_set_value(debug, info->value);
  autobright_debug_set_offset(debug, info->offset);
  autobright_debug_set_brightness(debug, info->brightness);
  autobright_debug_set_flags(debug, info->flags);
}

void AutobrightServicePrivate::connectMethods(AutobrightService *self) {
  AutobrightDebug *debug = self->debug.get();
  g_signal_connect(debug, "handle-quit", G_CALLBACK(::handleQuit), self);
  g_signal_connect(debug, "handle-enable", G_CALLBACK(::handleEnable), self);
  g_signal_connect(debug, "handle-disable", G_CALLBACK(::handleDisable), self);
}

void AutobrightServicePrivate::onBusAcquired(
    AutobrightService *self,
    GDBusConnection *connection) {

  AutobrightDebug *debug = self->debug.get();
  GDBusInterfaceSkeleton *iface = G_DBUS_INTERFACE_SKELETON(debug);
  GException error;

  g_dbus_interface_skeleton_export(
      iface,
      connection,
      "/com/github/fragoi/Autobright/Debug",
      error.get());

  if (error) {
    LOGGER_ERROR(logger) << "Error exporting debug: " << error.what() << endl;
  }
}

void AutobrightServicePrivate::onNameAquired(AutobrightService *self) {
  self->autobright->connect().grab([=](const exception_ptr &ex) {
    LOGGER_ERROR(logger) << "Error connecting: " << ex << endl;
    quit(self, EXIT_FAILURE);
  });
}

void AutobrightServicePrivate::onNameLost(AutobrightService *self) {
  AutobrightDebug *debug = self->debug.get();
  GDBusInterfaceSkeleton *iface = G_DBUS_INTERFACE_SKELETON(debug);
  if (g_dbus_interface_skeleton_get_connection(iface)) {
    g_dbus_interface_skeleton_unexport(iface);
  }

  quit(self, EXIT_FAILURE);
}

void AutobrightServicePrivate::quit(AutobrightService *self, int status) {
  if (!self->status)
    self->status = status;

  if (self->mainLoop)
    g_main_loop_quit(self->mainLoop);
}

void AutobrightServicePrivate::enableDebug(AutobrightService *self) {
  self->enable++;
  LOGGER(logger) << "Debug enabled, count: " << self->enable << endl;

  /* initial reading */
  if (self->enable == 1) {
    updateDebug(self);
  }
}

void AutobrightServicePrivate::disableDebug(AutobrightService *self) {
  if (self->enable < 1)
    return;

  self->enable--;
  LOGGER(logger) << "Debug disabled, count: " << self->enable << endl;

  /* reset to default, TODO: would be better to invalidate properties */
  if (self->enable == 0) {
    DebugInfo info;
    copyDebugInfo(&info, self->debug.get());
  }
}

void AutobrightServicePrivate::updateDebug(AutobrightService *self) {
  if (self->enable < 1)
    return;

  DebugInfo info;
  self->autobright->updateDebugInfo(&info);
  copyDebugInfo(&info, self->debug.get());
}

AutobrightService::AutobrightService(Autobright *autobright) :
    autobright(autobright),
    debug(autobright_debug_skeleton_new()) {

  llchid = autobright->lightLevelChanged << [=] {
    AutobrightServicePrivate::updateDebug(this);
  };

  bchid = autobright->brightnessChanged << [=] {
    AutobrightServicePrivate::updateDebug(this);
  };

  AutobrightServicePrivate::connectMethods(this);

  nameId = g_bus_own_name(
      G_BUS_TYPE_SESSION,
      "com.github.fragoi.Autobright",
      G_BUS_NAME_OWNER_FLAGS_NONE,
      onBusAcquired,
      onNameAquired,
      onNameLost,
      this,
      NULL);
}

AutobrightService::~AutobrightService() {
  autobright->lightLevelChanged.remove(llchid);
  autobright->brightnessChanged.remove(bchid);
  g_bus_unown_name(nameId);
}

int AutobrightService::run() {
  if (mainLoop)
    return EXIT_FAILURE;

  mainLoop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(mainLoop);
  return status;
}
