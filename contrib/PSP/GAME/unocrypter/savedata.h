#ifndef _SAVEDATA_H_
#define _SAVEDATA_H_

typedef enum
{
	SCE_UTILITY_SAVEDATA_TYPE_AUTOLOAD = 0,
	SCE_UTILITY_SAVEDATA_TYPE_AUTOSAVE = 1,
	SCE_UTILITY_SAVEDATA_TYPE_LOAD = 2,
	SCE_UTILITY_SAVEDATA_TYPE_SAVE = 3,
	SCE_UTILITY_SAVEDATA_TYPE_LISTLOAD = 4,
	SCE_UTILITY_SAVEDATA_TYPE_LISTSAVE = 5,
	SCE_UTILITY_SAVEDATA_TYPE_LISTDELETE = 6,
	SCE_UTILITY_SAVEDATA_TYPE_DELETE = 7,
	SCE_UTILITY_SAVEDATA_TYPE_SIZES = 8,
	SCE_UTILITY_SAVEDATA_TYPE_AUTODELETE = 9,
	SCE_UTILITY_SAVEDATA_TYPE_SINGLEDELETE = 10,
	SCE_UTILITY_SAVEDATA_TYPE_MC_USERIDLIST = 11,
	SCE_UTILITY_SAVEDATA_TYPE_MC_FILELIST = 12,
	SCE_UTILITY_SAVEDATA_TYPE_MC_CREATEDATA_SECUREFILE = 13,
	SCE_UTILITY_SAVEDATA_TYPE_MC_CREATEDATA_NORMALFILE = 14,
	SCE_UTILITY_SAVEDATA_TYPE_MC_READ_SECUREFILE = 15,
	SCE_UTILITY_SAVEDATA_TYPE_MC_READ_NORMALFILE = 16,
	SCE_UTILITY_SAVEDATA_TYPE_MC_WRITE_SECUREFILE = 17,
	SCE_UTILITY_SAVEDATA_TYPE_MC_WRITE_NORMALFILE = 18,
	SCE_UTILITY_SAVEDATA_TYPE_MC_REMOVE_SECUREFILE = 19,
	SCE_UTILITY_SAVEDATA_TYPE_MC_REMOVE_NORMALFILE = 20,
	SCE_UTILITY_SAVEDATA_TYPE_MC_DELETEDATA = 21,
	SCE_UTILITY_SAVEDATA_TYPE_MC_CHECKSIZE = 22
} SceUtilitySavedataType;

#define SCE_UTILITY_SAVEDATA_ERROR_TYPE 0x80110300
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_NO_MS 0x80110301
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_EJECT_MS 0x80110302 
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_ACCESS_ERROR 0x80110305
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_DATA_BROKEN 0x80110306
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_NO_DATA 0x80110307
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_PARAM 0x80110308
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_NO_FILE 0x80110309
#define SCE_UTILITY_SAVEDATA_ERROR_LOAD_INTERNAL 0x8011030B
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_NO_MS 0x80110381
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_EJECT_MS 0x80110382
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_MS_NOSPACE 0x80110383
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_MS_PROTECTED 0x80110384
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_ACCESS_ERROR 0x80110385
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_DATA_BROKEN 0x80110386
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_PARAM 0x80110388
#define SCE_UTILITY_SAVEDATA_ERROR_SAVE_INTERNAL 0x8011038B
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_NO_MS 0x80110341
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_EJECT_MS 0x80110342
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_MS_PROTECTED 0x80110344
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_ACCESS_ERROR 0x80110345
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_NO_DATA 0x80110347
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_PARAM 0x80110348
#define SCE_UTILITY_SAVEDATA_ERROR_DELETE_INTERNAL 0x8011034B
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_NO_MS 0x801103C1
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_EJECT_MS 0x801103C2
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_ACCESS_ERROR 0x801103C5
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_DATA_BROKEN 0x801103C6
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_NO_DATA 0x801103C7
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_PARAM 0x801103C8
#define SCE_UTILITY_SAVEDATA_ERROR_SIZES_INTERNAL 0x801103CB
#define SCE_UTILITY_SAVEDATA_ERROR_MC_NO_MS 0x80110321
#define SCE_UTILITY_SAVEDATA_ERROR_MC_EJECT_MS 0x80110322
#define SCE_UTILITY_SAVEDATA_ERROR_MC_MS_NOSPACE 0x80110323
#define SCE_UTILITY_SAVEDATA_ERROR_MC_MS_PROTECTED 0x80110324
#define SCE_UTILITY_SAVEDATA_ERROR_MC_ACCESS_ERROR 0x80110325
#define SCE_UTILITY_SAVEDATA_ERROR_MC_DATA_BROKEN 0x80110326
#define SCE_UTILITY_SAVEDATA_ERROR_MC_NO_DATA 0x80110327
#define SCE_UTILITY_SAVEDATA_ERROR_MC_PARAM 0x80110328
#define SCE_UTILITY_SAVEDATA_ERROR_MC_NO_FILE 0x80110329
#define SCE_UTILITY_SAVEDATA_ERROR_MC_SUSPEND_ERROR 0x8011032A
#define SCE_UTILITY_SAVEDATA_ERROR_MC_INTERNAL 0x8011032B
#define SCE_UTILITY_SAVEDATA_ERROR_MC_STATUS_ERROR 0x8011032C
#define SCE_UTILITY_SAVEDATA_ERROR_MC_SECUREFILE_FULL 0x8011032D

typedef enum
{
	SCE_UTILITY_SD_TITLEID_SIZE = 13,
	SCE_UTILITY_SD_USERID_SIZE = 20,
	SCE_UTILITY_SD_FILENAME_SIZE = 13,
	SCE_UTILITY_SD_LISTITEM_MAX = 1024,
	SCE_UTILITY_SD_SIZESTR_SIZE = 8,
	SCE_UTILITY_SD_SECUREFILEID_SIZE = 16,
	SCE_UTILITY_SD_SYSTEMFILELIST_MAX = 5,
	SCE_UTILITY_SD_SECUREFILELIST_MAX = 99,
	SCE_UTILITY_SD_NORMALFILELIST_MAX = 8192,
	SCE_UTILITY_SD_USERIDLIST_MAX = 2048
} SceUtilitySavedataSize;

typedef enum
{
	SCE_UTILITY_SD_SYSF_TITLE_SIZE = 128,
	SCE_UTILITY_SD_SYSF_SD_TITLE_SIZE = 128,
	SCE_UTILITY_SD_SYSF_DETAIL_SIZE = 1024
} SceUtilitySavedataSystemFileParamSize;

#define SCE_UTILITY_SAVEDATA_BIND_UNUSED 0
#define SCE_UTILITY_SAVEDATA_BIND_OK 1
#define SCE_UTILITY_SAVEDATA_BIND_ERROR 2
#define SCE_UTILITY_SAVEDATA_BIND_UNSUPPORTED 3
#define SCE_UTILITY_SAVEDATA_BIND_MYPSP_MASK 0x3

#define SCE_UTILITY_SAVEDATA_OVERWRITEMODE_OFF 0
#define SCE_UTILITY_SAVEDATA_OVERWRITEMODE_ON 1

typedef struct SceUtilitySDSystemFileParam
{
	char title[SCE_UTILITY_SD_SYSF_TITLE_SIZE];
	char savedataTitle[SCE_UTILITY_SD_SYSF_SD_TITLE_SIZE];
	char detail[SCE_UTILITY_SD_SYSF_DETAIL_SIZE];
	unsigned char parentalLev;
	unsigned char typeWriteRemoveUpdateParam;
	unsigned char reserved[2];
} SceUtilitySDSystemFileParam;

#define SCE_UTILITY_SAVEDATA_UPDATE_TITLE (1 << 0)
#define SCE_UTILITY_SAVEDATA_UPDATE_SDTITLE (1 << 1)
#define SCE_UTILITY_SAVEDATA_UPDATE_DETAIL (1 << 2)
#define SCE_UTILITY_SAVEDATA_UPDATE_PARENTALLEV (1 << 3)

typedef struct SceUtilitySDExtFile
{
	void * pDataBuf;
	unsigned int dataBufSize;
	unsigned int dataFileSize;
	char * reserved;
} SceUtilitySDExtFile;

typedef struct SceUtilitySavedataListSaveNewData
{
	SceUtilitySDExtFile icon0;
	const char * pTitle;
} SceUtilitySavedataListSaveNewData;

typedef struct SceUtilitySavedataMsFreeSize
{
	unsigned int msClusterSizeByte;
	unsigned int msFreeCluster;
	unsigned int msFreeSizeKB;
	char msFreeSizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
} SceUtilitySavedataMsFreeSize;

typedef struct SceUtilitySavedataMsDataSize
{
	char titleId[SCE_UTILITY_SD_TITLEID_SIZE];
	char reserved[3];
	char userId[SCE_UTILITY_SD_USERID_SIZE];
	unsigned int cluster;
	unsigned int sizeKB;
	char sizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
	unsigned int size32KB;
	char size32Str[SCE_UTILITY_SD_SIZESTR_SIZE];
} SceUtilitySavedataMsDataSize;

typedef struct SceUtilitySavedataUtilityDataSize
{
	unsigned int cluster;
	unsigned int sizeKB;
	char sizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
	unsigned int size32KB;
	char size32Str[SCE_UTILITY_SD_SIZESTR_SIZE];
} SceUtilitySavedataUtilityDataSize;

typedef enum
{
	SCE_UTILITY_SAVEDATA_INITFOCUS_USERID = 0,
	SCE_UTILITY_SAVEDATA_INITFOCUS_LISTFIRST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_LISTLAST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_DATALATEST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_DATAOLDEST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_DATAFIRST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_DATALAST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_EMPTYFIRST,
	SCE_UTILITY_SAVEDATA_INITFOCUS_EMPTYLAST
} SceUtilitySavedataInitFocus;

#define SCE_UTILITY_SAVEDATA_USERID_NULL "<>"

typedef enum
{
	SCE_UTILITY_SAVEDATA_VERSION_CURRENT = 0,
	SCE_UTILITY_SAVEDATA_VERSION_0,
	SCE_UTILITY_SAVEDATA_VERSION_1,
	SCE_UTILITY_SAVEDATA_VERSION_2
} SceUtilitySavedataVersion;

typedef enum
{
	SCE_UTILITY_SAVEDATA_MC_STATUS_SINGLE = 0,
	SCE_UTILITY_SAVEDATA_MC_STATUS_START,
	SCE_UTILITY_SAVEDATA_MC_STATUS_RELAY,
	SCE_UTILITY_SAVEDATA_MC_STATUS_END
} SceUtilitySavedataMultiCallStatus;

typedef struct SceUtilitySDUserIdStat
{
	SceMode st_mode;
	ScePspDateTime st_ctime;
	ScePspDateTime st_atime;
	ScePspDateTime st_mtime;
	char userId[SCE_UTILITY_SD_USERID_SIZE];
} SceUtilitySDUserIdStat;

typedef struct SceUtilitySavedataUserIdList
{
	unsigned int userIdMax;
	unsigned int userIdNum;
	SceUtilitySDUserIdStat * pUserIds;
} SceUtilitySavedataUserIdList;
 
typedef struct SceUtilitySDFileStat
{
	SceMode st_mode;
	unsigned int reserved;
	SceOff st_size;
	ScePspDateTime st_ctime;
	ScePspDateTime st_atime;
	ScePspDateTime st_mtime;
	char fileName[SCE_UTILITY_SD_FILENAME_SIZE];
	char reserved1[3];
} SceUtilitySDFileStat;

typedef struct SceUtilitySavedataFileList
{
	unsigned int secureFileMax;
	unsigned int normalFileMax;
	unsigned int systemFileMax;
	unsigned int secureFileNum;
	unsigned int normalFileNum;
	unsigned int systemFileNum;
	SceUtilitySDFileStat * pSecureFiles;
	SceUtilitySDFileStat * pNormalFiles;
	SceUtilitySDFileStat * pSystemFiles;
} SceUtilitySavedataFileList;

typedef struct SceUtilitySDFileStat2
{
	SceOff st_size;
	char fileName[SCE_UTILITY_SD_FILENAME_SIZE];
	char reserved[3];
} SceUtilitySDFileStat2;

typedef struct SceUtilitySavedataCheckSize
{
	unsigned int secureFileNum;
	unsigned int normalFileNum;
	SceUtilitySDFileStat2 * pSecureFiles;
	SceUtilitySDFileStat2 * pNormalFiles;
	unsigned int msClusterSizeByte;
	unsigned int msFreeCluster;
	unsigned int msFreeSizeKB;
	char msFreeSizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
	unsigned int createNeedSizeKB;
	char createNeedSizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
	unsigned int overwriteNeedSizeKB;
	char overwriteNeedSizeStr[SCE_UTILITY_SD_SIZESTR_SIZE];
} SceUtilitySavedataCheckSize;

typedef struct SceUtilityParamBase
{
	unsigned int size;
	int message_lang;
	int ctrl_assign;
	int main_thread_priority;
	int sub_thread_priority;
	int font_thread_priority;
	int sound_thread_priority;
	int result;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
} SceUtilityParamBase;

typedef struct SceUtilitySavedataParam660
{
	SceUtilityParamBase base;
	int type;
	unsigned int bind;
	unsigned int overWriteMode;
	char titleId[SCE_UTILITY_SD_TITLEID_SIZE];
	char reserved[3];
	char userId[SCE_UTILITY_SD_USERID_SIZE];
	char (* pUserIds)[SCE_UTILITY_SD_USERID_SIZE];
	char fileName[SCE_UTILITY_SD_FILENAME_SIZE];
	char reserved1[3];
	void * pDataBuf;
	unsigned int dataBufSize;
	unsigned int dataFileSize;
	SceUtilitySDSystemFileParam systemFile;
	SceUtilitySDExtFile icon0;
	SceUtilitySDExtFile icon1;
	SceUtilitySDExtFile pic1;
	SceUtilitySDExtFile snd0;
	SceUtilitySavedataListSaveNewData * pNewData;
	unsigned int initFocus;
	int abortedStatus;
	SceUtilitySavedataMsFreeSize * pMs;
	SceUtilitySavedataMsDataSize * pMsData;
	SceUtilitySavedataUtilityDataSize * pUtilityData;
	unsigned char secureFileId[SCE_UTILITY_SD_SECUREFILEID_SIZE];
	unsigned int dataVersion;
	unsigned int mcStatus;
	SceUtilitySavedataUserIdList * pUserIdList;
	SceUtilitySavedataFileList * pFileList;
	SceUtilitySavedataCheckSize * pCheckSize;
} SceUtilitySavedataParam660;

#endif

