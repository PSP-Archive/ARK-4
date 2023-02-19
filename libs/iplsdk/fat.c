#include <pspsdk.h>
#include <string.h>

#include "ms.h"
#include "fat.h"

#ifdef FAT_DEBUG
#include "printf.h"
#endif

u8	sector_buf[0x200];
u32	boot_sector;
u32	rootdir_sectors;
u32	first_fat_sector;
u32	first_data_sector;
u32	rootdir_cluster;
u32	rootdir_sector;
u8	sec_per_cluster;
int fat_type;
char filename[256], filename2[256];

MsFatFile thefile; // Only opened file

int MsFatReadLogicalSector(int sector, void *buf)
{
	int res = pspMsReadSector(boot_sector+sector, buf);

#ifdef FAT_DEBUG
	if (res < 0)
	{
		printf("Hardware error.\n");
	}
#endif

	return res;
}

u32 MsFatGetNextCluster(u32 cluster)
{
	u32	nextcluster = 0, offset, local_offset;
	
	switch (fat_type)
	{
		case FAT_TYPE_12:
			offset = cluster + (cluster / 2);
			local_offset = offset % 0x200;
		
			MsFatReadLogicalSector(first_fat_sector + (offset / 0x200), sector_buf);

			if (cluster & 1)
			{
				nextcluster = (sector_buf[local_offset] & 0x0F) << 8;

				if (local_offset == 0x1FF)
				{
					// Special case, read another sector
					MsFatReadLogicalSector(first_fat_sector + (offset / 0x200) + 1, sector_buf);
					nextcluster |= sector_buf[0];
				}
				else
				{
					nextcluster |= sector_buf[local_offset+1];
				}
			}

			else
			{
				nextcluster = (sector_buf[local_offset]<<4) | (sector_buf[local_offset+1]>>4);
			}			

		break;
	
		case FAT_TYPE_16:

			MsFatReadLogicalSector(first_fat_sector + ((cluster * 2) / 0x200), sector_buf);
			nextcluster = *(u16 *)&sector_buf[(cluster * 2) % 0x200];

		break;
	
		case FAT_TYPE_32:

			MsFatReadLogicalSector(first_fat_sector + ((cluster * 4) / 0x200), sector_buf);			
			nextcluster = *(u32 *)&sector_buf[(cluster * 4) % 0x200];
		
		break;		
	}

	return nextcluster;
}

int MsFatMount()
{
	u32 fat_size, total_sectors, nclusters;
	u16	rsvd_sec_cnt;

#ifdef FAT_DEBUG
	printf("Payloadex Fat driver v 0.21.\n");
	printf("MsFatMount\n");
#endif
	
	pspMsInit();

	if (pspMsReadSector(0, sector_buf) < 0)
	{
		return -1;
	}

	if (sector_buf[0x1FE] != 0x55 || sector_buf[0x1FF] != 0xAA)
	{
#ifdef FAT_DEBUG
		printf("No fat partition.\n");
#endif
		return -1;
	}

	boot_sector = sector_buf[0x1c6] | (sector_buf[0x1c7]<<8) | (sector_buf[0x1c8]<<16) | (sector_buf[0x1c9]<<24);

	if (MsFatReadLogicalSector(0, sector_buf) < 0)
	{
		return -1;
	}

	if (sector_buf[0x1FE] != 0x55 || sector_buf[0x1FF] != 0xAA)
	{
#ifdef FAT_DEBUG
		printf("Invalid partition.\n");
#endif
		return -1;
	}

	fat_size = *(u16 *)&sector_buf[0x16];

	if (fat_size != 0)
	{
		u16 root_entcount = sector_buf[0x11] | (sector_buf[0x12]<<8);
		
		rootdir_sectors = ((root_entcount*32) + 0x1FF) / 0x200;
	}
	else
	{
		fat_size = *(u32 *)&sector_buf[0x24];
		rootdir_sectors = 0;
	}

	rsvd_sec_cnt = *(u16 *)&sector_buf[0x0E];
	first_fat_sector = rsvd_sec_cnt;
	first_data_sector = rsvd_sec_cnt + (sector_buf[0x10] * fat_size) + rootdir_sectors;
	sec_per_cluster = sector_buf[0xD];

	total_sectors = sector_buf[0x13] | (sector_buf[0x14]<<8);
	if (total_sectors == 0)
	{
		total_sectors = *(u32 *)&sector_buf[0x20];
	}

	nclusters = (total_sectors - first_fat_sector) / sec_per_cluster;
	if (nclusters < 4085)
	{
		fat_type = FAT_TYPE_12;
	}
	else if (nclusters < 65525)
	{
		fat_type = FAT_TYPE_16;
	}
	else
	{
		fat_type = FAT_TYPE_32;
	}	

	if (fat_type == FAT_TYPE_32)
	{
		rootdir_cluster = *(u32 *)&sector_buf[0x2C];
	}
	else
	{
		rootdir_sector = first_data_sector-rootdir_sectors;
	}

	return 0;
}

int MsFatIsValidCluster(u32 cluster)
{
	switch (fat_type)
	{
		case FAT_TYPE_12:			
		return (cluster >= 0x002 && cluster <= 0xFF6);

		case FAT_TYPE_16:
		return (cluster >= 0x0002 && cluster <= 0xFFF6);

		case FAT_TYPE_32:
			cluster &= 0x0FFFFFFF;
		return (cluster >= 0x00000002 && cluster <= 0x0FFFFFF6);
	}

	return 0;
}

static void MsFatFillFileStruct(u8 *entry, MsFatFile *file)
{
	file->attributes = entry[0xB];
	file->remaining_bytes = *(u32 *)&entry[0x1C];
	file->cur_cluster = *(u16 *)&entry[0x1A];
	
	if (fat_type == FAT_TYPE_32)
	{
		file->cur_cluster |= ((*(u16 *)&entry[0x14]) << 16);
	}
	
	file->cur_sector = 0;	
}

int MsFatFindFile(u32 dir_cluster, char *filename, int rootcase, MsFatFile *file)
{
	int available_entries = 0, lfn = 0;	
	char *p = &filename2[254];

	filename2[255] = 0;

	while (MsFatIsValidCluster(dir_cluster) || rootcase)
	{
		u32 next_sector;  
		int rem_sectors;  
		
		if (rootcase)
		{
			next_sector = rootdir_sector;
			rem_sectors = rootdir_sectors;
		}
		else
		{
			next_sector = ((dir_cluster - 2) * sec_per_cluster) + first_data_sector;
			rem_sectors = sec_per_cluster;
		}

		available_entries = 0;

		while (rem_sectors >= 0)
		{		
			while (available_entries > 0)
			{
				u8 *entry;  
				
				entry = &sector_buf[(0x10-available_entries) * 0x20];
								
				if (entry[0xB] == (FAT_ATTR_LONG_NAME))
				{
					lfn = 1;

					*p-- = entry[0x1E];
					*p-- = entry[0x1C];
					*p-- = entry[0x18];
					*p-- = entry[0x16];
					*p-- = entry[0x14];
					*p-- = entry[0x12];
					*p-- = entry[0x10];
					*p-- = entry[0x0E];
					*p-- = entry[0x09];
					*p-- = entry[0x07];
					*p-- = entry[0x05];
					*p-- = entry[0x03];
					*p-- = entry[0x01];
				}
				else
				{
					if (entry[0] == 0)
					{
						goto FILE_NOT_FOUND;
					}

					if (entry[0] != 0xE5)
					{
						if (entry[0] == 0x05)
							entry[0] = 0xE5;

						if (lfn)
						{
							if (strcasecmp(filename, ++p) == 0)
							{
								// Found
								MsFatFillFileStruct(entry, file);
								return 0;
							}
						}

						// Try short name now
						int i, j;
						filename2[0] = entry[0];

						if (filename2[0] != 0x20)
						{
							for (i = 1; i < 8; i++)
							{
								if (entry[i] == 0x20)
									break;

								filename2[i] = entry[i];
							}

							if (entry[8] == 0x20)
							{
								filename2[i] = 0;
							}
							else
							{
								filename2[i] = '.';

								for (j = 8; j < 11; j++)
								{
									if (entry[j] == 0x20)
										break;

									filename2[i+1+j-8] = entry[j];
								}

								filename2[i+1+j-8] = 0; 
							}							

							if (strcasecmp(filename, filename2) == 0)
							{
								// Found
								MsFatFillFileStruct(entry, file);
								return 0;
							}
						}							
					}

					lfn = 0;
					p = &filename2[254];
				}
				
				available_entries--;
			}

			if (MsFatReadLogicalSector(next_sector, sector_buf) < 0)
			{
				return -1;
			}

			rem_sectors--;
			next_sector++;
			available_entries = 0x200/0x20;			
		}

		if (rootcase)
			break;
		
		dir_cluster = MsFatGetNextCluster(dir_cluster);					
	}

FILE_NOT_FOUND:

#ifdef FAT_DEBUG
	printf("File or dir \"%s\" not found.\n", filename);
#endif
	return -1;
}

int MsFatOpen(char *path)
{
	u32 dir_cluster;
	int rootcase = 0;
	char *p = path;

#ifdef FAT_DEBUG
	printf("Opening file %s\n", path);
#endif

	if (fat_type == FAT_TYPE_32)
	{
		dir_cluster = rootdir_cluster;
	}
	else
	{
		dir_cluster = rootdir_sector; // Sector instead of cluster
		rootcase = 1;
	}
	
	while (1)
	{
		char *q = filename;
		
		if (*p == '/' || *p == '\\')
			p++;

		while (*p != '/' && *p != '\\' && *p != 0)
		{
			*q = *p;
			p++;
			q++;
		}

		*q = 0;

		if ((*p == '/' || *p == '\\') && *(p+1) == 0)
		{
#ifdef FAT_DEBUG
			printf("Specified path \"%s\" is not valid for a file.\n", path);
#endif
			return -1;
		}

		if (MsFatFindFile(dir_cluster, filename, rootcase, &thefile) < 0)
			return -1;

		if ((*p == '/' || *p == '\\') &&  (thefile.attributes & FAT_ATTR_ARCHIVE))
		{
#ifdef FAT_DEBUG
			printf("\"%s\" is a file, not a dir.\n", filename);
#endif
			return -1;
		}
		else if (*p == 0)
		{
			if (thefile.attributes & FAT_ATTR_DIRECTORY)
			{
#ifdef FAT_DEBUG
				printf("\"%s\" is a dir, not a file.\n", filename);
#endif
				return -1;
			}

			break;
		}

		dir_cluster = thefile.cur_cluster;
		rootcase = 0;
	}	

	return 0;
}

int MsFatRead(void *buf, u32 size)
{
	u32 remaining = size, sector;
	int read = 0;
	u8 *p = buf;

	if (thefile.remaining_bytes == 0)
		return 0;

	sector = ((thefile.cur_cluster - 2) * sec_per_cluster) + first_data_sector + thefile.cur_sector;

	while (remaining > 0)
	{
		if (thefile.remaining_bytes < 0x200)
		{
			if (MsFatReadLogicalSector(sector, sector_buf) < 0)
				return read;

			memcpy(p, sector_buf, thefile.remaining_bytes);
			read += thefile.remaining_bytes;

			thefile.remaining_bytes = 0;
			break;
		}

		if (MsFatReadLogicalSector(sector, p) < 0)
			break;

		read += 0x200;
		p += 0x200;
		remaining -= 0x200;
		thefile.remaining_bytes -= 0x200;
		thefile.cur_sector++;

		if (thefile.cur_sector == sec_per_cluster)
		{
			thefile.cur_sector = 0;
			thefile.cur_cluster = MsFatGetNextCluster(thefile.cur_cluster);
			
			if (!MsFatIsValidCluster(thefile.cur_cluster))
			{
				if (thefile.remaining_bytes != 0)
				{
#ifdef FAT_DEBUG
					printf("WTF!\n");
#endif
					thefile.remaining_bytes = 0;
				}

				break;
			}
			
			sector = ((thefile.cur_cluster - 2) * sec_per_cluster) + first_data_sector;
		}
		else
		{
			sector++;
		}
	}

	return read;
}

int MsFatClose()
{
	// Implementation not needed
	return 0;
}

