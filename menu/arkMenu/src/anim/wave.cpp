#include "wave.h"
#include "gfx.h"
#include "common.h"
#include <ctime>

Waves :: Waves()
{
	this->step = WAVE_STEP;
	this->amplitude = WAVE_AMPLITUDE;
	this->angle = WAVE_ANGLE;
	this->frameskip = 0;
	
	this->vertices = new vertex[481];
	this->vertices2 = new vertex[481];
	for (int i=0; i<481; i++){
		this->vertices[i].x = i+4;
		this->vertices[i].z = 0;
		this->vertices[i].color = WAVE_COLOR_OUTLINE;
		
		this->vertices2[i].x = i+4;
		this->vertices2[i].z = 0;
		this->vertices2[i].color = WAVE_COLOR_OUTLINE;
	}
};

Waves :: ~Waves()
{
	delete[] vertices;
};



void Waves :: update()
{
	if (frameskip){
		frameskip--;
	}
	else{
		srand(time(NULL));
		for (int i=0; i<481; i+=4){
			// we only calculate the y offset for every 4 pixels
			vertices[i].y = 160 + amplitude * sin(angle + ((float)i/(float)480));
			vertices2[i].y = 160 + amplitude * sin(angle +((float)(240-i)/(float)480));
			if (i>0){
				// for the rest of the pixels we calculate simple averages
				// this makes the menu go at 60fps :D
				vertices[i-2].y = (vertices[i].y + vertices[i-4].y)/2;
				vertices2[i-2].y = (vertices2[i].y + vertices2[i-4].y)/2;
				
				vertices[i-1].y = (vertices[i].y + vertices[i-2].y)/2;
				vertices2[i-1].y = (vertices2[i].y + vertices2[i-2].y)/2;
				
				vertices[i-3].y = (vertices[i-2].y + vertices[i-4].y)/2;
				vertices2[i-3].y = (vertices2[i-2].y + vertices2[i-4].y)/2;
			}
		}
		angle += step;
		frameskip = WAVES_FRAMESKIP;
	}
};

void Waves :: draw()
{
	update();
	// we draw a slightly offcentered outline to reduce blockyness (simple antialiasing)
	sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices);
	sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices2);
	for (int i=0; i<480; i++){
		ya2d_draw_line(i, vertices[i].y-1, i, vertices[i].y+1, WAVE_COLOR_OUTLINE);
		ya2d_draw_line(i, vertices2[i].y-1, i, vertices2[i].y+1, WAVE_COLOR_OUTLINE);
		
		ya2d_draw_line(i, vertices[i].y, i, 272, WAVE_COLOR);
		ya2d_draw_line(i, vertices2[i].y, i, 272, WAVE_COLOR2);
	}
}
