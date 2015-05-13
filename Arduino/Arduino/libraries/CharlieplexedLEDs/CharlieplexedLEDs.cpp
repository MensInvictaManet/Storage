#include <CharlieplexedLEDs.h>

#include <limits.h>
#include "Arduino.h"

#define pixelCount 100
#define powerPortCount 17
const int powerPorts[powerPortCount] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
#define redPortCount 6
const int redPorts[redPortCount] = { 39, 40, 41, 42, 43, 44 };
#define greenPortCount 6
const int greenPorts[greenPortCount] = { 45, 46, 47, 48, 49, 14 };
#define bluePortCount 6
const int bluePorts[bluePortCount] = { 15, 16, 17, 2, 3, 4 };

void CharlieplexedLEDs::setAllPinsToInput()
{
  for (int i = 0; i < powerPortCount; ++i)
  {
    pinMode(powerPorts[i], INPUT);
  }
  for (int i = 0; i < redPortCount; ++i)
  {
    pinMode(redPorts[i], INPUT);
  }
  for (int i = 0; i < greenPortCount; ++i)
  {
    pinMode(greenPorts[i], INPUT);
  }
  for (int i = 0; i < bluePortCount; ++i)
  {
    pinMode(bluePorts[i], INPUT);
  }
}

void CharlieplexedLEDs::setLED(int index, const char color)
{
	//  First version has the rows backwards, so invert the index
	int remainder = (index % 10);
	index -= remainder + remainder - 10 + 1;

  int Pi = index % powerPortCount;
  int Ri = index / powerPortCount;
  int Gi = index / powerPortCount;
  int Bi = index / powerPortCount;
  
  setColor(powerPorts[Pi], redPorts[Ri], greenPorts[Gi], bluePorts[Bi], color);
  resetPins(powerPorts[Pi], redPorts[Ri], greenPorts[Gi], bluePorts[Bi]);
}

//  NOTE: "pixels" needs to be of size X, where X is the correct number of LEDs
void CharlieplexedLEDs::showFrame(const char* pixels, const unsigned long milliseconds, const char overrideColor)
{
	unsigned long startTime = millis();
	unsigned long currentTime = startTime;

	while (milliseconds >= (currentTime = millis()) - startTime)
	{
		for (int i = 0; i < pixelCount; ++i)
		{
			if (pixels == NULL || pixels[i] <= LED_COLOR_NONE) continue;
			setLED(i, overrideColor == LED_COLOR_NONE ? pixels[i] : overrideColor);
		}
	}
}


void CharlieplexedLEDs::showMarquee(const char* pixels_1, const char* pixels_2, const unsigned long milliseconds, const char overrideColor_1, const char overrideColor_2)
{
	unsigned long startTime = millis();
	unsigned long currentTime = startTime;

	while (milliseconds >= (currentTime = millis()) - startTime)
	{
		//  Determine what 10% multiple we are through the time
		const unsigned long tenth = ((currentTime = millis()) - startTime) / (milliseconds / 10);
		const unsigned long opposite = 10 - tenth;

		for (int i = 0; i < pixelCount; ++i)
		{
			if (i % 10 >= tenth)
			{
				if (pixels_1 != NULL && pixels_1[i] > LED_COLOR_NONE)
				{
					setLED(i - tenth, overrideColor_1 == LED_COLOR_NONE ? pixels_1[i] : overrideColor_1);
				}
			}
			else
			{
			if (pixels_2 != NULL && pixels_2[i] > LED_COLOR_NONE)
				{
					setLED(i + opposite, overrideColor_2 == LED_COLOR_NONE ? pixels_2[i] : overrideColor_2);
				}
			}
		}
	}
}

void CharlieplexedLEDs::showLetter(const char letter, const unsigned long milliseconds, const char overrideColor)
{
	showFrame(BasicFont.getASCII(letter), milliseconds, overrideColor);
}

void CharlieplexedLEDs::showString(const char* letters, const unsigned long millisecondsPerLetter, const char overrideColor)
{
	int stringLength = strlen(letters);
	for (int i = 0; i < stringLength; ++i)
	{
		showFrame(BasicFont.getASCII(letters[i]), millisecondsPerLetter, overrideColor);
	}
}

void CharlieplexedLEDs::marqueeString(const char* letters, const unsigned long millisecondsPerLetter, const char overrideColor)
{
	int stringLength = strlen(letters);
	for (int i = 0; i < stringLength; ++i)
	{
		showMarquee((i == 0) ? NULL : BasicFont.getASCII(letters[i - 1]), BasicFont.getASCII(letters[i]), millisecondsPerLetter, overrideColor, overrideColor);
	}
}

void CharlieplexedLEDs::showAnimation(TestAnimations::AnimationIndex index, const unsigned long millisecondsPerFrame, const char overrideColor)
{
	int frameCount = -1;
	const char** frames;
	Animations.getAnimation(index, frames, frameCount);

	for (int i = 0; i < frameCount; ++i)
	{
		showFrame(frames[i], millisecondsPerFrame, overrideColor);
	}
}

void CharlieplexedLEDs::resetPins(const int P, const int R, const int G, const int B)
{
  setPinToInput(P);
  setPinToInput(R);
  setPinToInput(G);
  setPinToInput(B);
}

void CharlieplexedLEDs::setColor(const int P, const int R, const int G, const int B, const char color)
{
  switch (color)
  {
    case LED_COLOR_RED:
      setPinToHigh(P);
      setPinToLow(R);
      setPinToInput(G);
      setPinToInput(B);
      break;
      
    case LED_COLOR_GREEN:
      setPinToHigh(P);
      setPinToInput(R);
      setPinToLow(G);
      setPinToInput(B);
      break;
      
    case LED_COLOR_BLUE:
      setPinToHigh(P);
      setPinToInput(R);
      setPinToInput(G);
      setPinToLow(B);
      break;
      
    case LED_COLOR_YELLOW:
      setPinToHigh(P);
      setPinToLow(R);
      setPinToLow(G);
      setPinToInput(B);
      break;
      
    case LED_COLOR_MAGENTA:
      setPinToHigh(P);
      setPinToLow(R);
      setPinToInput(G);
      setPinToLow(B);
      break;
      
    case LED_COLOR_TEAL:
      setPinToHigh(P);
      setPinToInput(R);
      setPinToLow(G);
      setPinToLow(B);
      break;
      
    case LED_COLOR_WHITE:
      setPinToHigh(P);
      setPinToLow(R);
      setPinToLow(G);
      setPinToLow(B);
      break;
  }
}

void CharlieplexedLEDs::setPinToHigh(const int index)
{
  pinMode(index, OUTPUT);
  digitalWrite(index, HIGH);
}

void CharlieplexedLEDs::setPinToLow(const int index)
{
  pinMode(index, OUTPUT);
  digitalWrite(index, LOW);
}

void CharlieplexedLEDs::setPinToInput(const int index)
{
  pinMode(index, INPUT);
}