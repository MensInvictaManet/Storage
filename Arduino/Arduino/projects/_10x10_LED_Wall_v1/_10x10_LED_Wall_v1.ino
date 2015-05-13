#include <CharlieplexedLEDs.h>

CharlieplexedLEDs charlieplexManager;

void setup()
{
  charlieplexManager.setAllPinsToInput();
  /*
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Serial Connection Established\n");
  */
}

void loop()
{
  //charlieplexManager.genericTest();
  const int testFrame1[100] =
  {
    0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 
    0, 0, 0, 0, -1, -1, 0, 0, 0, 0,
    0, 0, 0, 0, -1, -1, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0, 0, -1,
    -1, -1, 0, 0, 0, 0, 0, 0, -1, -1,
    -1, -1, 0, 0, 0, 0, 0, 0, -1, -1,
    -1, 0, 0, 0, 0, 0, 0, 0, 0, -1,
    0, 0, 0, 0, -1, -1, 0, 0, 0, 0,
    0, 0, 0, 0, -1, -1, 0, 0, 0, 0,
    0, 0, 0, -1, -1, -1, -1, 0, 0, 0
  };
  
 //charlieplexManager.showFrame(testFrame1, 1000);
  
  const int testFrame2[100] =
  {
    -1, -1, -1, 0, 0, 0, 0, -1, -1, -1,
    -1, -1, 0, -1, -1, -1, -1, 0, -1, -1,
    -1, 0, -1, -1, -1, -1, -1, -1, 0, -1,
    0, -1, -1, -1, -1, -1, -1, -1, -1, 0,
    0, -1, -1, -1, -1, -1, -1, -1, -1, 0,
    0, -1, -1, -1, -1, -1, -1, -1, -1, 0,
    0, -1, -1, -1, -1, -1, -1, -1, -1, 0,
    -1, 0, -1, -1, -1, -1, -1, -1, 0, -1,
    -1, -1, 0, -1, -1, -1, -1, 0, -1, -1,
    -1, -1, -1, 0, 0, 0, 0, -1, -1, -1
  };
  
  //charlieplexManager.showFrame(testFrame2, 1000);
  
  const int testFrame3[100] =
  {
    0, -1, 0, 1, -1, 1, 2, -1, 2, -1,
    -1, 0, -1, -1, 1, -1, -1, 2, -1, -1,
    0, -1, 0, 1, -1, 1, 2, -1, 2, -1,
    3, -1, 3, 4, -1, 4, 5, -1, 5, -1,
    -1, 3, -1, -1, 4, -1, -1, 5, -1, -1,
    3, -1, 3, 4, -1, 4, 5, -1, 5, -1,
    6, -1, 6, 0, -1, 0, 1, -1, 1, -1,
    -1, 6, -1, -1, 0, -1, -1, 1, -1, -1,
    6, -1, 6, 0, -1, 0, 1, -1, 1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };
  
  charlieplexManager.showFrame(testFrame3, 1000);
}
