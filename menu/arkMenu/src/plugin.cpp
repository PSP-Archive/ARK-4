#include "plugin.h"

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

Plugin::Plugin(string source, string type, bool se){
	if (se)
		parseSePlugin(source, type);
	else
		parsePlugin(source, type);
}

Plugin::Plugin(Plugin* other){
	this->parsed = other->parsed;
	this->type = other->type;
	this->category = other->category;
	this->path = other->path;
	this->state = other->state;
}

void Plugin::parsePlugin(string source, string type){
	this->parsed = false;
	
	this->type = type;
	
	bool catOk = false;
	bool pathOk = false;
	bool stateOk = false;
	
	string c, p, s;
	
	size_t delimiter = 0;
	size_t lastDelimiter = 0;
	
	delimiter = source.find(',');
	lastDelimiter = source.rfind(',');
	c = source.substr(0, delimiter);
	p = source.substr(delimiter+1, lastDelimiter);
	p = p.substr(0, p.find(','));
	s = source.substr(lastDelimiter+1, string::npos);
	
	catOk = parseCategoryString(c);
	pathOk = parsePathString(p);
	stateOk = parseBooleanString(s);
	
	this->parsed = (catOk && pathOk && stateOk);
}

void Plugin::parseSePlugin(string source, string type){
	this->parsed = false;
	
	this->type = "global";
	
	bool catOk = false;
	bool pathOk = false;
	bool stateOk = false;
	
	string p, s;
	
	size_t delimiter = source.find(' ');
	
	p = source.substr(0, delimiter);
	s = source.substr(delimiter+1, string::npos);
	
	catOk = parseCategoryString(type);
	pathOk = parsePathString(p);
	stateOk = parseBooleanString(s);
	
	this->parsed = (catOk && pathOk && stateOk);
}

Plugin::~Plugin(){
}

bool Plugin::parseCategoryString(string c){
	string validCategories[] = {"umdemu", "game", "pops"};
	
	trim(c);
	for (int i=0; i<3; i++){
		if (validCategories[i] == c){
			if (i == 0)
				c = "game";
			this->category = c;
			return true;
		}
	}
	return false;
}

bool Plugin::parsePathString(string p){
	trim(p);
	if (common::fileExists(p)){
		this->path = p;
		return true;
	}
	return false;
}

bool Plugin::parseBooleanString(string b){

	struct {
		string name;
		bool value;
	} booleanStrings[] = {
		{"on", true},
		{"off", false},
		{"1", true},
		{"0", false},
		{"true", true},
		{"false", false},
		{"enable", true},
		{"disable", false}
	};

	trim(b);
	for (int i=0; i<8; i++){
		if (booleanStrings[i].name == b){
			this->state = booleanStrings[i].value;
			return true;
		}
	}
	return false;
}

string Plugin::getType(){
	return type;
}

string Plugin::getLine(){
	ostringstream s;
	s<<category<<","<<path<<","<<state<<"\n";
	return s.str();
}

string Plugin::getPath(){
	return this->path;
}

string Plugin::getCategory(){
	return this->category;
}

void Plugin::write(ofstream* fp){
	*fp<<getLine();
}

void Plugin::switchState(){
	this->state = !this->state;
}

void Plugin::changeCategory(){
	this->category = (this->category == string("pops"))? "game" : "pops";
}

bool Plugin::parseOK(){
	return parsed;
}

bool Plugin::enabled(){
	return this->state;
}

void Plugin::setType(string type){
	this->type = type;
}

void Plugin::setCategory(string cat){
	this->category = cat;
}

void Plugin::setState(bool state){
	this->state = state;
}

bool Plugin::operator==(Plugin* other){
	return (this->type == other->type && !stricmp(this->path.c_str(), other->path.c_str()) && this->category == other->category);
}
