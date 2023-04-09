#include <glib.h>

#include "autobright.h"
#include "gsettings.h"

int main() {
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  Autobright autobright(gsettings::newDefault());
  autobright.connect().grab([=](std::exception_ptr exception) {
    // TODO: improve this
    promise::LogException("Main")(exception);
    g_main_loop_quit(loop);
  });
  g_main_loop_run(loop);
}
