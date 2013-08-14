/**
 *  +------------------------------------------------------+
 *  |      ____             _          ______              |
 *  |     / __ )_________ _(_)___     /_  __/___ _____ _   |
 *  |    / __  / ___/ __ `/ / __ \     / / / __ `/ __ `/   |
 *  |   / /_/ / /  / /_/ / / / / /    / / / /_/ / /_/ /    |
 *  |  /_____/_/   \__,_/_/_/ /_/    /_/  \__,_/\__, /     |
 *  |                                          /____/      |
 *  +------------------------------------------------------+
 * Created: 19 July 2013 by Alex Rosengarten
 * Arduino Brain Library by Eric Mika, 2010
 * Last Update: 14 August 2013
 *
 */

#include <Brain.h>
#include "pitches.h"

// Set up the brain parser, pass it the hardware serial object you want to listen on.
Brain brain(Serial);


/* SETTINGS */
boolean DEBUG = true;

// Pin Settings
const byte irPin = 2;          //  IR LED (or laser)
const byte rPin = 3;           //  RGB LED Pins
const byte gPin = 5;
const byte bPin = 6;
const byte speakerPin = 7;
const byte numMagPins = 3; const byte magPins[numMagPins] = {9, 10, 11};  // Magnitude indicators
const byte sgPin = 13;         // Signal Quality Pin
const byte buttonPin = 12;     // Pushbutton pin
  
const byte numStates = 3;               // Default: Off, Attention, Meditation

           // Possible States (Beta):   // 0 = off,   1 = attention,            2 = meditation, 
                                        // 3 = Delta, 4 = Alpha (High and Low), 5 = Beta (high and low), 
                                        // 6 = Gamma (Low and Mid)
                                        
const byte sampleSize = 10;             // Number of values used to calculate [level/trigger]
const int fireDuration = 200;           // Time (ms) IR led fires. 
const float defaultFireThreshold = 80;   // A value between 0 - 100. Determines brink of impulse trigger.
/* END SETTINGS */

// Flags
boolean enteredLoop     = false;
char headsetStatus;
boolean sampleFullFlag  = false;

// Sound
int allNotes[88] = {31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73, 78, 82, 87, 98, 104, 
                  110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 
                  311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 
                  880, 932, 1047, 1109, 1175, 1245, 1329, 1397, 1480, 1568, 1661, 1760, 1865, 1976, 2093, 2217,
                  2349, 2489, 2637, 2794, 2960, 3322, 3520, 3729, 3951, 4186, 4435, 4699, 4979
                  }; 
                  
int testMelody[10] = {};

// Other
byte toggleCount = -1;
byte eegState = -1; 
byte eegData[numStates] = {};
byte eegSample[sampleSize] = {}; byte sample_index = 0; 
  // The sum of a series {1, 2, 3, ..., n-1, n} is equal to n^2/2 + n/2.
  // Divide this by sample size (n) and you get n/2 + 1/2. 
  long avg_x = sampleSize/2+0.5;
  // 
byte oldAttention;
byte oldMeditation;
byte fadeCycle = 0;
byte fadeIncrement = 10;
float topMagReached = defaultFireThreshold;

void setup() {
  Serial.begin(9600); 
  
  // Inputs
  pinMode(buttonPin, INPUT);

  // Outputs
  pinMode(rPin, OUTPUT);      pinMode(gPin, OUTPUT);      pinMode(bPin, OUTPUT);      // RBG LED
  pinMode(magPins[0],OUTPUT); pinMode(magPins[1],OUTPUT); pinMode(magPins[2],OUTPUT); // Magnitude Indicators
  pinMode(sgPin,OUTPUT);      pinMode(speakerPin,OUTPUT);       // Signal Quality LED & Speaker output 
   
  if(DEBUG){
    // RBG pin test
    Serial.println("Test: RGB led should turn green now.");
    ledBlink(gPin,1,500);
    
    // Signal pin test
    Serial.println("Test: signal pin should turn on now.");
    ledBlink(sgPin,1,500);
    
    // Intensity pin(s) test
    Serial.println("Test: intensity pins should turn on now.");
    ledArrayBlink(magPins,1,500);
    
    // IR pin  test
    Serial.println("Test: IR pin should turn on now.");
    ledBlink(irPin,1,100);
    
    Serial.println("Test: Speaker");
    for (int i = 0; i < 88; i++){
      tone(speakerPin,allNotes[i],200);
      Serial.println(allNotes[i]);
      delay(1.3*200);
      noTone(speakerPin);
    }
    
    
    Serial.println("Ready.");
  } /* END DEBUG */
} /* END SETUP */

void loop() {
  if(enteredLoop == false && DEBUG){
    enteredLoop = true;  // Flag so that this condition is executed once
    toggleState(toggleCount++);
    Serial.println("Entered Loop");   
  } /* END DEBUG */
  
  // Toggle Game Mode via the Button Pin input
  if(digitalRead(buttonPin) == HIGH) {
    //delay(400);
    toggleState(toggleCount++);  
    delay(200);
  } /* END TOGGLE GAME MODE */
  
  // Gather EEG Data whenever a packet of data is recieved from the headset. 
  if(brain.update()){
    if(DEBUG){
      // Serial.println(brain.readCSV);        // <-- Uncomment this to see all raw data values
      Serial.print(brain.readSignalQuality()); Serial.print(" ");
      Serial.print(brain.readAttention());     Serial.print(" ");
      Serial.print(brain.readMeditation());    Serial.print(" LB:");
      Serial.print(brain.readLowBeta());       Serial.print(" HB:");
      Serial.print(brain.readHighBeta());      Serial.print(" LA:");
      Serial.print(brain.readLowAlpha());      Serial.print(" HA:");
      Serial.println(brain.readHighAlpha());
    } /* END DEBUG */
    
     // Collect EEG data 
    eegData[0] = 0; // Resting state is set to 0.
    eegData[1] = brain.readAttention();
    eegData[2] = brain.readMeditation();
    
 /* Note: To read more data, you have to uncomment these lines *AND* change the "numStates" variable in settings.
    eegData[3] = brain.readDelta();
    eegData[4] = brain.readTheta();
    eegData[5] = brain.readLowAlpha();
    eegData[6] = brain.readHighAlpha();
    eegData[7] = brain.readLowBeta();
    eegData[8] = brain.readHighBeta();
    eegData[9] = brain.readLowGamma();
    eegData[10] = brain.readMidGamma();  
    */
    
    // Headset Status: This informs the user about the quality of the signal
    if(brain.readSignalQuality() == 200){
      // Headset or Reference Electrode is not on. 
      ledBlink(sgPin, 1, 800);
      
      headsetStatus = 'off';
    } else if (brain.readSignalQuality() != 200 && brain.readAttention() == 0) {
      // Headset and Reference Electrode are on, but no signal is being picked up
      ledBlink(sgPin, 1, 600);
      headsetStatus = 'noSig';
      
    } else if (headsetStatus == 'on' && eegData[1] == oldAttention && eegData[2] == oldMeditation) {
      // Headset is frozen or stuck: It recieves signal, but that signal is not new, probably due to a poor signal.
      ledBlink(sgPin, 1, 500);
      headsetStatus = 'stuck';
      
    } else {
      // Game on!
      headsetStatus = 'on'; 
      digitalWrite(sgPin,HIGH);
    } /* END HEADSET STATUS */
    
    if(DEBUG) Serial.println(headsetStatus);
    if(headsetStatus == 'on') collectSample(eegData[eegState]); // This captures a few samples of eeg Data, used for trigger/level mechanism
  
    oldAttention   = eegData[1];   // Store old eeg data to test if brain actually updates. 
    oldMeditation  = eegData[2];
  } /* END BRAIN UPDATE */
  
    
  // Visualize EEG Data via Magnitude Indicator Pins
  intensityMeter(eegData[eegState]);  // Default is three mag pins 

   

    // Method call to determine if laser/IR LED should be triggered. 
    //determineIfFire(eegData[eegState]);
    

    //Cycle Toggle Count - I do this so the variable can be stored in a smaller datatype. 
    if(toggleCount == numStates) toggleCount = 0; 

    

} /* END LOOP */


/* METHODS */

/** determineIfFire(byte data)
 *
 */
void determineIfFire(byte data){
  float weight = random(-1,1);
  if(data >= topMagReached){
    topMagReached += (data - topMagReached) * weight;  // Incrementally adjust 
    ledBlink(sgPin,1,fireDuration);
  }
  Serial.print("Fired! Data was ");
  Serial.print(data);
  Serial.print("and topMagReached is now");
  Serial.println(topMagReached);  
  delay(500);  // Determines refractory period, or how long it takes until the next shot is possible. 
  
  /* I'm trying to figure out a more abstract way to asses the user's ability
     by processing the collected samples to determine rate of change from focus 
     to attention. I'll work on this later...
    int j;
    byte diff = data - topMagReached;
    byte weight = 
    if(data >= topMagReached){
      topMagReached += (data - topMagReached) * weight;  // Incrementally increases 
      blinkLED(irPin,1,fireDuration);
    }
  
  float yt, xt, t, df; 
  float syy=0.0, sxy = 0.0, sxx = 0.0, ay=0.0, ax=0.0;

  long adev = 0;
  long distFromMean
  for(int j = 0; j < sampleSize; j++) {
    sampleSum += eegSample[j];
  }
  long avg = sampleSum/sampleSize;
  // Calculate Average Deviation (or Mean Absolute Deviation)
  // This is equivalent to 1/n*(sum(abs(x_i - x_bar))|i = 1 to i = n)
  for(j = 0; j < sampleSize; j++){
    distFromMean = eegSample[j] - avg;
    if(distFromMean < 0) adev += distFromMean*-1;
    else                 adev += distFromMean;
    adev_x += j;
  }
  adev /= sampleSize;

  */
} /* END determineIfFire */


/** collectSample(byte data)
 *
 */
void collectSample(byte data){
  eegSample[sample_index++] = data; // Set current value to current eegSample index, then increment the index
  if (sample_index == sampleSize-1){
    sample_index == 0;  // This makes eegSample into a circular array
    sampleFullFlag = true;
    
  }
  if(DEBUG && sampleFullFlag) for(int i = 0; i<sampleSize;i++)  Serial.print(eegSample[i]);
  
}

void ledArrayBlink(const byte pin[], int numBlinks, int del){
  int i = 0;
  while(i < numBlinks){
    for(byte j = 0; j < numMagPins; j++){        // j is a byte since numMagPins is also a byte datatype
      digitalWrite(pin[j],HIGH);
    } /* END FOR LOOP */
    delay(del);
    
    for(byte j = 0; j < numMagPins; j++){
      digitalWrite(pin[j],LOW);
    } /* END FOR LOOP */
    delay(del); 
    
    i++;
  } /* END WHILE LOOP */    
} /* END ledArrayBlink */

void ledBlink(byte pin, int numBlinks, int del){
  int i = 0;
  while(i < numBlinks){
    digitalWrite(pin,HIGH);
    delay(del);
    digitalWrite(pin,LOW);
    delay(del); 
    i++;
  } /* END WHILE LOOP */
} /* END ledBlink */
 

void intensityMeter(int mag){
  byte ledPWR1 = map(constrain(mag, 00, 33), 00, 33, 0, 255);
  byte ledPWR2 = map(constrain(mag, 33, 66), 33, 66, 0, 255);
  byte ledPWR3 = map(constrain(mag, 66, 99), 66, 99, 0, 255);
  if(mag == -1) ledBlink(rPin,2,200);
  analogWrite(magPins[0],ledPWR1);
  analogWrite(magPins[1],ledPWR2);
  analogWrite(magPins[2],ledPWR3);
} /* END intensityMeter */

void intensityMeterGeneralCase(int mag){
  byte ledPWR[numMagPins];
  byte x1;  // the ith term of x
  byte x2;  // the ith + 1 term of x
  for(byte i = 1; i < numMagPins; i++){
    x1 = floor(100/numMagPins*i);
    x2 = floor(100/numMagPins*(i+1));
    ledPWR[i] = map(constrain(mag, x1, x2), x1, x2, 0, 255);
  } /* END FOR */
  if(mag == -1) ledBlink(rPin,2,200);
  for(byte i = 0; i < numMagPins; i++){
    analogWrite(magPins[i],ledPWR[i]);
  } /* END FOR */
} /* END intensityMeterGeneralCase */



void toggleState(byte t) {
  if(DEBUG) {
    Serial.print("Toggle Count is ");
    Serial.print(t);
    Serial.print(" and mod(t,number of states) is: ");
    Serial.println(t % numStates);
  } /* END DEBUG */
  
  
  eegState = t % numStates;
  
  switch (eegState){
    case 0: 
      digitalWrite(rPin,HIGH);
      digitalWrite(gPin,LOW);
      digitalWrite(bPin,LOW);
      if(DEBUG) Serial.println("State 0: Off - Red");
      break;
    case 1:
      digitalWrite(rPin,LOW);
      digitalWrite(gPin,HIGH);
      digitalWrite(bPin,LOW);
      if(DEBUG) Serial.println("State 1: Attention - Green");
      break;
    case 2:
      digitalWrite(rPin,LOW);
      digitalWrite(gPin,LOW);
      digitalWrite(bPin,HIGH);
      if(DEBUG) Serial.println("State 2: Meditation - Blue");
      break;
    case 3:
      digitalWrite(rPin,LOW);
      digitalWrite(gPin,LOW);
      digitalWrite(bPin,HIGH);
      if(DEBUG) Serial.println("State 3");
      break;
    case 4:
      analogWrite(rPin,255);
      analogWrite(gPin,6);
      analogWrite(bPin,0);
      if(DEBUG) Serial.println("State 4: Yellow");
      break;
    case 5:
      digitalWrite(rPin,LOW);
      digitalWrite(gPin,HIGH);
      digitalWrite(bPin,HIGH);
      if(DEBUG) Serial.println("State 5");
      break;
    case 6:
      digitalWrite(rPin,HIGH);
      digitalWrite(gPin,LOW);
      digitalWrite(bPin,HIGH);
      if(DEBUG) Serial.println("State 6");
      break;
    case 7: 
      digitalWrite(rPin,HIGH);
      digitalWrite(gPin,HIGH);
      digitalWrite(bPin,HIGH);
      if(DEBUG) Serial.println("State 7");
      break;
  } /* END SWITCH CASE */
} /* END toggleState */

/* END METHODS & PROGRAM */


/* 
          _                 _           _      
        / /\              / /\        /\ \    
       / /  \            / /  \      /  \ \   
      / / /\ \          / / /\ \__  / /\ \ \  
     / / /\ \ \        / / /\ \___\/ / /\ \_\ 
    / / /  \ \ \       \ \ \ \/___/ / /_/ / / 
   / / /___/ /\ \       \ \ \    / / /__\/ /  
  / / /_____/ /\ \  _    \ \ \  / / /_____/   
 / /_________/\ \ \/_/\__/ / / / / /\ \ \     
/ / /_       __\ \_\ \/___/ / / / /  \ \ \    
\_\___\     /____/_/\_____\/  \/_/    \_\/    

[ Alexander          Sky         Rosengarten ]
*/

