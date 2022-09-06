// make sfo for savedata by Yoti
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SfoFileSize 0x1330 // 4912

#define SfoHeaderMagic 0x46535000
#define SfoHeaderVersion 0x00000101
#define SfoHeaderKtOffset 0x00000094
#define SfoHeaderDtOffset 0x00000108
#define SfoHeaderRecCount 0x00000008

typedef struct {
	unsigned int Magic;
	unsigned int Version;
	unsigned int KtOffset; // keys table
	unsigned int DtOffset; // data table
	unsigned int RecCount; // usually 8
} SfoHeader;

typedef struct {
	unsigned short KtOffset;
	unsigned short DataType;
	unsigned int RecLength;
	unsigned int MaxLength;
	unsigned int DtOffset;
} SfoRecord;

static char *SfoRecordName[] = {
	"CATEGORY",
	"PARENTAL_LEVEL",
	"SAVEDATA_DETAIL",
	"SAVEDATA_DIRECTORY",
	"SAVEDATA_FILE_LIST",
	"SAVEDATA_PARAMS",
	"SAVEDATA_TITLE",
	"TITLE"
};

int main(int argc, char *argv[]) {
	if (argc < 8) {
		printf("make sfo for savedata by Yoti\n");
		printf("usage: mksfosd <args> [name]\n");
		printf("args list:\n");
		printf(" * PARENTAL_LEVEL (from 0 to 11, default 0)\n");
		printf(" * SAVEDATA_DETAIL (text data, description)\n");
		printf(" * SAVEDATA_DIRECTORY (savedata dir name)\n");
		printf(" * SAVEDATA_FILE_LIST (savedata file name[s])\n");
		printf(" * SAVEDATA_PARAMS (from 0 to 3, default 1)\n");
		printf(" * SAVEDATA_TITLE (text data, second line)\n");
		printf(" * TITLE (text data, savedata main title)\n");
		printf("name: output file name (default PARAM.SFO)\n");
		printf("program build print: %s %s\n", __DATE__, __TIME__);
		return 1;
	}

	FILE *fp;
	if (argc < 9)
		fp = fopen("PARAM.SFO", "wb");
	else
		fp = fopen(argv[8], "wb");

	if (fp != NULL) {
		SfoHeader fpSfoHeader;
		memset(&fpSfoHeader, 0, sizeof(fpSfoHeader));
		fpSfoHeader.Magic = SfoHeaderMagic;
		fpSfoHeader.Version = SfoHeaderVersion;
		fpSfoHeader.KtOffset = SfoHeaderKtOffset;
		fpSfoHeader.DtOffset = SfoHeaderDtOffset;
		fpSfoHeader.RecCount = SfoHeaderRecCount;
		fwrite(&fpSfoHeader, sizeof(fpSfoHeader), 1, fp);

		int i;
		SfoRecord fpSfoRecord;
		unsigned int KtOffsetTemp = 0;
		unsigned int DtOffsetTemp = 0;
		for (i = 0; i < fpSfoHeader.RecCount; i++) {
			memset(&fpSfoRecord, 0, sizeof(fpSfoRecord));
			switch (i) {
				case 0: // CATEGORY
					fpSfoRecord.DataType = 0x0204;
					fpSfoRecord.RecLength = 0x00000003; // fixed length for this record
					fpSfoRecord.MaxLength = 0x00000004;
				break;
				case 1: // PARENTAL_LEVEL
					fpSfoRecord.DataType = 0x0404;
					fpSfoRecord.RecLength = 0x00000004; // fixed length for type 0x0404
					fpSfoRecord.MaxLength = 0x00000004;
				break;
				case 2: // SAVEDATA_DETAIL
					fpSfoRecord.DataType = 0x0204;
					if (strlen(argv[i]) > 0x3FF)
						fpSfoRecord.RecLength = 0x00000400;
					else
						fpSfoRecord.RecLength = strlen(argv[i]) + 1; // null-terminated str
					fpSfoRecord.MaxLength = 0x00000400;
				break;
				case 3: // SAVEDATA_DIRECTORY
					fpSfoRecord.DataType = 0x0204;
					if (strlen(argv[i]) > 0x3F)
						fpSfoRecord.RecLength = 0x00000040;
					else
						fpSfoRecord.RecLength = strlen(argv[i]) + 1; // null-terminated str
					fpSfoRecord.MaxLength = 0x00000040;
				break;
				case 4: // SAVEDATA_FILE_LIST
					fpSfoRecord.DataType = 0x0004;
					fpSfoRecord.RecLength = 0x00000C60; // fixed length for type 0x0004
					fpSfoRecord.MaxLength = 0x00000C60; // RecLength is same as MaxLength
				break;
				case 5: // SAVEDATA_PARAMS
					fpSfoRecord.DataType = 0x0004;
					fpSfoRecord.RecLength = 0x00000080; // fixed length for type 0x0004
					fpSfoRecord.MaxLength = 0x00000080; // RecLength is same as MaxLength
				break;
				case 6: // SAVEDATA_TITLE
					fpSfoRecord.DataType = 0x0204;
					if (strlen(argv[i]) > 0x7F)
						fpSfoRecord.RecLength = 0x00000080;
					else
						fpSfoRecord.RecLength = strlen(argv[i]) + 1; // null-terminated str
					fpSfoRecord.MaxLength = 0x00000080;
				break;
				case 7: // TITLE
					fpSfoRecord.DataType = 0x0204;
					if (strlen(argv[i]) > 0x7F)
						fpSfoRecord.RecLength = 0x00000080;
					else
						fpSfoRecord.RecLength = strlen(argv[i]) + 1; // null-terminated str
					fpSfoRecord.MaxLength = 0x00000080;
				break;
			}
			fpSfoRecord.KtOffset = KtOffsetTemp;
			KtOffsetTemp += strlen(SfoRecordName[i]) + 1;
			fpSfoRecord.DtOffset = DtOffsetTemp;
			DtOffsetTemp += fpSfoRecord.MaxLength;
			fwrite(&fpSfoRecord, sizeof(fpSfoRecord), 1, fp);
		}

		for (i = 0; i < fpSfoHeader.RecCount; i++) {
			fwrite(SfoRecordName[i], strlen(SfoRecordName[i]), 1, fp);
			fwrite("\0", 1, 1, fp);
		}
		fwrite("\0", 1, 1, fp); // align

		int argv1 = 0;
		char buf[0xC60];
		char list[0xC60];
		char *token;
		int offset = 0;
		for (i = 0; i < fpSfoHeader.RecCount; i++) {
			memset(buf, 0, sizeof(buf));
			switch (i) {
				case 0: // CATEGORY
					//fwrite("MS\0\0", 4, 1, fp);
					memcpy(buf, "MS", strlen("MS"));
					fwrite(buf, 4, 1, fp);
				break;
				case 1: // PARENTAL_LEVEL
					argv1 = atoi(argv[i]);
					if (argv1 < 0) argv1 = 0;
					else if (argv1 > 11) argv1 = 11;
					fwrite(&argv1, sizeof(argv1), 1, fp);
				break;
				case 2: // SAVEDATA_DETAIL
					memcpy(buf, argv[i], strlen(argv[i]));
					buf[0x000003FF] = 0;
					fwrite(buf, 0x00000400, 1, fp);
				break;
				case 3: // SAVEDATA_DIRECTORY
					memcpy(buf, argv[i], strlen(argv[i]));
					buf[0x0000003F] = 0;
					fwrite(buf, 0x00000040, 1, fp);
				break;
				case 4: // SAVEDATA_FILE_LIST
					memset(list, 0, strlen(list));
					strncpy(list, argv[i], sizeof(list));

					token = strtok(list, " ");
					while (token != NULL) {
						// name max length is 12 (8.3 style)
						strncpy(&buf[offset], token, 12);
						// after name + \0 is 16 byte hash
						offset += 0x20;
						// total file record size is 0x20
						token = strtok(NULL, " ");
					}

					fwrite(buf, 0x00000C60, 1, fp);
				break;
				case 5: // SAVEDATA_PARAMS
					// First 16 bytes is SAVEDATA_PARAMS
					if (atoi(argv[i]) == 0)
						buf[0] = 0x00;
					else if (atoi(argv[i]) == 1)
						buf[0] = 0x01;
					else if (atoi(argv[i]) == 2)
						buf[0] = 0x21;
					else if (atoi(argv[i]) == 3)
						buf[0] = 0x41;
					else
						buf[0] = 0x01;
					// [ALL] Second 32 bytes are 2 hashes
					// [21/41] Last 16 bytes is some hash
					fwrite(buf, 0x00000080, 1, fp);
				break;
				case 6: // SAVEDATA_TITLE
					memcpy(buf, argv[i], strlen(argv[i]));
					buf[0x0000007F] = 0;
					fwrite(buf, 0x00000080, 1, fp);
				break;
				case 7: // TITLE
					memcpy(buf, argv[i], strlen(argv[i]));
					buf[0x0000007F] = 0;
					fwrite(buf, 0x00000080, 1, fp);
				break;
			}
		}
		fclose(fp);
	} else {
		printf("Can't open ");
		if (argc < 9)
			printf("PARAM.SFO");
		else
			printf(argv[8]);
		printf(" for writing!\n");
		return 2;
	}

	return 0;
}
