#ifndef REFLOW_PROFILE_H
#define REFLOW_PROFILE_H

#include <Arduino.h>

class ReflowProfileNode {
public:

private:
  float temperature;                      // ignored if previous/next are null, as this is the first/last node and we just start/end with room temp.
  ReflowProfileNode* previous = nullptr;
  ReflowProfileNode* next = nullptr;

};

class ReflowProfile {

public:
  ReflowProfile();

  // for reflow process
  bool load_profile(ReflowProfileNode* starting_node);
  bool is_loaded();
  bool start_profile();
  float get_target_temp();
  bool done_profile();

private:
  bool profile_loaded = false;
  unsigned long start_millis;
  ReflowProfileNode* start_profile_node = nullptr;

};

#endif