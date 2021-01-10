#ifndef ISO_H
#define ISO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "entry.h"

#define SECTOR_SIZE 0x800
#define VOLUME_DESCRIPTOR_START_SECTOR 0x10
#define PVD_MAGIC "CD001"
#define ISO_MAGIC 0x30444301

typedef struct s_both_endian_32
{
	unsigned little;
	unsigned big;
} both_endian_32;

typedef struct s_both_endian_16
{
	unsigned short little;
	unsigned short big;
} both_endian_16;

typedef struct s_date_time
{
	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char hour[2];
	unsigned char minute[2];
	unsigned char second[2];
	unsigned char mil[2];
	unsigned char gmt;
} date_time;

typedef struct s_primary_volume_descriptor
{
	unsigned char type;
	char magic[5];
	unsigned char version;
	unsigned char unused;
	unsigned char sys_id[32];
	unsigned char vol_id[32];
	unsigned char unused2[8];
	both_endian_32 vol_size;
	unsigned char unused3[32];

	both_endian_16 vol_count;
	both_endian_16 vol_index;
	both_endian_16 logical_block_size;
	both_endian_32 path_table_size;

	unsigned path_table_location_LSB;
	unsigned path_table_optional_location_LSB;
	unsigned path_table_location_MSB;
	unsigned path_table_optional_location_MSB;

	unsigned char root_entry[34];
	unsigned char vol_set_id[128];
	unsigned char publisher_id[128];
	unsigned char data_preparer_id[128];
	unsigned char app_id[128];
	unsigned char copyright_file[38];
	unsigned char abstract_file[36];
	unsigned char biblio_file[37];

	date_time vol_creation;
	date_time vol_modif;
	date_time vol_expiration;
	date_time vol_effective;

	unsigned char file_structure_version;
	unsigned char unused4;
	unsigned char extra_data[512];
	unsigned char reserved[653];
} __attribute__ ((__packed__)) /*((gcc_struct, __packed__))*/ primary_volume_descriptor;

typedef struct s_path_entry
{
	unsigned char name_length;
	unsigned char extended_length;
	unsigned location;
	unsigned short parent;
} __attribute__ ((__packed__)) /*((gcc_struct, __packed__))*/ path_entry;

typedef struct s_directory_record
{
	unsigned char length;
	unsigned char extended_length;
	both_endian_32 location;
	both_endian_32 data_length;
	unsigned char date[7];
	unsigned char flags;
	unsigned char unit_size;
	unsigned char gap_size;
	both_endian_16 sequence_number;
	unsigned char length_file_id; //files end with ;1
} __attribute__ ((__packed__)) /*((gcc_struct, __packed__))*/ directory_record;

enum e_directory_record_flags
{
	FLAG_HIDDEN = 1,
	FLAG_DIRECTORY = 2,
	FLAG_ASSOCIATED = 4,
	FLAG_EXTENDED = 8,
	FLAG_OWNER = 16,
	FLAG_RESERVED1 = 32,
	FLAG_RESERVED2 = 64,
	FLAG_NOT_FINAL = 128
};

class Iso : public Entry
{
	public:

		Iso();
		Iso(string path);
		~Iso();
	
		void loadIcon();
		void getTempData1();
		void getTempData2();
		
		static bool isPatched(string path);
		static bool isISO(const char* filepath);
		
		/* Much faster function for extracting files in PSP_GAME/ */
		static void* fastExtract(const char* path, char* file, unsigned* size=NULL);
		
		void execute();
		
		char* getType();
		char* getSubtype();

		void clear();
		int open(const char * path);

		int load();

		int find(const char * name, unsigned * store_offset = NULL, unsigned * store_size = NULL);
		int search_in_directory_entry(const char * path, unsigned dir_entry_sector, unsigned * store_offset, unsigned * store_size);
		char * get_name_end(const char * str);
		void readFile(void* dst, unsigned offset, unsigned size);
		
		int extractFile(const char * file, const char * destination);

		int read(void * destination, unsigned size, unsigned count, void * p);
		int seek(void * p, unsigned bytes, int flag);

	protected:

		FILE * stream;
		primary_volume_descriptor pvd;
		path_entry main_path_entry;
};

#endif
