#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#define OPTIONS_CANCELLED -1

#include "common.h"

typedef struct {
	int value;
	char* name;
} t_options_entry;

class OptionsMenu {

	private:
		char* description;
		int n_options;
		t_options_entry* entries;
		int index;
		int x, y, w, h;
		
		int maxString();
	
	public:
		OptionsMenu(char* description, int n_options, t_options_entry* entries);
		~OptionsMenu();
		
		void draw();
		
		int control();

};

#endif
