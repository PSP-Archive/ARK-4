#include "wave.h"
#include "gfx.h"
#include "common.h"
#include <ctime>

// reuse snow animation flake size
extern float flake_size[];

Waves :: Waves()
{
    this->step = 482;
    this->frameskip = 0;
    
    this->vertices = new vertex[481];
    this->vertices2 = new vertex[481];
    // generate the initial wave
    for (int i=0; i<481; i++){
        this->vertices[i].x = i+4;
        this->vertices[i].z = 0;
        this->vertices[i].color = WAVE_COLOR_OUTLINE;
        this->vertices[i].y = 160 + WAVE_AMPLITUDE * sin(480 + ((float)i/(float)100));
        
        this->vertices2[i].x = i+4;
        this->vertices2[i].z = 0;
        this->vertices2[i].color = WAVE_COLOR_OUTLINE;
        this->vertices2[i].y = 160 + WAVE_AMPLITUDE * sin(2*480 + ((float)i/(float)100) - 240);
    }

    for (int i=0; i<10; i++){
        bubbles[i].y = 273;
        bubbles[i].x = rand()%480;
        bubbles[i].color = rand()%3; // use color variable for size
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
        // move the wave one pixel to the left and calculate the new y coordinate for x=480
        memcpy(vertices, &vertices[1], 480*sizeof(vertex));
        vertices[480].y = 160 + WAVE_AMPLITUDE * sin(480 + ((float)step/(float)100));
        // for the second wave, move two pixels to the left, this makes both waves move at different rates
        memcpy(vertices2, &vertices2[2], 480*sizeof(vertex));
        vertices2[479].y = 160 + WAVE_AMPLITUDE * sin(2*480 + ((float)step/(float)100) - 240); // need to recalculate two pixels
        vertices2[480].y = 160 + WAVE_AMPLITUDE * sin(2*480 + ((float)(step+1)/(float)100) - 240);
        step++;
        frameskip = WAVES_FRAMESKIP;

        int r = rand();
        // process the bubbles
        for (int i=0; i<10; i++){
            if (bubbles[i].y > vertices[bubbles[i].x].y || bubbles[i].y > vertices2[bubbles[i].x].y){
                int sway = (r%3)-1; // move bubble 1 pixel to the left or right or don't move it at all
                r = r>>i;
                bubbles[i].x += sway;
                bubbles[i].y--; // move bubble one pixel up
                
            }
            else {
                // reset bubble when out of the waves
                if (bubbles[i].color > 0){
                    bubbles[i].color--; // make it gradually disappear
                    bubbles[i].y--; // move bubble one pixel up
                }
                else{
                    bubbles[i].y = 273;
                    bubbles[i].x = r%480;
                    bubbles[i].color = r%3;
                }
            }
        }
    }
};

void Waves :: draw()
{
    update();
    // we draw a slightly offcentered outline to reduce blockyness (simple antialiasing)
    sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices);
    sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices2);
    // fill the waves with color
    for (int i=0; i<480; i++){
        ya2d_draw_line(i, vertices[i].y-1, i, vertices[i].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i, vertices2[i].y-1, i, vertices2[i].y+1, WAVE_COLOR_OUTLINE);
        
        ya2d_draw_line(i, vertices[i].y, i, 272, WAVE_COLOR);
        ya2d_draw_line(i, vertices2[i].y, i, 272, WAVE_COLOR2);
    }
    // draw the bubbles
    for (int i=0; i<10; i++){
        intraFontSetStyle(common::getFont(), flake_size[bubbles[i].color], 0xafffffff, 0, 0.f, INTRAFONT_WIDTH_VAR);
        intraFontPrint(common::getFont(), bubbles[i].x, bubbles[i].y, ".");
    }
}
