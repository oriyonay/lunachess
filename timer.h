#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using namespace std::chrono;

// a simple class to time program execution
class timer {
private:
  steady_clock::time_point start_time, end_time;

public:
  void start() {
    start_time = steady_clock::now();
  }

  void stop() {
    end_time = steady_clock::now();
  }

  void print_duration() {
    double elapsed = duration<double>(duration_cast<milliseconds>(end_time - start_time)).count();
    printf("timer stopped at %f seconds\n", elapsed);
  }
};

#endif
