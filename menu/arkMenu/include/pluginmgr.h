#ifndef PLUGINSMGR_H
#define PLUGINSMGR_H

#include <vector>
#include "entry.h"

#include "plugin.h"

/*

[Global]game/pops, ms0:/..., on/1/true/enable
[Global]game/pops, ms0:/..., on/1/true/enable
[Game name]game/pops, ms0:/..., on/1/true
.
.
.

*/

class PluginsManager {

	private:
		vector<Plugin*>* plugins;
		vector<Plugin*>* specifics;
		
		vector<Plugin*>* entries;
		
		int index;
		int start;
		
		bool loadFromArkMenuPlugins();
		void loadFromSePlugins();
		
		void addPlugin(Plugin* p, bool replace = false);
		
		void writePluginsTXT(Entry* e);
		
		void writeGlobals(ofstream* fp);
		void writeSpecifics(string name, ofstream* fp);
		void writeArkMenuPlugins();
		
		void moveUp();
		void moveDown();
		
		void buildEntries(Entry* e);
		void flushEntries(Entry* e);
	
	public:
		PluginsManager();
		~PluginsManager();
		
		void draw();
		void control(Entry* e = NULL);
		
		void addNewPlugin(string path);
		void writeFiles(Entry* e);
};

#endif
