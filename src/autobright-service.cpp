#include <gio/gio.h>

#include "autobright-service.h"
#include "gexception.h"
#include "logger.h"

using namespace std;

static const Logger logger("[AutobrightService]", Logger::DEBUG);

struct AutobrightServicePrivate {
    static void onBusAcquired(
        AutobrightService *self,
        GDBusConnection *connection);
    static void onNameAquired(AutobrightService *self);
    static void onNameLost(
        AutobrightService *self,
        GDBusConnection *connection);
    static void quit(AutobrightService *self, int status);
    static void updateDebug(AutobrightService *self);
};

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
  AutobrightServicePrivate::onNameLost(self, connection);
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
    /* for now, later maybe throw and terminate */
    LOGGER_ERROR(logger) << error.what() << endl;
  }
}

void AutobrightServicePrivate::onNameAquired(AutobrightService *self) {
  self->autobright->connect().grab([=](const exception_ptr &ex) {
    LOGGER_ERROR(logger) << "Error connecting: " << ex << endl;
    quit(self, EXIT_FAILURE);
  });
}

void AutobrightServicePrivate::onNameLost(
    AutobrightService *self,
    GDBusConnection *connection) {

  AutobrightDebug *debug = self->debug.get();
  GDBusInterfaceSkeleton *iface = G_DBUS_INTERFACE_SKELETON(debug);
  g_dbus_interface_skeleton_unexport(iface);

  quit(self, EXIT_FAILURE);
}

void AutobrightServicePrivate::quit(AutobrightService *self, int status) {
  if (!self->status)
    self->status = status;

  if (self->mainLoop)
    g_main_loop_quit(self->mainLoop);
}

void AutobrightServicePrivate::updateDebug(AutobrightService *self) {
  AutobrightDebug *debug = self->debug.get();
  GDBusInterfaceSkeleton *iface = G_DBUS_INTERFACE_SKELETON(debug);
  if (g_dbus_interface_skeleton_get_connection(iface)) {
//    LOGGER(logger) << "I've a connection" << endl;
  }
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
