#include <CharlieplexedLEDs.h>

CharlieplexedLEDs charlieplexManager;

const unsigned long animationFrameTime = 2000;

void setup()
{
  charlieplexManager.setAllPinsToInput();
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Serial Connection Established\n");
  randomSeed(analogRead(0));
}

void loop()
{
  //charlieplexManager.setLED(0, 1);
  //for (char i = 0; i < 7; ++i)
  //{
  //  for (int j = 0; j < 100; ++j)
  //  {
  //    charlieplexManager.setAllPinsToInput();
  //    charlieplexManager.setLED(j, i);
  //    delay(1000);
  //  }
  //}
  //charlieplexManager.marqueeString("PRAISE THE HELIX!", 2500);
  charlieplexManager.showAnimation(TestAnimations::AID_COLOR_BURST, animationFrameTime);
}
