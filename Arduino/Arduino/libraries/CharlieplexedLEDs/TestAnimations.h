#ifndef TestAnimations_h
#define TestAnimations_h

#include <vector.h>

class TestAnimations
{
public:
	enum AnimationIndex
	{
		AID_COLOR_BURST = 0,
		ANIMATION_INDEX_COUNT
	};

	bool getAnimation(const int animationIndex, const char**& frames, int& frameCount);
};

#endif