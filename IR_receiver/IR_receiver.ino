/**
 * IR Receiver
 * This is based on David Cuartielles's code (which is based on Paul Malmsten's code) from
 * the following Arduino Forum thread in 2007: http://goo.gl/UxXAMd
 * 
 * The receiver facing you:
 *  Connect the left pin to pin 7 on the Arduino (with 220 ohm resistor)
 *  Connect the middle pin to GND
 *  Connect the right pin to 5V
 */

int ir_pin = 12;                         // Sensor pin 1 wired through a 220 ohm resistor
int led_pin = 13;                       // "Ready to Receive" flag, not needed but nice
//int led_pin2 = 10;
int magPins[3] = {5,9, 11};
int sgPin = 13;
boolean debug = true;                    // Serial connection must be started to debug
int start_bit = 2000;                   // Start bit threshold (Microseconds)
int bin_1 = 1000;                       // Binary 1 threshold (Microseconds)
int bin_0 = 400;                        // Binary 0 threshold (Microseconds)
int i;        // General counter
int mag = 100;

  byte ledPWR1;
  byte ledPWR2;
  byte ledPWR3;
/*
#define btnPower   149
#define btnMute    148
#define btnDown    147
#define btnUp      146
*/

void setup() {
 pinMode(sgPin, OUTPUT);             //This shows when we're ready to receive
 pinMode(magPins[0],OUTPUT); pinMode(magPins[1],OUTPUT); pinMode(magPins[2],OUTPUT);
 pinMode(ir_pin, INPUT);
 digitalWrite(led_pin, LOW);       //not ready yet
 Serial.begin(9600);
}

void loop() {
   intensityMeter(mag);
 int key = getIRKey();             //Fetch the key
 if (key != -1) {
   Serial.print("Key Recieved: ");
   Serial.println(key);
 }

 switch (key){
  case 1464:
    mag -= 5;
    digitalWrite(sgPin,LOW);
    delay(100);
   break;
  default:
 break; 
   
 }
}


int getIRKey() {
 int data[12];
 digitalWrite(led_pin, HIGH);     //Ok, i'm ready to recieve
 while(pulseIn(ir_pin, LOW) < 2200) { //Wait for a start bit
 }
 data[0] = pulseIn(ir_pin, LOW);       //Start measuring bits, I only want low pulses
 data[1] = pulseIn(ir_pin, LOW);
 data[2] = pulseIn(ir_pin, LOW);
 data[3] = pulseIn(ir_pin, LOW);
 data[4] = pulseIn(ir_pin, LOW);
 data[5] = pulseIn(ir_pin, LOW);
 data[6] = pulseIn(ir_pin, LOW);
 data[7] = pulseIn(ir_pin, LOW);
 data[8] = pulseIn(ir_pin, LOW);
 data[9] = pulseIn(ir_pin, LOW);
 data[10] = pulseIn(ir_pin, LOW);
 data[11] = pulseIn(ir_pin, LOW);
 digitalWrite(led_pin, LOW);

 if(debug == 1) {
   Serial.println("-----");
 }
 for(i=0;i<=11;i++) {                 //Parse them
   if (debug == 1) {
         Serial.println(data[i]);
   }
   if(data[i] > bin_1) {                 //is it a 1?
       data[i] = 1;
   }  else {
       if(data[i] > bin_0) {           //is it a 0?
         data[i] = 0;
       } else {
        data[i] = 2;                     //Flag the data as invalid; I don't know what it is!
       }
   }
 }

 for(i=0;i<=11;i++) {                 //Pre-check data for errors
   if(data[i] > 1) {
       return -1;                           //Return -1 on invalid data
   }
 }

 int result = 0;
  for(i = 0 ; i < 11 ; i++) //Convert data bits to integer
    if(data[i] == 1) result |= (1<<i);
    
 return result;                             //Return key number
}


/** intensityMeter
 *
 *
 */
void intensityMeter(int mag){
  if(mag <= -1) //ledBlink(rPin,2,200);
  byte ledPWR1 = map(constrain(mag, 00, 33), 00, 33, 0, 255);
  byte ledPWR2 = map(constrain(mag, 33, 66), 33, 66, 0, 255);
  byte ledPWR3 = map(constrain(mag, 66, 99), 66, 99, 0, 255);
  analogWrite(magPins[0],ledPWR1);
  analogWrite(magPins[1],ledPWR2);
  analogWrite(magPins[2],ledPWR3);
  
  
} /* END intensityMeter */
