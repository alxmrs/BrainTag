/**
 *  +------------------------------------------------------+
 *  |      ____             _          ______              |
 *  |     / __ )_________ _(_)___     /_  __/___ _____ _   |
 *  |    / __  / ___/ __ `/ / __ \     / / / __ `/ __ `/   |
 *  |   / /_/ / /  / /_/ / / / / /    / / / /_/ / /_/ /    |
 *  |  /_____/_/   \__,_/_/_/ /_/    /_/  \__,_/\__, /     |
 *  |                                          /____/      |
 *  +------------------------------------------------------+
 *
 * Created: 19 July 2013 by Alex Rosengarten
 * Arduino Brain Library by Eric Mika, 2010
 * Last Update: 7 March 2014 
 *
 * Since the last version:
 * - Separated intensityMeter and determineIfFire
 * - Fixed main loop -- now it's not being used to test the receiver
 * - Added method headers. 
 */

#include <Brain.h>
#include "pitches.h"

// Set up the brain parser, pass it the hardware serial object you want to listen on.
Brain brain(Serial);

/* SETTINGS */
boolean DEBUG = true;

// Pin Settings
const byte irPin = 2;          // IR LED
const byte irPin2 = 8;         // laser
const byte rPin = 3;           // RGB LED Pins
const byte gPin = 5;
const byte bPin = 6;
const byte spPin =  7;          // Speaker Pin
const byte numMagPins = 3; 
const byte magPins[numMagPins] = {
  9, 10, 11};  // Magnitude indicators
const byte buttonPin = 12;     // Pushbutton pin
const byte sgPin = 13;         // Signal Quality Pin


const byte numStates = 3;                  // Default: Off, Attention, Meditation
// Possible States (Beta):      // 0 = off,   1 = attention,            2 = meditation, 
// 3 = Delta, 4 = Alpha (High and Low), 5 = Beta (high and low), 
// 6 = Gamma (Low and Mid)

//const byte sampleSize = 10;              // Number of values used to calculate [level/trigger] (Beta)
const int fireDuration = 500;              // Time (ms) IR led fires. 
//const float defaultFireThreshold = 80;   // A value between 0 - 100. Determines brink of impulse trigger.
const int IRout = 10;    // The IR key the headset will send out when the headset fires.
/* END SETTINGS */

// Flags
boolean enteredLoop     = false;
String headsetStatus;
boolean sampleFullFlag  = false;

// Other
byte i; 
byte j; 
int k; // General counter
byte toggleCount = -1;
byte eegState = -1; 
byte eegData[numStates] = {
};

// Arrays to store collected EEG data
byte medData[100] = {
}; 
medInd = 0;
byte attData[100] = {
}; 
attInd = 0;

// The sum of a series of incrementing integers {1, 2, 3, ..., n-1, n} is equal to n^2/2 + n/2.
// Divide this by sample size (n) and you get n/2 + 1/2. 
//long series_x = sampleSize/2+0.5;

//float topMagReached = defaultFireThreshold;
unsigned long waitTime = 0;
unsigned long fireTime = 0;
byte oldAttention;
byte oldMeditation;

// IR remote sender
int data[12];
int start_bit = 2400;                    // Start bit threshold (Microseconds)
int bin_1 = 1200;                        // Binary 1 threshold (Microseconds)
int bin_0 = 600;                         // Binary 0 threshold (Microseconds)
int dataOut = 0;
int guardTime = 300;

// Tones
int offTone       =  NOTE_F4;
int atnTone[]     = {
  NOTE_E6,  NOTE_G6,  NOTE_GS6};
int mdtnTone[]    = {
  NOTE_DS6, NOTE_D6,  NOTE_C6 };
int fireTone[]    = {
  NOTE_C7,  NOTE_D7};
int headsetOnTone =  NOTE_F2;

void setup() {
  Serial.begin(57600); // Baud rate

  // Inputs
  pinMode(buttonPin, INPUT);

  // Outputs
  pinMode(rPin, OUTPUT);      
  pinMode(gPin, OUTPUT);      
  pinMode(bPin, OUTPUT);      // RBG LED
  pinMode(magPins[0],OUTPUT); 
  pinMode(magPins[1],OUTPUT); 
  pinMode(magPins[2],OUTPUT); // Magnitude Indicators
  pinMode(sgPin,OUTPUT);      
  pinMode(irPin,OUTPUT);           // Signal Quality & Laser 
  pinMode(irPin2,OUTPUT);     
  pinMode(spPin,OUTPUT);           // IR LED and Speaker

  // Init game to off mode
  toggleState(toggleCount++); 

  // Debug tests
  if(DEBUG){

    Serial.println("Test: RGB led should turn green now.");     // RBG pin test
    ledBlink(gPin,1,500);

    Serial.println("Test: signal pin should turn on now.");     // Signal pin test
    ledBlink(sgPin,1,500);

    Serial.println("Test: intensity pins should turn on now."); // Intensity pin(s) test
    ledArrayBlink(magPins,1,500);

    Serial.println("Test: IR pin should turn on now.");         // IR pin  test
    ledBlink(irPin,1,500);

    Serial.println("Test: Laser pin should turn on now.");      // Laser pin test
    ledBlink(irPin2,1,500);

    Serial.println("Ready.");

  } /* END DEBUG */
} /* END SETUP */

/** main loop 
 *
 */
void loop() {

  // Gate to execute test once. 
  if(!enteredLoop){
    enteredLoop = true; 
    if(DEBUG) Serial.println("Entered Loop");   
  } 

  // Toggle Game Mode
  if(digitalRead(buttonPin) == HIGH) {
    delay(200);  
    toggleState(toggleCount++);  
    delay(200);
  } /* END TOGGLE GAME MODE */

  // Gather EEG Data whenever a packet of data is recieved from the headset. 
  if(brain.update()){

    // Collect EEG data 
    eegData[0] = 0; // Resting state is set to 0.
    eegData[1] = brain.readAttention();
    eegData[2] = brain.readMeditation();

    /* Note: To read more data, you have to uncomment these lines *AND* change the "numStates" variable in settings 
     *AND* add cases in the toggleGame method below. 
     eegData[3] = brain.readDelta();
     eegData[4] = brain.readTheta();
     eegData[5] = brain.readLowAlpha();
     eegData[6] = brain.readHighAlpha();
     eegData[7] = brain.readLowBeta();
     eegData[8] = brain.readHighBeta();
     eegData[9] = brain.readLowGamma();
     eegData[10] = brain.readMidGamma();  
     */

    // HEADSET STATUS: This informs the user about the quality of the signal
    if(brain.readSignalQuality() == 200){
      headsetStatus = "off"; // Headset or Reference Electrode is not on. 
      ledBlink(sgPin,1,800);

    } 
    else if (brain.readSignalQuality() != 200 && brain.readAttention() == 0) {
      headsetStatus = "noSig"; // Headset and Reference Electrode are on, but no signal is being picked up
      ledBlink(sgPin, 1, 600);

    } 
    else if (headsetStatus == "on" && eegData[1] == oldAttention && eegData[2] == oldMeditation) {
      // Headset is frozen or stuck: It recieves signal, but that signal is not new, probably due to a poor signal.
      headsetStatus = "stuck"; 
      ledBlink(sgPin, 1, 500);

    } 
    else {
      // Game on!
      headsetStatus = "on"; 
      digitalWrite(sgPin,HIGH);
      //tone(spPin,headsetOnTone,250);
    } /* END HEADSET STATUS */

    oldAttention   = eegData[1];   // Store old eeg data to test if brain actually updates. 
    oldMeditation  = eegData[2];

    if(DEBUG){
      Serial.print(brain.readSignalQuality()); 
      Serial.print(" ");
      Serial.print(brain.readAttention());     
      Serial.print(" ");
      Serial.println(brain.readMeditation());    //Serial.print(" LB:");
      // Serial.print(brain.readLowBeta());       Serial.print(" HB:");
      // Serial.print(brain.readHighBeta());      Serial.print(" LA:");
      // Serial.print(brain.readLowAlpha());      Serial.print(" HA:");
      // Serial.println(brain.readHighAlpha());
      Serial.println("Headset Status: " + headsetStatus);
    } /* END DEBUG */
  } /* END BRAIN UPDATE */


  // Visualize EEG Data via Magnitude Indicator Pins
  intensityMeter(eegData[eegState]);  // Default is three mag pins 

  // Method call to determine and execute a shot
  determineIfFire(eegData[eegState]);
  // Turn off
  if(millis() >= fireTime){ 
    digitalWrite(irPin,LOW);
    digitalWrite(irPin2,LOW);
  }

  // Cycle Toggle Count - so the game can keep track of unlimited game state changes.
  if(toggleCount == numStates) toggleCount = 0;     

} /* END LOOP */


/* METHODS */

/**
 *
 */
void 

/** ledArrayBlink
 * Blinks array of LEDS. 
 * @param pin[]      Array of LED pins to blink
 * @param numBlinks  Number of times to blink the lights
 * @param del        Delay/duration of time LEDs are on and off
 */
void ledArrayBlink(const byte pin[], int numBlinks, int del){
  unsigned long waitTime;
  k = 0;
  while(k < numBlinks){
    for(j = 0; j < numMagPins; j++){        // j is a byte since numMagPins is also a byte datatype
      digitalWrite(pin[j],HIGH);
    } /* END FOR LOOP */
    delay(del);

    for(j = 0; j < numMagPins; j++){
      digitalWrite(pin[j],LOW);
    } /* END FOR LOOP */
    delay(del); 

    k++;
  } /* END WHILE LOOP */

} /* END ledArrayBlink */

/** ledBlink
 * Blinks a single LED
 * @param pin        Single LED pin to blink
 * @param numBlinks  Number of times to blink the LED
 * @param del        Delay/duration of time LED is on and off
 */
void ledBlink(byte pin, int numBlinks, int del){
  k = 0;
  while(k < numBlinks){
    digitalWrite(pin,HIGH);
    delay(del);
    digitalWrite(pin,LOW);
    delay(del); 
    k++;
  } /* END WHILE LOOP */
} /* END ledBlink */

/** intensityMeter
 * Displays the magnitude of attention, meditation, etc. over an array of three LEDs. 
 * Partitions the magnitude into three parts. Magnitude value changes the brightness
 * of each LED. 
 * @param mag  Magnitude value, an int between 0 and 100. 
 */
void intensityMeter(int mag){
  if(mag < 0) ledBlink(rPin,2,200); // Blink red if negative. Something is broken.

  // Divide the input magnitude into three ranges. Map each range to a brightness val
  byte ledPWR1 = map(constrain(mag, 00, 33), 00, 33, 0, 255);
  byte ledPWR2 = map(constrain(mag, 33, 66), 33, 66, 0, 255);
  byte ledPWR3 = map(constrain(mag, 66, 99), 66, 99, 0, 255);

  // Display the brightness values over the LEDs
  analogWrite(magPins[0],ledPWR1);
  analogWrite(magPins[1],ledPWR2);
  analogWrite(magPins[2],ledPWR3);
} /* END intensityMeter */

/** determineIfFire
 * Firing mechanism for headset. Records current time and calculates time when headset
 * should stop firing (fire time) and the time when the headset can fire again (wait
 * time). If the current time is after fire time and before wait time is over, turn
 * off the IR LEDs. If the current time is greater than the wait time, the magnitude
 * is greater than a certain value, and the headset status is on, transmit a fire 
 * signal via the IR LED. 
 */
void determineIfFire(int mag){ // mag = attention/ medi level
  int currentTime = millis();

  // Turn off IR LED (wait)
  if(currentTime >= fireTime && currentTime < waitTime){ 
    digitalWrite(irPin,LOW);
    digitalWrite(irPin2,LOW);
  }

  // Turn on IR LED  (fire)
  if(mag >= getThreshold( eegState) && headsetStatus == "on" && currentTime >= waitTime){    // getThreshold(mag) gets the threshold

    // This transmits a signal like a TV remote
    oscillationWrite(irPin, start_bit);
    digitalWrite(irPin,HIGH); 
    delayMicroseconds(guardTime);
    oscillationWrite(irPin,bin_0);
    digitalWrite(irPin,HIGH); 
    delayMicroseconds(guardTime);
    oscillationWrite(irPin,bin_1);
    digitalWrite(irPin,HIGH); 
    delayMicroseconds(guardTime);

    // Calculate when the IR LED should stop firing and when it can fire again.
    fireTime = currentTime + fireDuration;
    waitTime = fireTime + 100;

    if(DEBUG){
      Serial.print("Fired! [mag] [key sent] :");
      Serial.println(mag); //Serial.print("  "); /Serial.println(key);
    }
  }

}









/**

 * @param eegState  Current game mode. Used to decide which array of collected EEG data to choose from.

 * @return          Threshold value, used to determine if headset fires.

 */

int getThreshold(int eegState){
    // to make a copy of what is recording and what to change
    int threshold ;
    byte data[100] = {
    };
    Ind = 0;

    if( eegState == 1 ) // measuring attention level

    {

        //create a copy of the attention lv data
        for(int i = 0 ;i < n ; i++)
            data[i]=attData[i];
        

    }

    else if ( eegState == 2 )

    {

        //create a copy of the attention lv data
        for(int i = 0 ;i < n ; i++)
            data[i]=medData[i];

    }
    // reorder from lowest to highest
    sort_lowtohigh(data, 0, 99);
    // calc the threshold value
    threshold = (data[10]*data[11])/2;
    return threshold;

    

}


/** sort_lowtohigh
 * Sort an array from lowest to highest.
 * @param data[]      Array of data to sort
 * @param low index controller
 * @param high index controller
 */
void sort_lowtohigh(int data[], int low, int high)
{
    int i, max, temp1;
    if (low == high) return;
    for(i = 0; i < high; i++){
        if (a[i] > max) {
            max = a[i];
            temp1 = a[high - 1];
            a[i] = temp1;
            a[high - 1] = max;
        }
    }
    selection_sort(a, 0, high - 1);
}







/** toggleState
 * Takes in a toggle state counter to determine which game mode to be in. By 
 * default there are three game states: Off, Attention, Mediation. Changes  
 * color indicator and plays a tone depending on state. 
 * @param t state counter
 */
void toggleState(byte t) {
  // Mod function to determine state from counter
  eegState = t % numStates;

  // Switch statement to execute state. 
  switch (eegState){
  case 0: // OFF
    digitalWrite(rPin,HIGH);    // Display RED
    digitalWrite(gPin,LOW);
    digitalWrite(bPin,LOW);

    tone(spPin, offTone, 750);  // Play OFF tone
    delay(975);
    noTone(spPin);

    if(DEBUG) Serial.println("State 0: Off - Red");
    break; 

  case 1: // ATTENTION
    digitalWrite(rPin,LOW);     // Display Green
    digitalWrite(gPin,HIGH);
    digitalWrite(bPin,LOW);

    for(i = 0; i < 3; i++){     // Play ATTENTION tone sequence
      tone(spPin, atnTone[i], 250);
      delay(325);
      noTone(spPin);
    } /* END FOR LOOP */

    if(DEBUG) Serial.println("State 1: Attention - Green");
    break;

  case 2: // MEDITATION
    digitalWrite(rPin,LOW);    // Display BLUE
    digitalWrite(gPin,LOW);
    digitalWrite(bPin,HIGH);

    for(i = 0; i < 3; i++){    // Play MEDITATION tone sequence
      tone(spPin, mdtnTone[i], 250);
      delay(325);
      noTone(spPin);
    } /* END FOR LOOP */

    if(DEBUG) Serial.println("State 2: Meditation - Blue");
    break;

  } /* END SWITCH CASE */

  if(DEBUG) {
    Serial.print("Toggle Count is ");
    Serial.print(t);
    Serial.print(" and mod(t,number of states) is: ");
    Serial.println(eegState);
  } /* END DEBUG */

} /* END toggleState */


/** sendIRKey + oscillationWrite
 * The next two methods were written by David Cuartielle (based on Paul Malmsetm's code), taken from 
 * the following Arduino Forum thread, originally posted in 2007: http://goo.gl/UxXAMd
 */
int sendIRKey(int dataOut) {
  //digitalWrite(sgPin, HIGH);     //Ok, i'm ready to send
  for (i=0; i<12; i++) {
    data[i] = dataOut>>i & 0xB1;   //encode data as '1' or '0'
  }

  // send startbit
  oscillationWrite(irPin, start_bit);
  // send separation bit
  digitalWrite(irPin, HIGH);
  delayMicroseconds(guardTime);

  // send the whole string of data
  for (i=11; i>=0; i--) {
    if (data[i] == 0) oscillationWrite(irPin, bin_0);
    else oscillationWrite(irPin, bin_1);
    // send separation bit
    digitalWrite(irPin, HIGH);
    delayMicroseconds(guardTime);
  }
  delay(20);
  return dataOut;                            //Return key number
} //

// this will write an oscillation at 38KHz for a certain time in useconds
void oscillationWrite(int pin, int time) {
  for(i = 0; i <= time/26; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(13);
    digitalWrite(pin, LOW);
    delayMicroseconds(13);
  }
}

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
 
 */

/* END OF FILE */


