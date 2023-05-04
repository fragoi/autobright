#ifndef FILTER_H_
#define FILTER_H_

#include "debug-info.h"

class PressureFilter {
    friend class PressureFilterPrivate;

    int value = 0;
    double pressure = 0;
    double vpSum = 0;
    double vpNum = 0;

  public:
    void setValue(int);
    int filter(int);

    void updateDebugInfo(DebugInfo*) const;
};

#endif /* FILTER_H_ */
