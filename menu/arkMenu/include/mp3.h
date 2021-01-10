#ifndef MP3_H
#define MP3_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspmp3.h>
#include <pspaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <psputility.h>
#include <psputility_avmodules.h>

void playMP3File(char* filename, void* buffer, int buffer_size);

class MP3{

	private:
		char* filename;
		void* buffer;
		int buffer_size;
		int file_handle;
		int mp3_handle;
		
		static int playThread(SceSize _args, void *_argp);
		
	public:
		MP3(void* buffer, int size);
		MP3(char* filename, bool to_buffer=false);
		~MP3();
		
		void* getBuffer();
		int getBufferSize();

		void play();
		void stop();
};

#endif
