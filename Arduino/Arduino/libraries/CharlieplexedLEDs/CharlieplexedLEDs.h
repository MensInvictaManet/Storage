#ifndef CharlieplexedLEDs_h
#define CharlieplexedLEDs_h

#include "ASCII_10x10.h"
#include "TestAnimations.h"

//  Basic Definitions
enum Colors
{
  LED_COLOR_NONE		= 0,
  LED_COLOR_RED      = 1,
  LED_COLOR_GREEN    = 2,
  LED_COLOR_BLUE     = 3,
  LED_COLOR_YELLOW   = 4,
  LED_COLOR_MAGENTA  = 5,
  LED_COLOR_TEAL     = 6,
  LED_COLOR_WHITE    = 7,
  LED_COLOR_COUNT    = 8
};

class CharlieplexedLEDs
{
public:
	void setAllPinsToInput();
	void setLED(int index, const char color);
	void showFrame(const char* pixels, const unsigned long milliseconds, const char overrideColor = LED_COLOR_NONE);
	void showLetter(const char letter, const unsigned long milliseconds, const char overrideColor = LED_COLOR_NONE);
	void showString(const char* letters, const unsigned long millisecondsPerLetter, const char overrideColor = LED_COLOR_NONE);
	void marqueeString(const char* letters, const unsigned long millisecondsPerLetter, const char overrideColor = LED_COLOR_NONE);
	void showAnimation(TestAnimations::AnimationIndex index, const unsigned long millisecondsPerFrame, const char overrideColor = LED_COLOR_NONE);

private:
	void showMarquee(const char* pixels_1, const char* pixels_2, const unsigned long milliseconds, const char overrideColor_1 = LED_COLOR_NONE, const char overrideColor_2 = LED_COLOR_NONE);

	void resetPins(const int P, const int R, const int G, const int B);
	void setColor(const int P, const int R, const int G, const int B, const char color);
	void setPinToHigh(const int index);
	void setPinToLow(const int index);
	void setPinToInput(const int index);

	ASCII_10x10 BasicFont;
	TestAnimations Animations;
};

#endif // CharlieplexedLEDs_h