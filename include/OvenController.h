#ifndef OVEN_CONTROLLER_H
#define OVEN_CONTROLLER_H

#include "SSR.h"
#include "TempSensor.h"
#include "Display.h"
#include "ReflowProfile.h"

// states
#define ZONE_PREHEAT  0       // preheating zone (90s to get to a target temp, linearly) (max 2min)
#define ZONE_SOAK     1       // soaking zone (90s to get to another target temp, linearly) (max 2min)
#define ZONE_REFLOW   2       // reflow zone (50s to reach peak temperature, then come back down to initial temperature, almost trapezoid profile) (max 1.5min)
#define ZONE_COOL     3       // turn the oven off, don't end process until down to safe temperature

class OvenController {

public:
  OvenController();
  void start_profile(const char* profile_name_pref_key, const char* profile_data_pref_key);
  void update();
  void end();


private:
  ReflowProfile profile;

};

#endif