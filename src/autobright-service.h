#ifndef AUTOBRIGHT_SERVICE_H_
#define AUTOBRIGHT_SERVICE_H_

#include <glib.h>

#include <autobright-dbus.h>

#include "autobright.h"
#include "gobjectmm.h"

class AutobrightService {
    friend class AutobrightServicePrivate;

    Autobright *autobright;
    gobject_ptr<AutobrightDebug> debug;
    GMainLoop *mainLoop = nullptr;
    void *llchid = nullptr;
    void *bchid = nullptr;
    int nameId = 0;
    int status = 0;
    int enable = 0;

  public:
    AutobrightService(Autobright *autobright);
    ~AutobrightService();

    int run();
};

#endif /* AUTOBRIGHT_SERVICE_H_ */
