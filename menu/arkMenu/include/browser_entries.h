// DO NOT INCLUDE THIS FILE OTHER THAN IN BROWSER.CPP!!!

class File : public Entry{

	protected:
		bool selected;
		string fileSize;

	public:
	
		File(){
		}
	
		File(string path){
			this->path = path;
			size_t lastSlash = path.rfind("/", string::npos);
			this->name = path.substr(lastSlash+1, string::npos);
			this->selected = false;
			this->calcSize();
		}
		
		File(File* orig){
			this->path = orig->path;
			this->selected = false;
			this->fileSize = orig->fileSize;
		}
		
		~File(){
		}
		
	void calcSize(){
		// Calculate the size (in Bytes, KB, MB or GB) of a file, if it's a folder, simply return its type
		FILE* fp = fopen(this->getPath().c_str(), "rb");
		fseek(fp, 0, SEEK_END);
		unsigned size = ftell(fp);
		fclose(fp);
	
		ostringstream txt;
	
		if (size < 1024)
			txt<<size<<" Bytes";
		else if (1024 < size && size < 1048576)
			txt<<float(size)/1024.f<<" KB";
		else if (1048576 < size && size < 1073741824)
			txt<<float(size)/1048576.f<<" MB";
		else
			txt<<float(size)/1073741824.f<<" GB";
		this->fileSize = txt.str();
	}
		
		bool isSelected(){
			return this->selected;
		}
		
		void changeSelection(){
			this->selected = !this->selected;
		}
		
		string getPath(){
			return this->path;
		}
		
		string getName(){
			return this->name;
		}
		
		string getSize(){
			return this->fileSize;
		}
		
		char* getType(){
			return "FILE";
		}
		
		char* getSubtype(){
			return getType();
		}
		
		void loadIcon(){
		}
		
		void getTempData1(){
		}
		
		void getTempData2(){
		}
		
		void execute(){
		}
};

class Folder : public File{
	public:
		Folder(string path){
			this->path = path;
			size_t lastSlash = path.rfind("/", path.length()-2);
			this->name = path.substr(lastSlash+1, string::npos);
			this->selected = false;
			this->fileSize = "Folder";
		}
		
		Folder(Folder* orig){
			this->path = orig->path;
			this->name = orig->name;
			this->selected = false;
			this->fileSize = "Folder";
		}
		
		~Folder(){
		}
		
		char* getType(){
			return "FOLDER";
		}
		
		char* getSubtype(){
			return getType();
		}
};
