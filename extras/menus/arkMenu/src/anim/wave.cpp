#include "wave.h"
#include "gfx.h"
#include "common.h"
#include <ctime>

Waves :: Waves()
{
    this->step = 482;
    this->frameskip = 0;
    
    this->vertices = new vertex[481];
    this->vertices2 = new vertex[481];
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
        memcpy(vertices, &vertices[1], 480*sizeof(vertex));
        memcpy(vertices2, &vertices2[2], 480*sizeof(vertex));
        vertices[480].y = 160 + WAVE_AMPLITUDE * sin(480 + ((float)step/(float)100));
        vertices2[479].y = 160 + WAVE_AMPLITUDE * sin(2*480 + ((float)step/(float)100) - 240);
        vertices2[480].y = 160 + WAVE_AMPLITUDE * sin(2*480 + ((float)(step+(rand()%3))/(float)100) - 240);
        step++;
        frameskip = WAVES_FRAMESKIP;
    }
};

void Waves :: draw()
{
    update();
    // we draw a slightly offcentered outline to reduce blockyness (simple antialiasing)
    sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices);
    sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 480, 0, vertices2);
    for (int i=0; i<480; i+=10){
        ya2d_draw_line(i, vertices[i].y-1, i, vertices[i].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i, vertices2[i].y-1, i, vertices2[i].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i, vertices[i].y, i, 272, WAVE_COLOR);
        ya2d_draw_line(i, vertices2[i].y, i, 272, WAVE_COLOR2);

        ya2d_draw_line(i+1, vertices[i+1].y-1, i+1, vertices[i+1].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+1, vertices2[i+1].y-1, i+1, vertices2[i+1].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+1, vertices[i+1].y, i+1, 272, WAVE_COLOR);
        ya2d_draw_line(i+1, vertices2[i+1].y, i+1, 272, WAVE_COLOR2);

        ya2d_draw_line(i+2, vertices[i+2].y-1, i+2, vertices[i+2].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+2, vertices2[i+2].y-1, i+2, vertices2[i+2].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+2, vertices[i+2].y, i+2, 272, WAVE_COLOR);
        ya2d_draw_line(i+2, vertices2[i+2].y, i+2, 272, WAVE_COLOR2);

        ya2d_draw_line(i+3, vertices[i+3].y-1, i+3, vertices[i+3].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+3, vertices2[i+3].y-1, i+3, vertices2[i+3].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+3, vertices[i+3].y, i+3, 272, WAVE_COLOR);
        ya2d_draw_line(i+3, vertices2[i+3].y, i+3, 272, WAVE_COLOR2);

        ya2d_draw_line(i+4, vertices[i+4].y-1, i+4, vertices[i+4].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+4, vertices2[i+4].y-1, i+4, vertices2[i+4].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+4, vertices[i+4].y, i+4, 272, WAVE_COLOR);
        ya2d_draw_line(i+4, vertices2[i+4].y, i+4, 272, WAVE_COLOR2);

        ya2d_draw_line(i+5, vertices[i+5].y-1, i+5, vertices[i+5].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+5, vertices2[i+5].y-1, i+5, vertices2[i+5].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+5, vertices[i+5].y, i+5, 272, WAVE_COLOR);
        ya2d_draw_line(i+5, vertices2[i+5].y, i+5, 272, WAVE_COLOR2);

        ya2d_draw_line(i+6, vertices[i+6].y-1, i+6, vertices[i+6].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+6, vertices2[i+6].y-1, i+6, vertices2[i+6].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+6, vertices[i+6].y, i+6, 272, WAVE_COLOR);
        ya2d_draw_line(i+6, vertices2[i+6].y, i+6, 272, WAVE_COLOR2);

        ya2d_draw_line(i+7, vertices[i+7].y-1, i+7, vertices[i+7].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+7, vertices2[i+7].y-1, i+7, vertices2[i+7].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+7, vertices[i+7].y, i+7, 272, WAVE_COLOR);
        ya2d_draw_line(i+7, vertices2[i+7].y, i+7, 272, WAVE_COLOR2);

        ya2d_draw_line(i+8, vertices[i+8].y-1, i+8, vertices[i+8].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+8, vertices2[i+8].y-1, i+8, vertices2[i+8].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+8, vertices[i+8].y, i+8, 272, WAVE_COLOR);
        ya2d_draw_line(i+8, vertices2[i+8].y, i+8, 272, WAVE_COLOR2);

        ya2d_draw_line(i+9, vertices[i+9].y-1, i+9, vertices[i+9].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+9, vertices2[i+9].y-1, i+9, vertices2[i+9].y+1, WAVE_COLOR_OUTLINE);
        ya2d_draw_line(i+9, vertices[i+9].y, i+9, 272, WAVE_COLOR);
        ya2d_draw_line(i+9, vertices2[i+9].y, i+9, 272, WAVE_COLOR2);
    }
}
