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
  ReflowProfile();                          // create one from scratch
  ReflowProfile(const char* saved_profile_name);  // load from flash storage

  // saving, editting and deleting functions
  void delete_profile();                            // delete from flash storage
  void save_profile();                              // save to flash storage

  // for reflow process
  void start_profile();
  float get_target_temp();

private:
  unsigned long start_millis;
  ReflowProfileNode* start_profile_node = nullptr;

};

#endif