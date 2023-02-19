#ifndef __VLF_H__
#define __VLF_H__

#define VLF_DEFAULT	-1

#define VLF_BATTERY_ICON_HIGH		0
#define VLF_BATTERY_ICON_MEDIUM		1
#define VLF_BATTERY_ICON_LOW		2
#define VLF_BATTERY_ICON_LOWEST		3

#define VLF_ERROR_INVALID_INPUT			(-1)
#define VLF_ERROR_INVALID_INPUT_DATA	(-2)
#define VLF_ERROR_UNSUPPORTED_FORMAT	(-3)
#define VLF_ERROR_OBJECT_OVERFLOW		(-4)
#define VLF_ERROR_OBJECT_NOT_FOUND		(-5)
#define VLF_ERROR_NO_MEMORY				(-6)
#define VLF_ERROR_SYSTEM				(-7)
#define VLF_ERROR_DUPLICATED			(-8)

/** Fade modes */
#define VLF_FADE_MODE_IN		1
#define VLF_FADE_MODE_OUT		2
#define VLF_FADE_MODE_REPEAT	4

/** Fade effect */
#define VLF_FADE_EFFECT_STANDARD	0
#define VLF_FADE_EFFECT_FAST		1
#define VLF_FADE_EFFECT_VERY_FAST	2
#define VLF_FADE_EFFECT_SLOW		3
#define VLF_FADE_EFFECT_SUPERFAST	4

/* Alignment */
#define VLF_ALIGNMENT_LEFT			0
#define VLF_ALIGNMENT_CENTER	0x200
#define VLF_ALIGNMENT_RIGHT		0x400

enum VlfObjects
{
	VLF_TEXT = 0,
	VLF_PIC = 1,
	VLF_SHADOWED_PIC = 2,
	VLF_PROGRESS_BAR = 3
};

enum VlfButtonIcon
{
	VLF_ENTER = 0,
	VLF_CANCEL = 1,
	VLF_CROSS = 2,
	VLF_CIRCLE = 3,
	VLF_TRIANGLE = 4,
	VLF_SQUARE = 5
};

enum RCOType
{
	RCO_GRAPHIC,
	RCO_OBJECT,
	RCO_SOUND,
	RCO_LABEL,
	RCO_FILEPARAM,
	RCO_ANIMPARAM
};

enum VLF_MDType
{
	VLF_MD_TYPE_ERROR,
	VLF_MD_TYPE_NORMAL,
};

enum VLF_MD_Buttons
{
	VLF_MD_BUTTONS_NONE = 0,
	VLF_MD_BUTTONS_YESNO = 0x10,
};

enum VLF_MD_InitalCursor
{
	VLF_MD_INITIAL_CURSOR_YES = 0,
	VLF_MD_INITIAL_CURSOR_NO = 0x100,
};

enum VLF_MD_ButtonRes
{
	VLF_MD_NONE,
	VLF_MD_YES,
	VLF_MD_NO,
	VLF_MD_BACK
};

enum VLF_DialogItem
{
	VLF_DI_ENTER,
	VLF_DI_CANCEL,
	VLF_DI_BACK,
	VLF_DI_YES,
	VLF_DI_NO,
	VLF_DI_EDIT,
};

enum
{
	VLF_SPIN_STATE_NOT_FOCUS, // Spin control has not focus
	VLF_SPIN_STATE_FOCUS, // Spin control text has focus but arrow is not shown
	VLF_SPIN_STATE_ACTIVE, // Spin control has focus, and it is active 
};

#define VLF_EV_RET_NOTHING				0
#define VLF_EV_RET_REMOVE_EVENT			1 
#define VLF_EV_RET_REMOVE_OBJECTS		2
#define VLF_EV_RET_REMOVE_HANDLERS		4
#define VLF_EV_RET_REFRESH_ON_DELAY		8
#define VLF_EV_RET_DELAY				0x80000000 /* Delay VLF_EV_RET_DELAY | (X << 16), 0 <= X <= 32767 milisecs */


enum PspCtrlExtension
{
	PSP_CTRL_ENTER = 0x40000000,
	PSP_CTRL_CANCEL = 0x80000000
};

/**
 * Inits VLF library
*/
void vlfGuiInit(int heap_size, int (* app_main)(int argc, char *argv[]));

int  vlfGuiSystemSetup(int battery, int clock, int notuserwp);
int  vlfGuiGetLanguage();
void vlfGuiSetLanguage(int lang);
int  vlfGuiGetButtonConfig();
void  vlfGuiSetButtonConfig(int config);
void vlfGuiSetResourceDir(char *dir);

/**
 * Performs the draw of the current frame
*/
void vlfGuiDrawFrame();

/**
 * Loads resources from a rco file
 *
 * @param rco - It can be one of following things:
 * - path relative to the flash0:/vsh/resource directory without extension (e.g. "system_plugin_bg")
 * - path relative to the flash0:/vsh/resource directory with extension (e.g. "system_plugin_bg.rco")
 * - path to a file (e.g. "flash0:/vsh/resource/system_plugin_bg.rco", "ms0:/myresfile.rco")
 *
 * RCO param is evaluated in the order given above, so if a rco file exists in current directory with name 
 * "system_plugin_bg.rco", it would load the one of flash0 and not the one of current directory. (in such a case, use "./system_plugin_bg.rco")
 * 
 * @param n - The number of resources to loads
 * @param names (IN) - An array with the names of resources
 * @param types (IN) - An array with the types of the resources (one of RCOType)
 * @param datas (OUT) - A pointer to a variable that will receive an array of pointers to the content of each resource,
 * or NULL if a specific resource has not been found.
 * Pointers returned are allocated with malloc, and should be deallocated by the application.
 *
 * @param sizes (OUT) - It will receive the sizes of the resources
 * @param pntable (OUT) - A pointer that will receive the string table. Pass NULL if no required.
 * Returned pointer is allocated with malloc and should be deallocated by the application.
 *
 * @returns - the number of resources loaded, or < 0 if there is an error.
 *
 * @Sample: Load battery icon pic and shadow
 *
 * char *names[2];
 * void *datas[2];
 * int types[2], sizes[2];
 *
 * names[0] = "tex_battery";
 * names[1] = "tex_battery_shadow";
 * types[0] = types[1] = RCO_GRAPHIC;
 *
 * int res = vlfGuiLoadResources("system_plugin_fg", 2, names, types, datas, sizes, NULL);
 * if (res != 2) // error or not all resources loaded
 * {
 *    if (res > 0)
 *    {
 *       if (datas[0])
 *          free(datas[0]);
 *       if (datas[1])
 *          free(datas[1]);
 *    }
 * }
 * else
 * {
 *    void *bat;    
 *    vlfGuiAddShadowedPicture(&bat, datas[0], sizes[0], datas[1], sizes[1], 441, 4, 1, 1, 1); 
 *    free(datas[0]);
 *    free(datas[1]);
 * }
 *
*/
int  vlfGuiLoadResources(char *rco, int n, char **names, int *types, void **datas, int *sizes, char **pntable);

int  vlfGuiCacheResource(char *rco);
int  vlfGuiUncacheResource(char *rco);


int  vlfGuiGetResourceSubParam(void *entry, int insize, char *ntable, char *name, void **data, int *size);


int  vlfGuiLoadLabel(u16 *str, char *rco, char *name);

/**
 * Sets the background from 8888 texture data
 *
 * @param texture - The texture data in 8888 format
 * @param width - The width of texture. Must be a power of 2.
 * @param height - The height of texture. Must be multiple of 8.
 * @param swizzled - Indicates if the texture is already in the psp GE fast texture format
 * @param scale_x - The x scale to apply
 * @param scale_y - The y scale to apply
 *
 * @returns 0 on success, or < 0 on error (params invalid)
*/
int  vlfGuiSetBackground(u32 *texture, int width, int height, int swizzled, float scale_x, float scale_y);

/**
 * Sets the background from a file buffer.
 * Supported formats are currently: BMP, TIM and GIM, with a depth of 24 or 32 bits.
 *
 * @param data - The buffer with the file data
 * @param size - The size of the data
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetBackgroundFileBuffer(void *data, int size);

/**
 * Sets the background from a file
 * Supported formats are currently: BMP, TIM and GIM, with a depth of 24 or 32 bits.
 *
 * @param file - Path to the file.
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetBackgroundFile(char *file);

/**
 * Sets one of system backgrounds based on the index.
 *
 * @param index - The index of the background, valid values are 1-27 
 * (note that 13-27 is only available on slim and will return an error on old psp)
 *
 * @returns 0 on success, < 0 on error
*/
int  vlfGuiSetBackgroundIndex(int index);

/**
 * Sets one of system backgrounds based on the current date
 *
 * @returns 0 on success, < 0 on error
*/
int  vlfGuiSetBackgroundDate();

/** 
 * Sets a background of a single color
 *
 * @param color - The color in XXBBGGRR format (XX is ignored).
 *
 * @returns - this functions always succeeds returning 0
*/
int  vlfGuiSetBackgroundPlane(u32 color);

/**
 * Sets the background according to the system configuration
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetBackgroundSystem(int notuserwp);

/**
 * Sets the system color, used in titlebars or menus
 *
 * @param index - the index of the color, 1-27
*/
void vlfGuiSetSystemColor(int index); 

/**
 * Sets the background model from a buffer.
 *
 * @param data - The buffer with the model in GMO format
 * @param size - The size of the model
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetModel(void *data, int size);

/**
 * Sets the background model from a file.
 *
 * @param file - The file with the model in GMO format
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetModelFile(char *file);

/**
 * Sets the background model from a resource.
 *
 * @param rco - The path to the RCO file
 * @param name - The name of the resource
 *
 * @returns - 0 on success, < 0 on error.
*/
int  vlfGuiSetModelResource(char *rco, char *name);

/**
 * Sets the background model of the system, and applies the proper world matrix to it.
 *
 * @returns 0 on success, < 0 on error.
*/
int  vlfGuiSetModelSystem();

/**
 * Sets the world matrix for the model. (by default, the world matrix is the identity 
 * after a model has been loaded, except when calling vlfGuiSetModelSystem).
 *
 * @param matrix - The matrix to set.
 *
 * @Sample: Load waves (this sample assumes the default scale of 8.5)
 *
 * int res = vlfGuiSetModelResource("system_plugin_bg", "mdl_bg");
 * if (res < 0) process_error;
 *
 * ScePspFMatrix4 matrix;
 * ScePspFVector3 scale;
 *
 * scale.x = scale.y = scale.z = 8.5f;
 * gumLoadIdentity(&matrix);
 * gumScale(&matrix, &scale);
 * vlfGuiSetModelWorldMatrix(&matrix);
*/
void vlfGuiSetModelWorldMatrix(ScePspFMatrix4 *matrix);

/**
 * Gets the world matrix of the model
 *
 * @returns a pointer to the model world matrix
*/
ScePspFMatrix4 *vlfGuiGetModelWorldMatrix();

void vlfGuiSetTitleBar(int text, int pic, int visible, int hideobj);
void vlfGuiSetTitleBarEx(int text, int pic, int visible, int hideobj, u32 color);
void vlfGuiSetTitleBarVisibility(int visible);

int  vlfGuiAddText(int x, int y, char *string);
int  vlfGuiAddTextW(int x, int y, wchar_t *string);
int  vlfGuiAddTextF(int x, int y, char *fmt, ...);
int  vlfGuiAddTextResource(char *rco, char *name, int x, int y);
int  vlfGuiRemoveText(int text);
int  vlfGuiSetText(int text, char *string);
int  vlfGuiSetTextW(int text, wchar_t *string);
int  vlfGuiSetTextF(int text, char *string, ...);
int  vlfGuiSetTextFocus(int text);
int  vlfGuiRemoveTextFocus(int text, int keepres);
int  vlfGuiSetTextVisibility(int text, int visible);
int  vlfGuiSetTextBlinking(int text, u32 nshow, u32 nhide);
int  vlfGuiSetTextFade(int text, int mode, int effect, int direction_out);
int  vlfGuiSetTextAlignment(int text, int alignment);
int  vlfGuiSetTextXY(int text, int x, int y);
int  vlfGuiSetTextSize(int text, float size);
int  vlfGuiChangeCharacterByButton(u16 ch, int button);

int  vlfGuiAddPicture(void *data, int size, int x, int y);
int  vlfGuiAddPictureFile(char *file, int x, int y);
int  vlfGuiAddPictureResource(char *rco, char *name, int x, int y);
int  vlfGuiRemovePicture(int pic);
int  vlfGuiSetPictureXY(int pic, int x, int y);
int  vlfGuiGetPictureSize(int pic, int *width, int *height);
int  vlfGuiSetPictureDisplayArea(int pic, u32 x, u32 y, u32 width, u32 height);
int  vlfGuiSetPictureAlphaBlend(int pic, int op, int src, int dst, u32 srcfix, u32 dstfix);
int  vlfGuiClonePicture(int pic, int real, int x, int y);
int  vlfGuiSetPictureVisibility(int pic, int visible);
int  vlfGuiSetPictureBlinking(int pic, u32 nshow, u32 nhide);
int  vlfGuiAnimatePicture(int pic, int w, int h, int frames, int vertical);
int  vlfGuiSetPictureFade(int pic, int mode, int effect, int direction_out);

int  vlfGuiAddShadowedPicture(void **sp, void *pic, int pic_size, void *shpic, int shpic_size, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);
int  vlfGuiAddShadowedPictureFile(void **sp, char *pic, char *shpic, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);
int  vlfGuiAddShadowedPictureResource(void **sp, char *rco, char *pic, char *shpic, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);
int  vlfGuiRemoveShadowedPicture(void *sp);
int  vlfGuiSetShadowedPictureVisibility(void *sp, int visible);
int  vlfGuiSetShadowedPictureBlinking(void *sp, u32 nshow, u32 nhide);
int  vlfGuiAnimateShadowedPicture(void *sp, int w, int h, int ws, int hs, int frames, int vertical);
int  vlfGuiSetShadowedPictureFade(void *sp, int mode, int effect, int direction_out);

int  vlfGuiAddBatteryIcon(void **baticon, u32 status, int blink);
int  vlfGuiAddBatteryIconEx(void **baticon, int x, int y, u32 status, int blink);
int  vlfGuiAddBatteryIconSystem(void **baticon, int timer_ms);
int  vlfGuiSetBatteryIconStatus(void *baticon, int status, int blink);
int  vlfGuiRemoveBatteryIcon(void *baticon);

int  vlfGuiAddClock();

int  vlfGuiAddWaitIcon(void **waiticon);
int  vlfGuiAddWaitIconEx(void **waiticon, int x, int y);

int  vlfGuiAddCross(void **cross, int x, int y);
int  vlfGuiAddCircle(void **circle, int x, int y);
int  vlfGuiAddTriangle(void **triangle, int x, int y);
int  vlfGuiAddSquare(void **square, int x, int y);
int  vlfGuiAddEnter(void **enter, int x, int y);
int  vlfGuiAddCancel(void **cancel, int x, int y);
int  vlfGuiAddSpinUp(void **spinup, int x, int y);
int  vlfGuiAddSpinDown(void **spindown, int x, int y);
int  vlfGuiAddArrowLeft(void **arrowleft, int x, int y);
int  vlfGuiAddArrowRight(void **arrowright, int x, int y);

int  vlfGuiAddGameIcon(void **game, int x, int y);

int  vlfGuiAddProgressBar(int y);
int  vlfGuiAddProgressBarEx(int x, int y);
int  vlfGuiRemoveProgressBar(int pb);
int  vlfGuiProgressBarSetProgress(int pb, u32 perc);

int  vlfGuiSetRectangleVisibility(int x, int y, int w, int h, int visible);
void *vlfGuiSaveRectangleVisibilityContext(int x, int y, int w, int h);
void vlfGuiRestoreVisibilityContext(void *ctx);
void vlfGuiFreeVisibilityContext(void *ctx);

int  vlfGuiSetRectangleBlinking(int x, int y, int w, int h, u32 nshow, u32 nhide);
int  vlfGuiSetSynchronizedBlinking(void *dst, int dst_type, void *src, int src_type);

int  vlfGuiMessageDialog(char *msg, u32 flags);
int  vlfGuiErrorDialog(int error);
int  vlfGuiNetConfDialog();

int  vlfGuiBottomDialog(int button1, int button2, int automatic, int enter_is_left, int distance, int (* handler)(int enter));
int  vlfGuiCustomBottomDialog(char *button1, char *button2, int automatic, int enter_is_left, int distance, int (* handler)(int enter));
void vlfGuiCancelBottomDialog();

int  vlfGuiCentralMenu(int noptions, char **items, int defaultsel, int (* handler)(int sel), int dispx, int dispy);
void vlfGuiCancelCentralMenu();
int  vlfGuiCentralMenuSelection();

int  vlfGuiAddIntegerSpinControl(int x, int y, int min, int max, int cur, int step, int loop, int speed, int initstate, char *prefix, char *suffix);
int  vlfGuiRemoveSpinControl(int spin);
int  vlfGuiSetSpinState(int spin, int state);
int  vlfGuiSetIntegerSpinMinMax(int spin, int min, int max);
int  vlfGuiGetIntegerSpinValue(int spin, int *value);
int  vlfGuiSetIntegerSpinValue(int spin, int value);

int  vlfGuiPreviousPageControl(int (* handler)(int page));
int  vlfGuiNextPageControl(int (* handler)(int page));
void vlfGuiCancelPreviousPageControl();
void vlfGuiCancelNextPageControl();
void vlfGuiSetPageControlEnable(int enable);

int  vlfGuiAddEventHandler(int buttons, int wait, int (* func)(void *), void *param);
int  vlfGuiAddNegativeEventHandler(int buttons, int wait, int (* func)(void *), void *param);
int  vlfGuiRemoveEventHandler(int (* func)(void *));
int  vlfGuiRemoveEventHandlerEx(int (* func)(void *), void *param);
int  vlfGuiIsEventRegistered(int (* func)(void *));
int  vlfGuiSetEventDelay(int (* func)(void *), u32 delay);
int  vlfGuiSetEventDelayEx(int (* func)(void *), void * param, u32 delay);
int  vlfGuiDelayAllEvents(u32 delay);


#endif

