/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
const int powerPortCount = 17;
int powerPorts[powerPortCount] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38 };

const int redPortCount = 6;
int redPorts[redPortCount] = { 39, 42, 45, 48, 51, 8 };

const int greenPortCount = 6;
int greenPorts[greenPortCount] = { 40, 43, 46, 49, 52, 9 };

const int bluePortCount = 6;
int bluePorts[bluePortCount] = { 41, 44, 47, 50, 53, 10 };

// the setup routine runs once when you press reset:
void setup() {                
}

// the loop routine runs over and over again forever:
void loop() {
  for (int i = 0; i < powerPortCount; ++i)
  {
    pinMode(powerPorts[i], OUTPUT);  
    digitalWrite(powerPorts[i], HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);               // wait for a second
    digitalWrite(powerPorts[i], LOW);    // turn the LED off by making the voltage LOW
    pinMode(powerPorts[i], INPUT);  
  }
}
