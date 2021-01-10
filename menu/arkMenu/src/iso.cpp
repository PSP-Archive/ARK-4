#include "iso.h"

using namespace std;

Iso :: Iso()
{
};

Iso :: Iso(string path)
{
	stream = NULL;
	this->path = path;
	size_t lastSlash = path.rfind("/", string::npos);
	this->name = path.substr(lastSlash+1, string::npos);
	this->icon0 = common::getImage(IMAGE_WAITICON);
};

Iso :: ~Iso()
{
	clear();
	if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
		delete this->icon0;
};

void Iso::loadIcon(){
	Image* icon = NULL;
	void* buffer = Iso::fastExtract(this->path.c_str(), "ICON0.PNG");
	if (buffer != NULL){
		icon = new Image(buffer, YA2D_PLACE_RAM);
		free(buffer);
	}
	if (icon == NULL)
		sceKernelDelayThread(50000);
	icon = (icon == NULL)? common::getImage(IMAGE_NOICON) : icon;
	icon->swizzle();
	this->icon0 = icon;
}

void Iso::getTempData1(){
	this->pic0 = NULL;
	this->pic1 = NULL;

	void* buffer = NULL;

	// grab pic0.png
	buffer = Iso::fastExtract(this->path.c_str(), "PIC0.PNG");
	if (buffer != NULL){
		this->pic0 = new Image(buffer, YA2D_PLACE_RAM);
		free(buffer);
		buffer = NULL;
	}

	// grab pic1.png
	buffer = Iso::fastExtract(this->path.c_str(), "PIC1.PNG");
	if (buffer != NULL){
		this->pic1 = new Image(buffer, YA2D_PLACE_RAM);
		free(buffer);
		buffer = NULL;
	}
}

void Iso::getTempData2(){
	this->icon1 = NULL;
	this->snd0 = NULL;
	this->at3_size = 0;
	this->icon1_size = 0;

	void* buffer = NULL;
	unsigned size;
	
	// grab snd0.at3
	buffer = Iso::fastExtract(this->path.c_str(), "SND0.AT3", &size);
	if (buffer != NULL){
		this->snd0 = buffer;
		this->at3_size = size;
		buffer = NULL;
		size = 0;
	}
	
	// grab icon1.pmf
	buffer = Iso::fastExtract(this->path.c_str(), "ICON1.PMF", &size);
	if (buffer != NULL){
		this->icon1 = buffer;
		this->icon1_size = size;
		buffer = NULL;
		size = 0;
	}
}

int Iso :: extractFile(const char * file, const char * destination)
{
	unsigned sector, size;
	
	if(!find(file, &sector, &size)){ //find file
		debugFile((string("could not find file in iso\n")+string(file)).c_str());
		return 0;
	}
		
	sector /= SECTOR_SIZE; //offset gets transformed into sector

	FILE * output = fopen(destination, "wb"); //open output file
	
	if(!output){
		debugFile("could not open output for writing: ");
		debugFile(destination);
		return 0;
	}

	bool abort = false;
		
	while(size && abort == false) 
	{
		unsigned char buffer[SECTOR_SIZE];
		unsigned copy_size = size > SECTOR_SIZE? SECTOR_SIZE: size; //calculate bytes count
		
		if(!seek(stream, sector * SECTOR_SIZE, SEEK_SET)) //seek to sector
		{
			if(read(buffer, copy_size, 1, stream)) //read bytes
				fwrite(buffer, copy_size, 1, output); //copy bytes
			else
				abort = true;
		}
		else
			abort = true;
	
		sector++;
		size -= copy_size;
	};
	
	fclose(output);
	
	return abort == false;
};

void Iso :: clear()
{
	if(stream)
		fclose(stream);

	stream = NULL;
};

int Iso :: open(const char * path)
{
	clear();

	stream = fopen(path, "rb");

	if(!stream)
		return 0;

	if(!load()) //try to load volume descriptor and path table
	{
		clear();
		return 0;
	};

	return 1;
};

int Iso :: load()
{
	if(seek(stream, SECTOR_SIZE * VOLUME_DESCRIPTOR_START_SECTOR, SEEK_SET)) //try to seek to pvd offset
		return 0;

	if(!read(&pvd, sizeof(primary_volume_descriptor), 1, stream)) //read pvd
		return 0;

	if(strncmp(PVD_MAGIC, pvd.magic, 5)) //check magic pvd string
		return 0;

	if(seek(stream, pvd.path_table_location_LSB * SECTOR_SIZE, SEEK_SET)) //seek to path table
		return 0;

	if(!read(&main_path_entry, sizeof(path_entry), 1, stream)) //read first path entry
		return 0;

	return 1;
};

int Iso :: find(const char * path, unsigned * store_offset, unsigned * store_size)
{
	if(!stream)
		return 0;

	return search_in_directory_entry(path, main_path_entry.location, store_offset, store_size); //start recursive search
};

int Iso :: search_in_directory_entry(const char * path, unsigned dir_entry_sector, unsigned * store_offset, unsigned * store_size)
{
	directory_record dr;
	char * folder_end = get_name_end(path);

	if(seek(stream, dir_entry_sector * SECTOR_SIZE, SEEK_SET)) //go to dir record offset
		return 0;

	if(!read(&dr, sizeof(directory_record), 1, stream)) //try to read first record
		return 0;

	unsigned bytes_read = 0;

	while(dr.length)
	{
		char * file_name = new char[dr.length_file_id + 1]; //allocate mem for name
		read(file_name, dr.length_file_id, 1, stream); //read name
		*(file_name + dr.length_file_id) = 0x0;

		if(!strcmp(file_name + dr.length_file_id - 2, ";1")) //fix the extra ";1" added by some software
			*(file_name + dr.length_file_id - 2) = 0x0;

		//is this the file/subdir we are searching?
		int found = !strncmp(file_name, path, (int)(folder_end - path));

		delete [] file_name; //we dont need the entry name anymore

		if(found) //names match
		{
			if(dr.flags & FLAG_DIRECTORY) //start searching in new dir
				return search_in_directory_entry(folder_end + 1, dr.location.little, store_offset, store_size);
			else //file was found omg!
			{
				//store results
				if(store_offset)
					* store_offset = dr.location.little * SECTOR_SIZE;

				if(store_size)
					* store_size = dr.data_length.little;

				return 1;
			};
		};

		//skip padding/system use
		if(seek(stream, dr.length - (sizeof(directory_record) + dr.length_file_id), SEEK_CUR))
			return 0;

		//check that dir entry does not exit the current sector
		bytes_read += dr.length;
		if(bytes_read + sizeof(directory_record) + 8 > SECTOR_SIZE)
		{
			seek(stream, SECTOR_SIZE - bytes_read, SEEK_CUR);
			bytes_read = 0;
		};

		if(!read(&dr, sizeof(directory_record), 1, stream)) //try to read next record
			break;
	};

	return 0;
};

void Iso::readFile(void* dst, unsigned offset, unsigned size){
	FILE* src = fopen(this->path.c_str(), "rb");
	fseek(src, offset, SEEK_SET);
	fread(dst, size, 1, src);
	fclose(src);
}

int Iso :: read(void * destination, unsigned size, unsigned count, void * p)
{
	return fread(destination, size, count, (FILE *)p);
};

int Iso :: seek(void * p, unsigned bytes, int flag)
{
	return fseek((FILE *)p, bytes, flag);
};

char * Iso :: get_name_end(const char * str)
{
	while(* str != '/' && * str)
		str++;

	return (char *)str;
};

void Iso::execute(){
	this->executeISO();
}

char* Iso::getType(){
	return "ISO";
}

char* Iso::getSubtype(){
	return getType();
}

bool Iso::isPatched(string path){
	if (!Iso::isISO(path.c_str()))
		return false;
	Iso* iso = new Iso(path);
	iso->open(path.c_str());
	int found = iso->find("PSP_GAME/SYSDIR/EBOOT.OLD");
	delete iso;
	return found;
}

bool Iso::isISO(const char* filename){
	return (common::getMagic(filename, 0x8000) == ISO_MAGIC);
}

void* Iso::fastExtract(const char* path, char* file, unsigned* size){
	FILE* fp = fopen(path, "rb");
	if (fp == NULL)
		return NULL;
	
	void* buffer = NULL;
	if (size != NULL)
		*size = 0;
	
	common::upperString(file);
	
	fseek(fp, 32926, SEEK_SET);
	unsigned dir_lba;
	fread(&dir_lba, 4, 1, fp);
	fseek(fp, 4, SEEK_CUR);
	
	unsigned block_size;
	fread(&block_size, 4, 1, fp);
	
	unsigned dir_start = dir_lba*block_size + block_size;
	
	fseek(fp, dir_start, SEEK_SET);

	unsigned search_end = ftell(fp) + 2048;
	
	while (true){
		
		unsigned cur_pos = ftell(fp);
		
		unsigned char entry_size;
		fread(&entry_size, 1, 1, fp);
		
		if (entry_size == 0 || ftell(fp) >= search_end){
			fclose(fp);
			return NULL;
		}

		fseek(fp, cur_pos+32, SEEK_SET);
		
		unsigned char name_size;
		fread(&name_size, 1, 1, fp);
		
		char entry_name[256];
		memset(entry_name, 0, sizeof(entry_name));
		fread(entry_name, 1, name_size, fp);
		common::upperString(entry_name);
		
		if (!strcmp(entry_name, file)){
			fseek(fp, cur_pos + 2, SEEK_SET);
			unsigned icon0_offset;
			fread(&icon0_offset, 4, 1, fp);
			icon0_offset *= block_size;
			
			fseek(fp, ftell(fp) + 4, SEEK_SET);
			unsigned icon0_size;
			fread(&icon0_size, 4, 1, fp);
			
			if (icon0_size == 0){
				fclose(fp);
				return NULL;
			}
			
			fseek(fp, icon0_offset, SEEK_SET);
			
			buffer = malloc(icon0_size);
			memset(buffer, 0, icon0_size);
			if (size != NULL)
				*size = icon0_size;
			fread(buffer, icon0_size, 1, fp);
			
			fclose(fp);
			
			return buffer;
		}
		
		fseek(fp, cur_pos + entry_size, SEEK_SET);
	}
}
