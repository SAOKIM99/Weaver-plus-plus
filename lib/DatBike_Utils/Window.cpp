#include "Window.h"

Window::Window(double* baseArray) : samples(baseArray) {
}

void Window::addSample(double sample) {
  sum -= samples[n];
  samples[n] = sample;
  sum += sample;
  
  n = (n + 1) % sizeof(samples);
}

double Window::getLast() {
  return samples[(n + sizeof(samples) - 1) % sizeof(samples)];
}

double Window::getSum() {
  return sum;
}

double Window::getAverage() {
  return sum / sizeof(samples);
}
