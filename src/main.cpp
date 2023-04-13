#include "gsettings.h"
#include "autobright.h"
#include "autobright-service.h"

int main() {
  Autobright autobright(gsettings::newDefault());
  AutobrightService service(&autobright);
  return service.run();
}
