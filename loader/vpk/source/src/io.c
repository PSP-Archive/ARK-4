#include <vitasdk.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "io.h"
#include "ui.h"

void CopyFileAndUpdateUi(char* src, char* dst) {
	char msg[MAX_PATH+0x100];
	snprintf(msg, sizeof(msg), "Copying %s ...", src); 
	updateUi(msg);
	
	CopyFile(src, dst);
}


void CreateDirAndUpdateUi(char* dir) {
	char msg[MAX_PATH+0x100];
	snprintf(msg, sizeof(msg), "Creating directory %s ...", dir); 
	updateUi(msg);
	
	sceIoMkdir(dir, 0006);
}

int WriteFile(const char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT , 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int ReadFile(const char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file,SCE_O_RDONLY, 0777);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

size_t GetFileSize(const char *file) {
	SceIoStat stat;
	
	int ret = sceIoGetstat(file, &stat);
	if(ret < 0)
		return ret;
	
	return stat.st_size;
}

void CopyFile(const char *file, const char* dstfile) {
	size_t sz = GetFileSize(file);
	
	char* buf = malloc(sz);
	if(buf != NULL) {
		ReadFile(file, buf, sz);
		WriteFile(dstfile, buf, sz);
		free(buf);
	}
}

void CopyTree(const char* src, const char* dst) {
	SceUID dfd = sceIoDopen(src);	
	char* srcEnt = malloc(MAX_PATH);
	memset(srcEnt, 0x00, MAX_PATH);
	
	char* dstEnt = malloc(MAX_PATH);
	memset(dstEnt, 0x00, MAX_PATH);
	
	int dir_read_ret = 0;
	SceIoDirent* dir = malloc(sizeof(SceIoDirent));
	do{
		memset(dir, 0x00, sizeof(SceIoDirent));
		
		dir_read_ret = sceIoDread(dfd, dir);
		
		snprintf(srcEnt, MAX_PATH, "%s/%s", src, dir->d_name);
		snprintf(dstEnt, MAX_PATH, "%s/%s", dst, dir->d_name);
		
		if(SCE_S_ISDIR(dir->d_stat.st_mode)) {
			CreateDirAndUpdateUi(dstEnt);
			CopyTree(srcEnt, dstEnt);
		}
		else{
			CopyFileAndUpdateUi(srcEnt, dstEnt);
		}
		
		
	} while(dir_read_ret > 0);
	
	
	free(dir);
	sceIoDclose(dfd);
	
	free(srcEnt);
	free(dstEnt);
}

size_t CountTree(const char* src) {
	size_t count = 0;
	SceUID dfd = sceIoDopen(src);	
	
	char* srcEnt = malloc(MAX_PATH);
	memset(srcEnt, 0x00, MAX_PATH);
	
	
	int dir_read_ret = 0;
	SceIoDirent* dir = malloc(sizeof(SceIoDirent));
	do{
		memset(dir, 0x00, sizeof(SceIoDirent));
		
		dir_read_ret = sceIoDread(dfd, dir);
		
		snprintf(srcEnt, MAX_PATH, "%s/%s", src, dir->d_name);
		
		if(SCE_S_ISDIR(dir->d_stat.st_mode)) {
			count++;
			count += CountTree(srcEnt);
		}
		else{
			count++;
		}
		
		
	} while(dir_read_ret > 0);
	
	
	free(dir);
	sceIoDclose(dfd);
	
	free(srcEnt);
	
	return count;
}