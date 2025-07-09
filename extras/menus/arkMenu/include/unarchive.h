extern "C" {
    int unarchiveFile(const char* filepath, const char* parent, void (*logger)(const char*, int, int));
}