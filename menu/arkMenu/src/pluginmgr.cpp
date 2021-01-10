#include "pluginmgr.h"
#include <sstream>
#include <fstream>
#include <algorithm>

#define PAGE_SIZE 10

PluginsManager::PluginsManager(){
	this->entries = NULL;
	this->plugins = new vector<Plugin*>();
	this->specifics = new vector<Plugin*>();
	this->index = 0;
	this->start = 0;
	loadFromArkMenuPlugins();
	loadFromSePlugins();
}

PluginsManager::~PluginsManager(){
	plugins->clear();
	delete plugins;
}
		
void PluginsManager::writeFiles(Entry* e){

	writeArkMenuPlugins();

	if (common::getConf()->plugins){
		writePluginsTXT(e);
	}
	else {
		sceIoRemove("PLUGINS.TXT");
	}
}

void PluginsManager::writePluginsTXT(Entry* e){
	entries = new vector<Plugin*>();
	
	for (int i=0; i<plugins->size(); i++)
		entries->push_back(new Plugin(plugins->at(i)));
	
	for (int i=0; i<specifics->size(); i++){
		if (specifics->at(i)->getType() != e->getName())
			continue;
	
		bool found = false;
		for (int j=0; j<entries->size(); j++){
			if (entries->at(j)->getPath() == specifics->at(i)->getPath() &&
					entries->at(j)->getCategory() == specifics->at(i)->getCategory()){
				found = true;
				entries->at(j)->setType(specifics->at(i)->getType());
				entries->at(j)->setState(specifics->at(i)->enabled());
				break;
			}
		}
		if (!found)
			entries->push_back(new Plugin(specifics->at(i)));
		
	}
	
	ofstream fp;
	fp.open("PLUGINS.TXT");
	
	for (int i=0; i<entries->size(); i++){
		entries->at(i)->write(&fp);
		delete entries->at(i);
	}
	delete entries;
}

void PluginsManager::writeArkMenuPlugins(){
	
	if (plugins->size() == 0)
		return;
	
	ofstream fp;
	
	sceIoMkdir("ms0:/seplugins", 0777);
	fp.open("ms0:/seplugins/arkmenuplugins.txt");
	
	for (int i=0; i<plugins->size(); i++){
		Plugin* p = plugins->at(i);
		fp<<"["<<p->getType()<<"]";
		p->write(&fp);
	}
	for (int i=0; i<specifics->size(); i++){
		Plugin* p = specifics->at(i);
		fp<<"["<<p->getType()<<"]";
		p->write(&fp);
	}
	fp.close();
}

bool PluginsManager::loadFromArkMenuPlugins(){
	ifstream fp;
	fp.open("ms0:/seplugins/arkmenuplugins.txt");
	
	if (fp.is_open()){
		string line;
		while (true) {
			getline(fp, line);
			
			if (line.length() == 0)
				break;
			if (line[0] != '[')
				continue;
			
			int typeEnd = line.find("]");
			string type = line.substr(1, typeEnd-1);
			Plugin* p = new Plugin(line.substr(typeEnd+1, string::npos), type);
			
			if (p->parseOK())
				addPlugin(p);
			else
				delete p;
		}
		fp.close();
		return (plugins->size() > 0);
	}
	return false;
}

void PluginsManager::loadFromSePlugins(){
	struct {
		char* file;
		char* type;
	} files[2] = {
		{"ms0:/seplugins/game.txt", "game"},
		{"ms0:/seplugins/pops.txt", "pops"}
	};
	for (int i=0; i<2; i++){
		ifstream fp;
		fp.open(files[i].file);
	
		if (fp.is_open()){
			string line;
			while (true) {
				getline(fp, line);
			
				if (line.length() == 0)
					break;
			
				Plugin* p = new Plugin(line, files[i].type, true);
			
				if (p->parseOK())
					addPlugin(p);
				else
					delete p;
			}
			fp.close();
		}
	}
}

void PluginsManager::addPlugin(Plugin* p, bool replace){
	vector<Plugin*>* vec = (p->getType() == string("global"))? plugins : specifics;
	for (int i=0; i<vec->size(); i++){
		if (vec->at(i)->operator==(p)){
			if (replace){
				Plugin* aux = vec->at(i);
				vec->erase(vec->begin()+i);
				delete aux;
				vec->push_back(p);
				return;
			}
			else{
				delete p;
				return;
			}
		}
	}
	vec->push_back(p);
}

void PluginsManager::buildEntries(Entry* e){

	if (e == NULL){
		entries = plugins;
		return;
	}

	vector<Plugin*>* new_entries = new vector<Plugin*>();
	
	if (specifics->size() > 0){
		for (int i=0; i<specifics->size(); i++){
			if (specifics->at(i)->getType() == e->getName())
				new_entries->push_back(specifics->at(i));
		}
	}
	
	for (int i=0; i<plugins->size(); i++){
		if (plugins->at(i)->enabled())
			continue;
		bool found = false;
		if (new_entries->size() > 0){
			for (int j=0; j<new_entries->size() && !found; j++){
				if (plugins->at(i)->getPath() == new_entries->at(j)->getPath())
					found = true;
			}
		}
		if (!found){
			Plugin* p = new Plugin(plugins->at(i));
			p->setType(e->getName());
			p->setCategory( (e->getSubtype() == string("POPS"))? "pops" : "game");
			new_entries->push_back(p);
		}
	}
	entries = new_entries;
}

void PluginsManager::flushEntries(Entry* e){
	if (e == NULL)
		return;

	if (entries->size() == 0)
		return;
	
	vector<Plugin*>* del = new vector<Plugin*>();
	
	for (int i=0; i<entries->size(); i++){
		if (entries->at(i)->enabled())
			addPlugin(entries->at(i), true);
		else
			delete entries->at(i);
	}
}

void PluginsManager::moveUp(){
	if (this->entries->size() == 0)
		return;
	if (this->index == 0)
		return;
	else if (this->index == this->start){
		this->index--;
		if (this->start>0)
			this->start--;
	}
	else
		this->index--;
	common::playMenuSound();
}

void PluginsManager::moveDown(){
	if (this->entries->size() == 0)
		return;
	if (this->index == (entries->size()-1))
		return;
	else if (this->index-this->start == PAGE_SIZE-1){
		if (this->index+1 < entries->size())
			this->index++;
		if (this->start+PAGE_SIZE < entries->size())
			this->start++;
	}
	else if (this->index+1 < entries->size())
		this->index++;
	common::playMenuSound();
}

void PluginsManager::control(Entry* e){

	this->index = 0;

	common::playMenuSound();

	buildEntries(e);
	
	Controller pad;
	
	bool changed = false;
	
	while (true){
		pad.update();
		
		if (pad.up()){
			this->moveUp();
		}
		else if (pad.down()){
			this->moveDown();
		}
			
		else if (pad.accept()){
			if (entries->size() > 0){
				entries->at(index)->switchState();
				changed = true;
				common::playMenuSound();
			}
		}
		else if (pad.decline()){
			common::playMenuSound();
			break;
		}
		else if (pad.square() && e == NULL){
			if (entries->size() > 0){
				common::playMenuSound();
				
				bool found = false;
				Plugin* p = new Plugin(entries->at(index));
				p->changeCategory();
				for (int i=0; i<entries->size(); i++){
					if (entries->at(i)->operator==(p)){
						found = true;
						break;
					}
				}
				delete p;
				
				if (!found){
					entries->at(index)->changeCategory();
					changed = true;
				}
			}
		}
		else if (pad.triangle() && e == NULL){
			index--;
			entries->erase(entries->begin()+index+1);
			common::playMenuSound();
		}
	}
	
	flushEntries(e);
	
	vector<Plugin*>* aux = entries;
	entries = NULL;
	if (aux != plugins)
		delete aux;
	
	if (changed)
		writeArkMenuPlugins();
	
	sceKernelDelayThread(100000);
}

void PluginsManager::draw(){

	if (entries == NULL)
		return;
	
	int w = 300;
	int h = 150;
	
	int x = (480-w)/2;
	int y = (272-h)/2;
	
	int yoffset = y+15;
	
	common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);

	if (entries->size() == 0){
		common::printText(x+10, yoffset, "No plugins found");
		return;
	}
	
	for (int i=this->start; i<min(this->start+PAGE_SIZE, (int)entries->size()); i++){
	
		string state = (entries->at(i)->enabled())? "on" : "off";
	
		if (i == index){
			common::printText(x+10, yoffset, entries->at(i)->getCategory().c_str(), LITEGRAY, SIZE_LITTLE, true);
			common::printText(x+50, yoffset, entries->at(i)->getPath().c_str(), LITEGRAY, SIZE_LITTLE, true, true);
			common::printText(x+280, yoffset, state.c_str(), LITEGRAY, SIZE_LITTLE, true);
		}
		else {
			string path = entries->at(i)->getPath();
			if (path.length() >= 35)
				path = path.substr(0, 32) + string("...");
			common::printText(x+10, yoffset, entries->at(i)->getCategory().c_str());
			common::printText(x+50, yoffset, path.c_str());
			common::printText(x+280, yoffset, state.c_str());
		}
		yoffset += 15;
	}
}

void PluginsManager::addNewPlugin(string path){

	if (!common::fileExists(path))
		return;

	bool found_game = false;
	bool found_pops = false;
	
	for (int i=0; i<plugins->size(); i++){
		Plugin* p = plugins->at(i);
		if (!stricmp(p->getPath().c_str(), path.c_str())){
			if (p->getCategory() == "game")
				found_game = true;
			else
				found_pops = true;
		}
	}
	if (found_game && found_pops)
		return;
		
	ostringstream plugin_line;
	if (found_game)
		plugin_line<<"pops,";
	else
		plugin_line<<"game,";
		
	plugin_line<<path<<","<<"1";
	
	plugins->push_back(new Plugin(plugin_line.str(), "global"));
	writeArkMenuPlugins();
}
