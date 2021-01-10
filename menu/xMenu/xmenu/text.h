#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <time.h>
#include "common.h"
#include "../graphics/graphics.h"
#include <intraFont.h>

// Colors
enum colors {
	RED =	0xFF0000FF,
	GREEN =	0xFF00FF00,
	BLUE =	0xFFFF0000,
	WHITE =	0xFFFFFFFF,
	LITEGRAY = 0xFFBFBFBF,
	GRAY =  0xFF7F7F7F,
	DARKGRAY = 0xFF3F3F3F,		
	BLACK = 0xFF000000,
};

using namespace std;

class TextAnim{

	private:

		float scroll;
		string glowText;
		string scrollText;
		
	public:
	
		TextAnim(string glowText, string scrollText);
		~TextAnim();
		
		void draw(float y);
		
};

#endif
