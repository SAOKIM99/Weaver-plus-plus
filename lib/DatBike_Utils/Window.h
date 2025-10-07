#ifndef WINDOW_H
#define WINDOW_H

#include <stdio.h>

class Window {
  private:
    double* samples;
    size_t n;
    double sum;
  public:
    Window(double* baseArray);
    void addSample(double sample);
    double getAverage();
    double getSum();
    double getLast();  
};

#endif