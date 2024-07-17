#ifndef REFLOW_PROFILE_H
#define REFLOW_PROFILE_H

#include <Arduino.h>

class ReflowProfile {

public:
  ReflowProfile();
  void start();
  float get_target_temp();

private:
  unsigned long start_millis;

  

};

#endif