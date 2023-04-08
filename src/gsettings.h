#ifndef GSETTINGS_H_
#define GSETTINGS_H_

#include <gio/gio.h>

#include "gobjectmm.h"

namespace gsettings {

  using PGSettings = gobject_ptr<GSettings>;

  PGSettings newDefault();

  PGSettings newFromPath(const char *path);

}

#endif /* GSETTINGS_H_ */
