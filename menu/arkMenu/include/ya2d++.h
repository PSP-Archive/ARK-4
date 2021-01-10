#ifndef YA2DPP_H
#define YA2dPP_H

#include "ya2d.h"

#define PNG_MAGIC 0x474E5089
#define JPG_MAGIC 0xD8FF
#define BMP_MAGIC 0x18204D42

using namespace std;

class Image{

	private:
		ya2d_texture* texture;
	
	public:
		bool is_system_image;
	
		Image();
		Image(ya2d_texture* tex);
		Image(const char* filename); // read PNG, JPG or BMP file into RAM
		Image(const char* filename, int place); // read PNG, JPG or BMP file into either RAM or VRAM
		Image(void* buffer); // read from a PNG or BMP buffer into RAM
		Image(void* buffer, int place); // read from a PNG or BMP buffer into either RAM or VRAM
		Image(void* buffer, unsigned long buffer_size); // read from a JPG buffer into RAM
		Image(void* buffer, unsigned long buffer_size, int place); // read from a JPG buffer into either RAM or VRAM
		Image(const char* filename, int place, SceOff offset); // read a PNG file within another file into either RAM or VRAM

		~Image();
		
		ya2d_texture* getTexture();
		
		void swizzle();
		void flush();
		
		void draw(int x, int y);
		void draw_centered(int x, int y);
		void draw_hotspot(int x, int y, int center_x, int center_y);
		void draw_scale(int x, int y, float scale_x, float scale_y);
		void draw_scale(int x, int y, int newWidth, int newHeight);
		void draw_rotate(int x, int y, float angle);
		void draw_rotate_hotspot(int x, int y, float angle, int center_x, int center_y);
		
		bool operator==(Image* other);
};

#endif
