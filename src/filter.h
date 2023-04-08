#ifndef FILTER_H_
#define FILTER_H_

class PressureFilter {
    double pressure = 0;
    int value = 0;

  public:
    void setValue(int value);
    int filter(int value);
};

#endif /* FILTER_H_ */
