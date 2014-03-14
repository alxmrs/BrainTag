/**
 * IR Receiver
 * This is based on David Cuartielles's code (which is based on Paul Malmsten's code) from
 * the following Arduino Forum thread in 2007: http://goo.gl/UxXAMd
 * 
 * In this update:
 * - Renamed variables so they wouldn't conflict with the variables in the main BrainTag file.
 *  
 * The receiver facing you:
 *  Connect the left pin to pin 0 on the Arduino (with 220 ohm resistor)
 *  Connect the middle pin to GND
 *  Connect the right pin to 5V
 */
 
#define numlifePins 5

const boolean debug = true;

int led_pin = 3;                    // "Ready to Receive" flag, not needed but nice
int sgPin =   13;

int start_bit_in = 2000;               // Start bit threshold (Microseconds)
int bin_1_in =     1000;               // Binary 1 threshold (Microseconds)
int bin_0_in =     400;                // Binary 0 threshold (Microseconds)

int health = 100;
int i;                              // All purpose counter 

byte lifePins[5] = {3, 4, 5, 7, 8};
byte lifePWR[5]  = {0, 0, 0, 0, 0};

/*
#define btnPower   149
#define btnMute    148
#define btnDown    147
#define btnUp      146
*/

void setup() {
  // Input
  pinMode(irRec, INPUT);
  
  
  // Output
  pinMode(sgPin, OUTPUT);  //This shows when we're ready to receive
  pinMode(lifePins[0], OUTPUT); pinMode(lifePWR[0], OUTPUT);
  pinMode(lifePins[1], OUTPUT); pinMode(lifePWR[1], OUTPUT);
  pinMode(lifePins[2], OUTPUT); pinMode(lifePWR[2], OUTPUT);
  pinMode(lifePins[3], OUTPUT); pinMode(lifePWR[3], OUTPUT);
  pinMode(lifePins[4], OUTPUT); pinMode(lifePWR[4], OUTPUT);
  digitalWrite(led_pin, LOW);
  
   
 Serial.begin(9600);
}

void loop() {
   healthMeter(health);
 int key = getIRKey();             //Fetch the key
 if (key != -1) {
   Serial.print("Key Recieved: ");
   Serial.println(key);
 }

 switch (key){
  case 1464:
    health -= 5;
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
 while(pulseIn(irRec, LOW) < 2200) { //Wait for a start bit
 }
 data[0]  = pulseIn(irRec, LOW);       //Start measuring bits, I only want low pulses
 data[1]  = pulseIn(irRec, LOW);
 data[2]  = pulseIn(irRec, LOW);
 data[3]  = pulseIn(irRec, LOW);
 data[4]  = pulseIn(irRec, LOW);
 data[5]  = pulseIn(irRec, LOW);
 data[6]  = pulseIn(irRec, LOW);
 data[7]  = pulseIn(irRec, LOW);
 data[8]  = pulseIn(irRec, LOW);
 data[9]  = pulseIn(irRec, LOW);
 data[10] = pulseIn(irRec, LOW);
 data[11] = pulseIn(irRec, LOW);
 digitalWrite(led_pin, LOW);

 if(debug == 1) {
   Serial.println("-----");
 };
 for(i=0; i<= 11; i++) {                 //Parse them
   if (debug == 1) {
         Serial.println(data[i]);
   }
   if(data[i] > bin_1_in) {                 //is it a 1?
       data[i] = 1;
   }  else {
       if(data[i] > bin_0_in) {           //is it a 0?
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


/** healthMeter
 *
 *
 */
void healthMeter(int health){
  if(health <= -1) //ledBlink(rPin,2,200);
  lifePWR[0] = map(constrain(health, 00, 20),  00, 20,  0, 255);
  lifePWR[1] = map(constrain(health, 21, 40),  21, 40,  0, 255);
  lifePWR[2] = map(constrain(health, 41, 60),  41, 60,  0, 255);
  lifePWR[3] = map(constrain(health, 61, 80),  61, 80,  0, 255);
  lifePWR[4] = map(constrain(health, 81, 100), 81, 100, 0, 255);
  analogWrite(lifePins[0],lifePWR[0]);
  analogWrite(lifePins[1],lifePWR[1]);
  analogWrite(lifePins[2],lifePWR[2]);
  analogWrite(lifePins[3],lifePWR[3]);
  analogWrite(lifePins[4],lifePWR[4]);
} /* END healthMeter */

/* END OF FILE */
