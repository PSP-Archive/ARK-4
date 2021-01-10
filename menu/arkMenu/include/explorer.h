#ifndef Explorer_H
#define Explorer_H

#include "common.h"
#include "controller.h"
#include "gfx.h"
#include <vector>


#define COPY_BUFFER_SIZE 0x8000

#define LIST_START_X 107
#define LIST_START_Y 62
#define LIST_SEPARATION 14

#define MAX_ON_SCREEN_Explorer 12
#define ENTRY_NAME_LIMIT 25
#define PATH_NAME_LIMIT 40

#define TOOLS_X 360
#define TOOLS_Y 62

#define INSTALLERS_PATH "ms0:/PSP/SAVEDATA/"
#define APP_PATH "ms0:/PSP/GAME/"

enum E_COMMANDS
{
	NONE = -1,
	COPY = 0,
	CUT = 1,
	PASTE = 2,
	DELETE = 3,
	RENAME = 4,	
	MKDIR = 5,
	CANCEL = 6,
	RETURN = 7
};

typedef struct s_tool_option
{
	const char * name;
	int enabled;
	int command;
} t_tool_option;

static t_tool_option tool_options[] = 
{
	{NULL, 0, COPY},
	{NULL, 0, CUT},
	{NULL, 0, PASTE},
	{NULL, 0, DELETE},
	{NULL, 0, RENAME},
	{NULL, 0, MKDIR},
	{NULL, 0, CANCEL},
	{NULL, 0, RETURN}
};

static const int tool_count = sizeof(tool_options) / sizeof(t_tool_option);

typedef struct s_file
{
	char * path;
	char * name;
	unsigned size;
	int type;
} t_file;

using namespace std;

class filelist
{
	public:

		enum E_FILE_TYPE
		{
			FILES = 0,
			FOLDER = 1
		};

		filelist();
		~filelist();

		int load(const char * path, const char * extension_filter = NULL);
		void clear();

		void add_file(const char * path, const char * name, int type, int size);
		int delete_file(const char * path, int COMPARE_NAME = 0);
		int find_file(const char * path, int COMPARE_NAME = 0);

		static bool sort_function(const t_file * file1, const t_file * file2);

		t_file * get(unsigned index);
		int get_count();

	private:

		void delete_entry(t_file * file);
		vector<t_file*> files;
};

class Explorer
{
	public:
	
		static unsigned copy_progress;
		static unsigned copy_total;
		static char * copy_name;
		static bool copy_active;
	
		static int create_dummy(const char * path);
	
		static int file_exists(const char * path);
		static int folder_exists(const char * path);
	
		static int delete_file(const char * path);
		static int delete_folder(const char * path);
	
		static int copy_bytes(SceUID source, SceUID destination, unsigned bytes);
		static int copy_file(const char * source, const char * destination, unsigned source_offset = 0, unsigned source_size = 0);
		static int copy_folder_recursive(const char * source, const char * destination);
		static int copy_folder(const char * source, const char * destination);
	
		static int move_file(const char * source, const char * destination);
		static int move_folder(const char * source, const char * destination);
	
		static int create_folder(const char * path);
		static int rename(const char * old_name, const char * new_name);
		static unsigned file_size(const char * path);
	
		Explorer();
		~Explorer();
		void clear();
		void browse(const char * path);
		void refresh();
		int show();
	
		void draw_all();
	
	private:
	
		void check();
		void draw();
	
		void dofile(const char * path);
		int show_image(const char * path);
	
		void update_language();
		void tools_draw();
		void tools_check();
		void tools_open();
		void tools_docommand(int command);
		struct
		{
			bool enabled;
			int option;
			int command;
		} tools;
	
		bool active;
		char * current_path;
		int list_option, offset;
		filelist files;
		filelist clipboard;
		Image* Explorer_tools;
		Image* Explorer_check;
		Image* Explorer_folder;
		Image* Explorer_file;
};

#endif
