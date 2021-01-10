#include "cso.h"

#include <cmath>
#include "systemctrl.h"


void readUnsignedFromMem(unsigned * store, void * address)
{
	unsigned char * p = (unsigned char *)store;
	*(p) = *(unsigned char *)address;
	*(p+1) = *(unsigned char *)((unsigned)address+1);
	*(p+2) = *(unsigned char *)((unsigned)address+2);
	*(p+3) = *(unsigned char *)((unsigned)address+3);
};

Cso :: Cso(string path)
{
	this->path = path;
	this->file = NULL;
	this->indices = NULL;
	this->data = NULL;
	this->read_buffer = NULL;
	size_t lastSlash = path.rfind("/", string::npos);
	this->name = path.substr(lastSlash+1, string::npos);
	this->icon0 = common::getImage(IMAGE_WAITICON);
	this->block_out = NULL;
};


Cso::~Cso(){
	if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
		delete this->icon0;
	free(this->block_out);
}


void Cso::loadIcon(){
	Image* icon = NULL;
	void* buffer = this->fastExtract(this->path.c_str(), "ICON0.PNG");
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


void Cso::getTempData1(){
	this->pic0 = NULL;
	this->pic1 = NULL;

	void* buffer = NULL;

	// grab pic0.png
	buffer = this->fastExtract(this->path.c_str(), "PIC0.PNG");
	if (buffer != NULL){
		this->pic0 = new Image(buffer, YA2D_PLACE_RAM);
		free(buffer);
		buffer = NULL;
	}

	// grab pic1.png
	buffer = this->fastExtract(this->path.c_str(), "PIC1.PNG");
	if (buffer != NULL){
		this->pic1 = new Image(buffer, YA2D_PLACE_RAM);
		free(buffer);
		buffer = NULL;
	}
}

void Cso::getTempData2(){
	this->icon1 = NULL;
	this->snd0 = NULL;
	this->at3_size = 0;
	this->icon1_size = 0;

	void* buffer = NULL;
	unsigned size;
	
	// grab snd0.at3
	buffer = this->fastExtract(this->path.c_str(), "SND0.AT3", &size);
	if (buffer != NULL){
		this->snd0 = buffer;
		this->at3_size = size;
		buffer = NULL;
		size = 0;
	}
	
	// grab icon1.pmf
	buffer = this->fastExtract(this->path.c_str(), "ICON1.PMF", &size);
	if (buffer != NULL){
		this->icon1 = buffer;
		this->icon1_size = size;
		buffer = NULL;
		size = 0;
	}
}

bool Cso :: open(const char * path)
{
	file = fopen(path, "rb");
	if(!file) return false;
	fread(&head, sizeof(cso_header), 1, file);
	
	if(head.magic!=CSO_MAGIC)
	{
		fclose(file);
		return false;
	};
	

	total_blocks = head.file_size/head.block_size;
	indices_len = 4 * (total_blocks+1);
	current_index = 0;

	this->indices = (unsigned *)malloc(indices_len);
	this->data = (unsigned char *)malloc(head.block_size*2);
	this->read_buffer = (unsigned char *)malloc(head.block_size*2);
	dec.zalloc = NULL;
	dec.zfree = NULL;
	dec.opaque = NULL;

	fread(this->indices, 1, indices_len, file);

	return true;
};

void Cso :: clear()
{
	if(file) fclose(file);
	if(indices) free(indices);
	if(data) free(data);
	if(read_buffer) free(read_buffer);
};

unsigned char * Cso :: getDataBlock(unsigned block)
{
	unsigned get_size, is_plain, read_position, compare_size;
	int status;
	
	if (sctrlHENGetVersion() < 0x2003){
		if(inflateInit2(&dec,-15) != Z_OK)
		{
			this->clear();
			return NULL;
		};
	}

	current_index = this->indices[block];
	is_plain = current_index & 0x80000000;
	current_index = current_index & 0x7fffffff;
	read_position = current_index << (head.align);

	if(!is_plain)
	{
		current_index2 = this->indices[block+1] & 0x7fffffff;
		get_size = (current_index2-current_index) << (head.align);
	}
	else get_size = head.block_size;

	fseek(file, read_position, SEEK_SET);	
	dec.avail_in = fread(this->read_buffer, 1, get_size, file);

	if(is_plain)
	{
		memcpy(this->data, this->read_buffer, get_size);
		compare_size = get_size;
	}
	else
	{
		if (sctrlHENGetVersion() < 0x2003){
			dec.next_out = this->data;
			dec.avail_out = head.block_size;
			dec.next_in = this->read_buffer;

			status = inflate(&dec, Z_FULL_FLUSH);
			compare_size = head.block_size - dec.avail_out;
			
			if((status != Z_STREAM_END) || (compare_size != head.block_size))
			{
				this->clear();
				return NULL;
			};
		}
		else {
			sctrlDeflateDecompress(this->data, this->read_buffer, head.block_size);
		}
	};
	
	if (sctrlHENGetVersion() < 0x2003){
		if(inflateEnd(&dec) != Z_OK)
		{
			this->clear();
			return NULL;
		};
	}

	return this->data;
};

unsigned Cso :: identifyEntry(const char * name, unsigned block, unsigned * fileSize)
{

	unsigned ret = 0;

	char * entry_name = NULL;

	unsigned char * ptr = this->data;

    dirRecord dr;

	

	this->getDataBlock(block);

	dr.length = *(char *)ptr;

    while(!ret && dr.length)

    {

        if(dr.length!=0x30)

        {

			readUnsignedFromMem(&dr.location, ptr+2);

			readUnsignedFromMem(&dr.size, ptr+10);

			dr.nameLen = *(char *)(ptr+32);


            entry_name = (char *)malloc(dr.nameLen+1);

			strncpy(entry_name, (char *)(ptr+33), dr.nameLen);

            *(entry_name+dr.nameLen)='\0';

			

            if(!strcmp(entry_name, name)) //File was found!

			{

                ret = dr.location;

				*fileSize = dr.size;

			};

            free(entry_name);

        };

		ptr+=dr.length;

        dr.length = *(char *)ptr;

    };

	return ret;

};


void Cso::readFile(void* dst, unsigned block, unsigned size){

	unsigned cont = 0;

	unsigned total_write;

	while(size>0)

	{

		if(size>=0x800) total_write = 0x800;

		else total_write = size;

		

		this->getDataBlock(block);

		memcpy((void*)((u32)dst+cont), this->data, total_write);

		size-=total_write;

		block++;

		cont+=total_write;

	}

}


void Cso :: extractFile(const char * name, unsigned block, unsigned size)

{

	FILE * b;

	b = fopen(name, "wb");

	unsigned total_write;

	while(size>0)

	{

		if(size>=0x800) total_write = 0x800;

		else total_write = size;

		

		this->getDataBlock(block);

		fwrite(this->data, total_write, 1, b);

		size-=total_write;

		block++;

	};

	fclose(b);

};


unsigned Cso :: findFile(const char * name, unsigned * fileSize)

{

	pathTable pt;

	this->getDataBlock(this->pvd.pathTableOffset);

	unsigned char * ptr = this->data;

	unsigned ret = 0;

	char * dir_name = NULL;

	int end = 0;

	*fileSize = 0;


	while(((unsigned)(ptr-this->data) < pvd.pathTableSize) && !end)

	{

		pt.length = *(char *)(ptr);

		readUnsignedFromMem(&pt.location, ptr+2);

		ptr+=8;

		dir_name = (char *)malloc(pt.length+1);

		strncpy(dir_name, (char *)ptr, pt.length);

		*(dir_name+pt.length) = '\0';

		

		if(!strncmp(name, dir_name, (int)(strrchr(name, '/')-name)))

		{

			ret = this->identifyEntry(strrchr(name, '/')+1, pt.location, fileSize);

			end = 1;

		};

		ptr+=pt.length;

		if(pt.length % 2) ptr++;

		free(dir_name);

	};

	return ret;

};


int Cso :: getPrimaryVolumeDescriptor()

{

	this->getDataBlock(0x10);

	this->pvd = *(primaryVolumeDescriptor *)(this->data+sizeof(volumeDescriptor)+0x7D);

	return 1;

};


void Cso::execute(){

	this->executeISO();

}


char* Cso::getType(){

	return "ISO";

}


char* Cso::getSubtype(){

	return "CSO";

}


bool Cso::isPatched(string path){

	// yes, this is a copy-paste of the icon0 code, I didn't even change the variable names, problem?

	if (!Cso::isCSO(path.c_str()))

		return false;

	Cso* iso = new Cso(path);

	iso->open(path.c_str());

	iso->getPrimaryVolumeDescriptor();

	unsigned cso_icon0_size, cso_icon0_off = iso->findFile("SYSDIR/EBOOT.OLD", &cso_icon0_size);

	iso->clear();

	delete iso;

	return cso_icon0_off;

}


bool Cso::isCSO(const char* filename){

	return (common::getMagic(filename, 0) == CSO_MAGIC);

}


void zlib_decompress(uint8_t *data, uint8_t* block_out, unsigned size)

{

	if (sctrlHENGetVersion() >= 0x2003){ // are we using the latest ARK-2?

		sctrlDeflateDecompress(block_out, data, SECTOR_SIZE); // use ARK-2's deflateDecompress function

	}

	else { // use zlib

		z_stream strm;

		strm.zalloc = Z_NULL;

		strm.zfree = Z_NULL;

		strm.opaque = Z_NULL;

		strm.avail_in = 0;

		strm.next_in = Z_NULL;


		inflateInit2(&strm, -15);


		strm.avail_in = size;

		strm.next_in = data;


		strm.avail_out = SECTOR_SIZE;

		strm.next_out = block_out;


		inflate(&strm, Z_FINISH);


		inflateEnd(&strm);

	}

}


void Cso::getInitialBlock(FILE* fp){

	if (fp == NULL)
		return;

	uint8_t* compressed;

	fseek(fp, 16, SEEK_SET);
	fread(&block_size, 4, 1, fp);
	start_read = (32768 / block_size) * 4 + 24;
	fseek(fp, start_read, SEEK_SET);

	unsigned fo, fs;
	fread(&fo, 4, 1, fp);
	fread(&fs, 4, 1, fp);
	fseek(fp, fo, SEEK_SET);

	fs = min((int)fs, 200);

	compressed = (uint8_t*)malloc(fs);
	fread(compressed, 1, fs, fp);

	zlib_decompress(compressed, block_out, fs);
	free(compressed);
	unsigned start = (block_out[158] + block_out[159] + block_out[160] + block_out[161]) * 4;
	fseek(fp, start+28, SEEK_SET);

	unsigned offset;
	fread(&offset, 4, 1, fp);

	unsigned size;
	fread(&size, 4, 1, fp);
	size -= offset;

	fseek(fp, offset, SEEK_SET);

	compressed = (uint8_t*)malloc(size);
	fread(compressed, 1, size, fp);
	zlib_decompress(compressed, block_out, size);
	free(compressed);

}


void* Cso::fastExtract(const char* path, char* file, unsigned* size_out){


	FILE* fp = fopen(path, "rb");

	if (fp == NULL)

		return NULL;


	if (block_out == NULL){

		this->block_out = (u8*)memalign(64, SECTOR_SIZE);

		this->getInitialBlock(fp);

	}

		

	if (size_out != NULL)

		*size_out = 0;

	

	void* buffer = NULL;

	uint8_t* compressed;

	

	common::upperString(file);

	

	int pos = 0;

	unsigned offset, size;

	

	while (true){

	

		if (pos > block_size){

			fclose(fp);

			return NULL;

		}

	

		char tmpText[128];

		memset(tmpText, 0, 128);

		strncpy(tmpText, (char*)&block_out[pos], strlen(file));

		common::upperString(tmpText);

		

		if (!strcmp(tmpText, file)){

		

			pos -= 31;

		

			unsigned b_offset, b_size;

			

			b_offset = ((block_out[pos]) + (block_out[pos+1]<<8) + (block_out[pos+2]<<16) + (block_out[pos+3]<<24))*4 + 24;

			b_size = ((block_out[pos+8]) + (block_out[pos+9]<<8) + (block_out[pos+10]<<16) + (block_out[pos+11]<<24));

			

			int b_iter = (int)ceil(b_size/2048.f) + 1;

			int trail_size = block_size - (((b_iter -1) * block_size) - b_size);

			

			fseek(fp, b_offset, SEEK_SET);

			

			void** blocks = (void**)malloc(sizeof(void*)*b_iter+1);

			int* blocks_size = (int*)malloc(sizeof(int)*b_iter+1);


			for (int x = 1; x<b_iter; x++){

			

				unsigned cur_pos = ftell(fp);

				bool is_compressed = true;

		

				fread(&offset, 4, 1, fp);


				if (offset >= 0x80000000){

					is_compressed = false;

					offset -= 0x80000000;

				}

				

				fread(&size, 4, 1, fp);

				if (size >= 0x80000000)

					size -= 0x80000000;

				

				size -= offset;

				

				fseek(fp, offset, SEEK_SET);

				

				if (is_compressed){

					if (x < b_iter - 1){

						blocks_size[x-1] = SECTOR_SIZE;

						blocks[x-1] = memalign(64, SECTOR_SIZE);

						compressed = (uint8_t*)malloc(size);

						fread(compressed, 1, size, fp);

						zlib_decompress(compressed, (uint8_t*)blocks[x-1], size);

						free(compressed);

					}

					else{

						blocks_size[x-1] = trail_size;

						blocks[x-1] = memalign(64, SECTOR_SIZE);

						compressed = (uint8_t*)malloc(size);

						fread(compressed, 1, size, fp);

						zlib_decompress(compressed, (uint8_t*)blocks[x-1], size);

						free(compressed);

					}

				}

				else{

					blocks_size[x-1] = size;

					blocks[x-1] = memalign(64, size);

					fread(blocks[x-1], 1, size, fp);

				}

				fseek(fp, cur_pos+4, SEEK_SET);

			}

			int total_size = 0;

			for (int i=0; i<b_iter-1; i++)

				total_size += blocks_size[i];

			buffer = memalign(64, total_size);

			memset(buffer, 0, total_size);

			int counter = 0;

			for (int i=0; i<b_iter-1; i++){

				memcpy((void*)((char*)buffer+counter), blocks[i], blocks_size[i]);

				counter += blocks_size[i];

				free(blocks[i]);

			}

			free(blocks);

			free(blocks_size);

			if (size_out != NULL)

				*size_out = total_size;

			fclose(fp);

			return buffer;

		}

		pos++;

	}

	fclose(fp);

	return NULL;

}

