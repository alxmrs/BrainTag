/*
 */
 
#define numMagPins 1

int i = 0;
int start_bit = 2400;                    // Start bit threshold (Microseconds)
int bin_1 = 1200;                        // Binary 1 threshold (Microseconds)
int bin_0 = 600;                         // Binary 0 threshold (Microseconds)
int dataOut = 0;
int guardTime = 300;
int data[12];
byte irPin = 13;

/*
#define btnPower   149
#define btnMute    148
#define btnDown    147
#define btnUp      146
*/

void setup() {
  // Input

  // Output
  pinMode(irPin,OUTPUT);
  digitalWrite(irPin, LOW);
  
   
 Serial.begin(9600);
}

void loop() {
  // sendIRKey(1464);
    oscillationWrite(irPin, start_bit);
    digitalWrite(irPin,HIGH); delayMicroseconds(guardTime);
    oscillationWrite(irPin,bin_0);
    digitalWrite(irPin,HIGH); delayMicroseconds(guardTime);
    oscillationWrite(irPin,bin_1);
    digitalWrite(irPin,HIGH); delayMicroseconds(guardTime);
 }



int sendIRKey(int dataOut) {
  //digitalWrite(sgPin, HIGH);     //Ok, i'm ready to send
  for (i=0; i<12; i++) {
    data[i] = dataOut>>i & B1;   //encode data as '1' or '0'
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
  //delay(20);
  return dataOut;                            //Return key number
} //*/


void oscillationWrite(int pin, int time) {
  for(i = 0; i <= time/26; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(13);
    digitalWrite(pin, LOW);
    delayMicroseconds(13);
  }
}

/* END OF FILE */
