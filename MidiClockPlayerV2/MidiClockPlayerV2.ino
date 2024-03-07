/* ESP 32 PINS
PUSH 1 D14
PUSH 2 D12
PUSH 3 d13 TAP

LED 1 D33
LED 2 D26
LED 3 D25
LED 4 D27

Rotary D34
Rotary D35
*/


#include <MIDI.h>
#include <HardwareSerial.h>
#include "JimmyTheGavas.h"

#include <SSD1283A.h>  //Hardware-specific library

SSD1283A display(/*CS=5*/ 15, /*DC=*/2, /*RST=*/4, -1);  //hardware spi,cs,cd,reset,led

byte clockMSG = 0xF8;     ///< System Real Time - Timing Clock
byte startMSG = 0xFA;     ///< System Real Time - Start
byte continueMSG = 0xFB;  ///< System Real Time - Continue
byte stopMSG = 0xFC;


#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


#define TAP_BPM 14
#define MIDI_START_STOP 13


#define TAP_BPM_LED 12
#define START_STOP_LED 5
#define LED_1 33
#define LED_2 26
#define LED_3 25
#define LED_4 27
#define BPM_ROT_1 34
#define BPM_ROT_2 35


int nBeat = 0;
int beats[] = { LED_1, LED_2, LED_3, LED_4 };



//******* DISPLAY TEST VARIABLES ************

bool isPlaying = false;
bool newInfoToPrint = true;
long lastPrint = 0;
int waitToPrint = 80;

long tt = 0;  // Time variable inside bpm counter

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midiA);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiB);

//void bpmOut() Variables
long bpm = 120;
long tempo = 0;
long prevTime = 0;
long startTime = 0;
long intervalInMicros = 0;
bool firstSend = true;
long beatIntervalInMicros = 0;

// Bpm Counter

int tickCounter = 0;
long firstTick = 0;
bool isFirstTick = true;

float bpmToDisplay = bpm;
long beatTimeMicros = 0;

// tapBpm Variables
const int tapsSamples = 3;
long taps[tapsSamples];  //where the inverval of the last 2 taps are saved
long lastTap = 0;
int indexTap = 0;

//Led Tempo Variables
int tickBpmLeds = 0;
int ledArray = 4;
int beatIndex = 3;

void startBtnPressed();
void tapBpm();

LEDButton startBtn(MIDI_START_STOP, START_STOP_LED, 150, startBtnPressed);
LEDButton tapBtn(TAP_BPM, TAP_BPM_LED, 150, tapBpm, 2);
JimmyRotary rotaryBpm(BPM_ROT_1, BPM_ROT_2);


// midiHandler() variables
bool discard2sMSG = true;
long secondCount = 0;

void setup() {
  Serial1.setPins(21, 22);
  midiB.begin(16);
  midiB.setThruFilterMode(midi::Thru::Full);
  midiA.begin(MIDI_CHANNEL_OMNI);  // Initialize the Midi Library.
  midiA.setThruFilterMode(midi::Thru::Full);

  pinMode(LED_1, OUTPUT);        //
  pinMode(LED_2, OUTPUT);        //
  pinMode(LED_3, OUTPUT);        //
  pinMode(LED_4, OUTPUT);        //
  pinMode(TAP_BPM_LED, OUTPUT);  //
  Serial.begin(57600);

  pinMode(TAP_BPM, INPUT);

  display.init();
  display.setTextColor(BLACK, GREEN);
  display.fillScreen(GREEN);

  rotaryBpm.setup();
  ligthShow();
  display.fillScreen(GREEN);



}  // SETUP END



void loop() {
  startBtn.update();
  tapBtn.update();

  int bpmTemp = bpm;
  bpm += rotaryBpm.update();
  if (bpmTemp != bpm) {
    newInfoToPrint = true;
  }

  bpmOut();


  if (newInfoToPrint && (millis() - lastPrint > waitToPrint)) {  //Let the printInfo function update the display. Reduce a bit the latency
    printInfo();
    lastPrint = millis();
  }


  secondCount = micros();  //Used on discart first 2seconds

  midiHandler();

}  // END LOOP




void midiHandler() {// Discard any clock msg or start/stop coming from the input
  if (midiA.read()) {
    byte type = midiA.getType();
    if (type == clockMSG || type == startMSG || type == stopMSG) {
      // Don't do nothing, just discart
    } else {
      midiA.send(midiA.getType(),
                   midiA.getData1(),
                   midiA.getData2(),
                   midiA.getChannel());
    }
  }  // end id(midARead)
}



void printInfo() {

  display.setTextSize(3);
  display.setCursor(34, 30);
  
  //display.println(String(bpmToDisplay)+" "+String(nBeat+1)+" "+String(bpm)+" "+playStatus);
  //display.println(String(tt)+" E:"+String(intervalInMicros)+" "+ (bpm < 100 ? " " : "") + String(bpm) + " " + (bpmToDisplay < 100 ? " " : "") + String(bpmToDisplay));

  display.print((bpm < 100 ? " " : "") + String(bpm) + "\n  ");
  display.setTextSize(2);
  display.print((bpmToDisplay < 100 ? " " : "") + String(bpmToDisplay));
  //Serial.println("DESIRED BPM");
  //Serial.println(bpm);
  //Serial.println("TRUE BPM");
  //Serial.println(bpmToDisplay);

  newInfoToPrint = false;
}


void startBtnPressed() {
  // Toggle the LED state when the start button is pressed

  if (startBtn.toggleLED()) {
    isPlaying = true;
    startTime = micros();
    tickCounter = 0;
    beatIndex = 3;
    isFirstTick = true;
    firstSend = true;
    tickBpmLeds = 0;

    //play send midiMSG to PLAY
    midiA.sendRealTime(midi::MidiType::Start);
    midiB.sendRealTime(midi::MidiType::Start);
    bpmOut();
  }

  else {
    isPlaying = false;
    midiA.sendRealTime(midi::MidiType::Stop);
    midiB.sendRealTime(midi::MidiType::Stop);
  }
}



void bpmOut() {
  intervalInMicros = round((60000000.00 / (bpm * 24.00)) * 0.997);  // this round and * 0.997 corrects the latency
  beatIntervalInMicros = 60000000 / bpm;
  if (firstSend) {
    firstSend = false;
    prevTime = startTime;
  }
  if (micros() - prevTime > intervalInMicros) {
    prevTime = micros();
    bpmLeds();
    bpmCounter();
    midiB.sendRealTime(midi::MidiType::Clock);
    midiA.sendRealTime(midi::MidiType::Clock);
  }
}



void bpmCounter() {  // Finds BPM counting MIDI Ticks.
  if (isFirstTick) {
    isFirstTick = false;
    firstTick = micros();
  }
  //calculates BPM
  if (tickCounter == 48) {
    long t = (micros() - firstTick) / 47.00;  // It's 95 because it is the number of intervals (time distance between two ticks), so if you have 96 ticks, you will have 95 intervals in total between them
    bpmToDisplay = (60000000.00 / 24.00) / t;
    tt = t;
    //start new sample;
    tickCounter = 0;
    isFirstTick = true;
    newInfoToPrint = true;
  }
  tickCounter++;
}


void bpmLeds() {

  if (tickBpmLeds % 24 == 0) {
    tapBtn.setLed(HIGH);
    digitalWrite(beats[beatIndex], LOW);
    beatIndex = (beatIndex + 1) % ledArray;
    digitalWrite(beats[beatIndex], isPlaying);
  } else if (tickBpmLeds % 2 == 0) {
    tapBtn.setLed(LOW);
  }
  tickBpmLeds = (tickBpmLeds + 1) % 96;
}

void tapBpm() {
  long currentTime = micros();
  if (lastTap == 0) {
    lastTap = currentTime;
  } else if (currentTime - lastTap > 1500000) {
    lastTap = currentTime;
    indexTap = 0;
  } else {
    taps[indexTap] = currentTime - lastTap;
    long avgTapInterval = 0;
    int numSamples = indexTap + 1;
    indexTap = (indexTap + 1) % tapsSamples;
    Serial.println("INDEX: " + String(indexTap));
    lastTap = currentTime;
    if (indexTap == 0) {  // if index returned to 0 again, that means there is 3 samples inputed within the 1.5s
      numSamples = 3;
    }
    for (int i = 0; i < numSamples; i++) {
      avgTapInterval += taps[i];
    }
    avgTapInterval = avgTapInterval / numSamples;
    long tapedBpm = 60000000 / avgTapInterval;
    if (tapedBpm > 240) {
      bpm = 240;
    } else if (bpm < 40) {
      bpm = 40;
    } else {
      bpm = tapedBpm;
    }
    newInfoToPrint = true;
  }
}  // End tap bpm

void ledTest() {
  int allLeds[] = { TAP_BPM, TAP_BPM_LED, START_STOP_LED, LED_1, LED_2, LED_3, LED_4 };
  for (int i = 0; i < 7; i++) {
    digitalWrite(allLeds[i], HIGH);
  }
}

void ligthShow() {
  int allLeds[] = { TAP_BPM, TAP_BPM_LED, START_STOP_LED, LED_1, LED_2, LED_3, LED_4 };
  for (int count = 0; count < 2; count++) {
    for (int i = 0; i < 7; i++) {
      digitalWrite(allLeds[i], HIGH);
      delay(100);
    }
    display.fillScreen(YELLOW);
    delay(100);
    display.fillScreen(BLUE);
    delay(100);
    display.fillScreen(GREEN);
    delay(100);
    display.fillScreen(RED);
    delay(100);
    display.fillScreen(BLUE);
    delay(100);
    display.fillScreen(MAGENTA);
    delay(100);
    display.fillScreen(GREEN);
    delay(100);
    display.fillScreen(RED);
    delay(100);
    display.fillScreen(BLUE);
    delay(100);

    for (int i = 7; i > 0; i--) {
      digitalWrite(allLeds[i], LOW);
      delay(100);
    }
  }


  display.setCursor(20, 40);
  display.setTextSize(1);
  display.fillScreen(GREEN);
  display.println("JIMMY THE GAVAS");
  display.println("    Technologies");

  delay(2000);
  //display.fillScreen(GREEN);
  display.println("");
  display.println("  MIDI CLOCK PLAYER");
  /*
  display.fillScreen(GREEN);
  display.setTextSize(6);
  display.setTextColor(BLACK, GREEN);
  display.setCursor(10, 40);
  */
}





//****** MAYBE LATER THAT MIGHT BE USEFUL POT


/*
const int numReadings = 10;  // Adjust the number of readings for the moving average filter
int readings[numReadings];   // Array to store the readings
int indexPot = 0;            // Index for the readings array
int totalReads = 0;          // Running total of the readings


int readBpmPot() {
  // Read the raw analog input
  int newRead=0;
  //int newRead = analogRead(BPM_POT);

  // Subtract the oldest reading from the total
  totalReads -= readings[indexPot];

  // Store the new reading in the array
  readings[indexPot] = newRead;

  // Add the new reading to the total
  totalReads += newRead;

  // Move to the next position in the array
  indexPot = (indexPot + 1) % numReadings;

  // Calculate the average reading
  int average = totalReads / numReadings;

  return average;
// This code below is from the loop()
/*
potValue = readBpmPot();  // Read the analog value from the potentiometer

  // O algoritmo pra evitar o jitter mistura de debounce com avg que acontece no readBpmPot
  if ((potValue < (m - 4)) || (potValue > (m + 4))) {
    m = potValue;
    bpm = floor((200.00 / 1023.00) * potValue + 40);
    newInfoToPrint = true;
  }

*/
