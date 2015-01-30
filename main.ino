#include <MIDI.h>

const int ledPin = 13;
//void BlinkLed(byte num) { 	// Basic blink function
//  for (byte i=0;i<num;i++) {
//    digitalWrite(ledPin,HIGH);
//    delay(50);
//    digitalWrite(ledPin,LOW);
//    delay(50);
//  }
//}

const boolean overrideVelocityPot = false;
const boolean overrideTransposePot = false;    
const int transposePotRange = 24; // 24 = one full octave up or down

const int instrumentNumber = 1;    // 1 works; most general midi instruments don't?
const int octaveNumber = 3;        // values are 0 through ?? (3 is a good bass range)
const int transposeHalfSteps = 0;  // 1: C becomes C#... -2: C becomes B (in middle of pot range)
                                  
const long debounceDelay = 10;

const int velocityPotPin = A5;    // select the input pin for the potentiometer
const int transposePotPin = A4;

const int keyCount = 13;

const int buttonSustainPin = A2;
const int buttonPin[keyCount] = {2,  3,  4,  5, 
                           6,  7,  8,  9, 
                          10, 11, 12, A0, A1};

int buttonSustainState;
int buttonState[keyCount] = {LOW, LOW, LOW, LOW, 
                       LOW, LOW, LOW, LOW, 
                       LOW, LOW, LOW, LOW, LOW};

int lastButtonSustainState = LOW;
int lastButtonState[keyCount] = {LOW, LOW, LOW, LOW, 
                           LOW, LOW, LOW, LOW, 
                           LOW, LOW, LOW, LOW, LOW};

long lastButtonSustainChangeMillis = 0;
long lastButtonChangeMillis[keyCount] = {0, 0, 0, 0, 
                                   0, 0, 0, 0, 
                                   0, 0, 0, 0, 0};


//===============================================================
// FUNCTIONS
//===============================================================

void SendNoteOn(byte pitch, int velocity) {
  MIDI.sendNoteOn(pitch,velocity,instrumentNumber); 
}

void SendNoteOff(byte pitch) {
  MIDI.sendNoteOff(pitch,0,1);
}

void SendSustainOn() {
  MIDI.sendControlChange(64,127,1); //sustain on
}

void SendSustainOff() {
  MIDI.sendControlChange(64,0,1); //sustain off
}

int getVelocity() {
  if (!overrideVelocityPot) {
    return analogRead(velocityPotPin)/8;
  }  
  else {
    return 127; //default if overriding
  } 
}

int getTranspose() {
  int transposeHalfStepsWithPot;
  if (!overrideTransposePot) {
    
    //all of this should be changed to use the "map" function, e.g.:
      //   val = map(val, 0, 1023, 0, 179);
    
    //highest value in range isn't actually achievable with a pot (goes to 1023, not 1024) so add one
    int transposePotRangePlusOne = transposePotRange + 1; 
    int transposePotReading = analogRead(transposePotPin);        
    //convert pot range (0 to 1023) into declared tranpose range: -(range/2) to +(range/2)    
    float convertedRange = (float) transposePotReading * ((float) transposePotRangePlusOne/1024);
    float transposeConvertedReading = convertedRange - (transposePotRangePlusOne/2);    
    transposeHalfStepsWithPot = transposeHalfSteps + (int) transposeConvertedReading;
  }
  else {
    transposeHalfStepsWithPot = transposeHalfSteps;
  }
  return transposeHalfStepsWithPot;    
}


void setup() {
  pinMode(ledPin, OUTPUT);
  
  pinMode(buttonSustainPin, INPUT);  
  for (int thisPin = 0; thisPin < keyCount; thisPin++) { 
    pinMode(buttonPin[thisPin], INPUT);
  }

  delay(1000);      //until the noteoff is figured out
  MIDI.begin(4);    // Launch MIDI with default options, input channel is 4
}


void loop() 
{
  //declare empty values (only really need to do this for the array, 
  // but declaring buttonSustainReading too just to keep things parallel.  
  
  int buttonSustainReading = LOW;
  buttonSustainReading = digitalRead(buttonSustainPin);
  buttonSustainState = buttonSustainReading;    
  if (buttonSustainState != lastButtonSustainState) {
    if (buttonSustainState == LOW)  
         { SendSustainOn(); }      
    else { SendSustainOff(); }      
    delay(debounceDelay);    
  }
 
  int buttonReading[keyCount] = {LOW, LOW, LOW, LOW, 
                           LOW, LOW, LOW, LOW, 
                           LOW, LOW, LOW, LOW, LOW};
  for (int i = 0; i < keyCount; i++) { 
    buttonReading[i] = digitalRead(buttonPin[i]);
    buttonState[i] = buttonReading[i];    
    if (buttonState[i] != lastButtonState[i]) {
      if (buttonState[i] == HIGH)  
           { SendNoteOn(octaveNumber*12 + getTranspose() + i, getVelocity()); }      
      else { SendNoteOff(octaveNumber*12 + getTranspose() + i); }      
      delay(debounceDelay);    
    }
  }  


  lastButtonSustainState = buttonSustainState;  
  
  for (int i = 0; i < keyCount; i++) { 
    lastButtonState[i] = buttonState[i];
  }
  

  
}
