#define MAX_PATH (0x1000)

size_t GetFileSize(const char *file);
void CopyFile(const char *file, const char* dstfile);
void CopyTree(const char* src, const char* dst);
int ReadFile(const char *file, void *buf, int size);
int WriteFile(const char *file, void *buf, int size);

void CopyFileAndUpdateUi(char* src, char* dst);
void CreateDirAndUpdateUi(char* dir);
size_t CountTree(const char* src);