#ifndef OVEN_CONTROLLER_H
#define OVEN_CONTROLLER_H

// states
#define ZONE_PREHEAT  0       // preheating zone (90s to get to a target temp, linearly) (max 2min)
#define ZONE_SOAK     1       // soaking zone (90s to get to another target temp, linearly) (max 2min)
#define ZONE_REFLOW   2       // reflow zone (50s to reach peak temperature, then come back down to initial temperature, almost trapezoid profile) (max 1.5min)
#define ZONE_COOL     3       // turn the oven off, don't end process until down to safe temperature

class OvenController {

public:
  OvenController(SSR* ssr, TempSensor* temp_sensor, Display* display);
  void follow_profile(ReflowProfile* profile);


private:
  SSR* ssr = nullptr;
  TempSensor* temp_sensor = nullptr;
  Display* display = nullptr;

};

#endif