/**
 * IR Receiver
 * This is based on David Cuartielles's code (which is based on Paul Malmsten's code) from
 * the following Arduino Forum thread in 2007: http://goo.gl/UxXAMd
 * 
 * In this update:
 * - Deleted a lot of extra code that made this program messy. 
 * - Adapted to work with prototype IR reciever. 
 *  
 * The receiver facing you:
 *  Connect the left pin to pin 0 on the Arduino (with 220 ohm resistor)
 *  Connect the middle pin to GND
 *  Connect the right pin to 5V
 */
 
#define numMagPins 5

const boolean debug = true;
int ir_pin =  7;                    // Sensor pin 1 wired through a 220 ohm resistor
int led_pin = 3;                    // "Ready to Receive" flag, not needed but nice
int sgPin =   13;
int start_bit = 2000;               // Start bit threshold (Microseconds)
int bin_1 =     1000;               // Binary 1 threshold (Microseconds)
int bin_0 =     400;                // Binary 0 threshold (Microseconds)
int mag = 100;
int i;                              // All purpose counter 

int magPins[5] = {3, 4, 5, 7, 8};
byte ledPWR[5] = {0, 0, 0, 0, 0};

/*
#define btnPower   149
#define btnMute    148
#define btnDown    147
#define btnUp      146
*/

void setup() {
  // Input
  pinMode(ir_pin, INPUT);
  
  
  // Output
  pinMode(sgPin, OUTPUT);  //This shows when we're ready to receive
  pinMode(magPins[0], OUTPUT); pinMode(ledPWR[0], OUTPUT);
  pinMode(magPins[1], OUTPUT); pinMode(ledPWR[1], OUTPUT);
  pinMode(magPins[2], OUTPUT); pinMode(ledPWR[2], OUTPUT);
  pinMode(magPins[3], OUTPUT); pinMode(ledPWR[3], OUTPUT);
  pinMode(magPins[4], OUTPUT); pinMode(ledPWR[4], OUTPUT);
  digitalWrite(led_pin, LOW);
  
   
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

/** getIRKey()
 * 
 */
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
 };
 for(i=0; i<= 11; i++) {                 //Parse them
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

 for(i = 0; i <= 11; i++) {                 //Pre-check data for errors
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
  ledPWR[0] = map(constrain(mag, 00, 20), 00, 20, 0, 255);
  ledPWR[1] = map(constrain(mag, 21, 40), 21, 40, 0, 255);
  ledPWR[2] = map(constrain(mag, 41, 60), 41, 60, 0, 255);
  ledPWR[3] = map(constrain(mag, 61, 80), 61, 80, 0, 255);
  ledPWR[4] = map(constrain(mag, 81, 100), 81, 100, 0, 255);
  analogWrite(magPins[0],ledPWR[0]);
  analogWrite(magPins[1],ledPWR[1]);
  analogWrite(magPins[2],ledPWR[2]);
  analogWrite(magPins[3],ledPWR[3]);
  analogWrite(magPins[4],ledPWR[4]);
} /* END intensityMeter */

/* END OF FILE */
