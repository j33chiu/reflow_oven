#ifndef SSR_H
#define SSR_H

#define DEFAULT_SSR_CONTROL_PIN 9 // D9

// singleton solid state relay class
class SSR {
public:
  // make assignment and copy inaccesible
  SSR(SSR const&) = delete;
  SSR(SSR &&) = delete;
  SSR& operator=(SSR const&) = delete;
  SSR& operator=(SSR &&) = delete;

  static SSR& get_instance(int pin = DEFAULT_SSR_CONTROL_PIN);
  
  void set_on();
  void set_off();
  void emer_stop();
  void reset();

private:
  SSR(int pin = DEFAULT_SSR_CONTROL_PIN);
  bool force_off = false;
  bool state_on = false;
  int pin;


};

#endif