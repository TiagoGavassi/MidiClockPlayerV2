#include "esp32-hal-gpio.h"
// LedButton.cpp
#include "JimmyTheGavas.h"

LEDButton::LEDButton(int buttonPin, int ledPin, unsigned long debounceDelay, void (*pressHandler)())
  : buttonPin(buttonPin), ledPin(ledPin), debounceDelay(debounceDelay), pressHandler(pressHandler) {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lastDebounceTime = 0;
  lastButtonState = LOW;
  ledState = LOW;
  ledBehaviour = 0;
}
// ****** NOT IMPLEMENTED
LEDButton::LEDButton(int buttonPin, int ledPin, unsigned long debounceDelay, void (*pressHandler)(), int ledBehaviour)
  : buttonPin(buttonPin), ledPin(ledPin), ledBehaviour(ledBehaviour), debounceDelay(debounceDelay), pressHandler(pressHandler) {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lastDebounceTime = 0;
  lastButtonState = LOW;
  ledState = LOW;
}// ****** NOT IMPLEMENTED

void LEDButton::update() {
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastButtonState = reading;
    if (((millis() - lastDebounceTime) > debounceDelay) && (lastButtonState == HIGH)) {
      if (pressHandler != NULL) {

        if(ledBehaviour==2){// it can be improved. maybe create a switch case
          digitalWrite(ledPin, HIGH);
        }


        pressHandler();
      }
      lastDebounceTime = millis();
    } else if(lastButtonState==LOW && ledBehaviour==2){
      digitalWrite(ledPin, LOW);
    }
  }
}

bool LEDButton::toggleLED() {
  // Toggle the LED status
  if (ledState == HIGH) {
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    return false;
  } else {
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    return true;
  }
  return false;
}

// ****** NOT IMPLEMENTED
bool LEDButton::ledFollowsTheButton() {
  if (lastButtonState == HIGH) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}// ****** NOT IMPLEMENTED

bool LEDButton::getBtnState(){
  return lastButtonState;
}

void LEDButton::setLed(int i){
  digitalWrite(ledPin, i);
}




//**********************************************************


JimmyRotary::JimmyRotary(int pinA, int pinB, long debounceTime, long turningTimerTime)
    : outputA(pinA), outputB(pinB), debounce(debounceTime), turningTimer(turningTimerTime) {}

void JimmyRotary::setup() {
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);

  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA);
  bLastState = digitalRead(outputB);
}

int JimmyRotary::update() {
 aState = digitalRead(outputA);
  bState = digitalRead(outputB);
  int side=0;
  
  if (millis() - lastUpdate > debounce) {
    lastUpdate = millis();
    if (aState != aLastState || bState != bLastState) {
      if (millis() - turningStart > turningTimer) {
        if ((aState == 1 && bState == 0)) {  // test if turning right
          turningRight = true;
          turningStart = millis();
        } else if ((aState == 0 && bState == 1)) {
          turningRight = false;
          turningStart = millis();
        }
      }
      if (turningRight && aState + bState == 2) {  // if turning right count the 1 1 state
      
        side = 1;

      } else if (!turningRight && aState + bState == 2) {  // if NOT turning right count the 1 1 state
      
        side = -1;
      }
      aLastState = aState;
      bLastState = bState;
    }
  }
  return side;
}



