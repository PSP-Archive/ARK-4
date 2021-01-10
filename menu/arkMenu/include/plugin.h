#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "common.h"

using namespace std;

class Plugin {
	private:
		string type;
		string category;
		string path;
		bool state;
		bool parsed;
		
		bool parseCategoryString(string c);
		bool parsePathString(string p);
		bool parseBooleanString(string b);
		
		void parsePlugin(string source, string type);
		void parseSePlugin(string source, string type);
		
	public:
		Plugin(string source, string type, bool se=false);
		Plugin(Plugin* other);
		~Plugin();
		
		string getType();
		string getLine();
		string getPath();
		string getCategory();
		void write(ofstream* fp);
		void switchState();
		void changeCategory();
		bool parseOK();
		bool enabled();
		
		void setType(string type);
		void setCategory(string cat);
		void setState(bool state);
		
		bool operator==(Plugin* other);
};

#endif
