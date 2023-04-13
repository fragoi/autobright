#include <gio/gio.h>

#include "autobright-service.h"
#include "logger.h"

using namespace std;

static const Logger logger("[AutobrightService]", Logger::DEBUG);

struct AutobrightServicePrivate {
    static void connect(AutobrightService *self);
    static void quit(AutobrightService *self, int status);
};

static void onBusAcquired(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] bus acquired, connection: "
      << g_dbus_connection_get_unique_name(connection) << endl;
}

static void onNameAquired(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] name acquired" << endl;
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::connect(self);
}

static void onNameLost(
    GDBusConnection *connection,
    const gchar *name,
    gpointer user_data) {
  LOGGER(logger) << "[" << name << "] name lost" << endl;
  AutobrightService *self = (AutobrightService*) user_data;
  AutobrightServicePrivate::quit(self, EXIT_FAILURE);
}

void AutobrightServicePrivate::connect(AutobrightService *self) {
  self->autobright->connect().grab([=](const exception_ptr &ex) {
    LOGGER_ERROR(logger) << "Error connecting: " << ex << endl;
    quit(self, EXIT_FAILURE);
  });
}

void AutobrightServicePrivate::quit(AutobrightService *self, int status) {
  self->status = status;
  if (self->mainLoop)
    g_main_loop_quit(self->mainLoop);
}

AutobrightService::AutobrightService(Autobright *autobright) :
    autobright(autobright) {
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
  g_bus_unown_name(nameId);
}

int AutobrightService::run() {
  if (mainLoop)
    return EXIT_FAILURE;
  mainLoop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(mainLoop);
  return status;
}
