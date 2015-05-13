/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
//  Set the pins for the rows
int row1 = 7;
int row2 = 6;
int row3 = 5;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(row1, OUTPUT);    
  pinMode(row2, OUTPUT);     
  pinMode(row3, OUTPUT);      
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(row1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row2, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row3, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  
  digitalWrite(row1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row2, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  
  digitalWrite(row1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row2, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  
  digitalWrite(row1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row2, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(row3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(3000);               // wait for X seconds
}
