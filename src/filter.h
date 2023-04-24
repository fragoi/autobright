#ifndef FILTER_H_
#define FILTER_H_

#include "debug-info.h"

class PressureFilter {
    double pressure = 0;
    int value = 0;

  public:
    void setValue(int);
    int filter(int);

    void updateDebugInfo(DebugInfo*) const;
};

#endif /* FILTER_H_ */
