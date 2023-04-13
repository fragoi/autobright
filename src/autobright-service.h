#ifndef AUTOBRIGHT_SERVICE_H_
#define AUTOBRIGHT_SERVICE_H_

#include <glib.h>

#include "autobright.h"

class AutobrightService {
    friend class AutobrightServicePrivate;

    Autobright *autobright;
    GMainLoop *mainLoop = nullptr;
    int nameId = 0;
    int status = 0;

  public:
    AutobrightService(Autobright *autobright);
    ~AutobrightService();

    int run();
};

#endif /* AUTOBRIGHT_SERVICE_H_ */
