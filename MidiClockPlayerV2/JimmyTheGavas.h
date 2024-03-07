// "JimmyTheGavas.h"
// #include "JimmyTheGavas.h"


#ifndef JimmyTheGavas_H
#define JimmyTheGavas_H

#include <Arduino.h>


//LED RESPONSE

#define LED_TOGGLE_ONCLICK 1
#define LED_TURN_ON_PRESSED 2

class LEDButton {
  private:
    int buttonPin;
    int ledPin;
    int ledBehaviour;// ****** NOT IMPLEMENTED
    unsigned long debounceDelay;
    unsigned long lastDebounceTime;
    bool buttonState;
    bool lastButtonState;
    void (*pressHandler)();
    bool ledState;

  public:
    LEDButton(int buttonPin, int ledPin, unsigned long debounceDelay, void (*pressHandler)());
    LEDButton(int buttonPin, int ledPin, unsigned long debounceDelay, void (*pressHandler)(), int ledBehaviour);
    void update();
    bool toggleLED(); // New method declaration
    void setLed(int i);
    bool ledFollowsTheButton(); // ****** NOT IMPLEMENTED
    bool getBtnState();
};



//*****


class JimmyRotary {
public:
  JimmyRotary(int pinA, int pinB, long debounceTime = 20, long turningTimerTime = 200);

  void setup();
  int update();

private:
  int outputA;
  int outputB;
  int aState;
  int bState;
  int aLastState;
  int bLastState;

  long debounce;
  long lastUpdate;

  long turningTimer;
  long turningStart;
  bool turningRight;
};





#endif // LEDBUTTON_H