#include "cstdlib"
#include "bsod.h"
#include "common.h"
#include "gfx.h"

extern char GetChar(int iGenerator, char cBase, int iRange);

// Register String Literals
static const unsigned char regName[32][5] =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

BSoD::BSoD(){
    cur_col = rand()%MAX_CHARS;
    cur_row = 0;
    r=7;

    // Output a random row of characters
    for (int i=0; i<MAX_ROWS; i++){
        for (int j=0; j<MAX_CHARS; j++){
            caRow[i][j] = GetChar(r + j*j, 33, 30);
        }
        caRow[i][MAX_CHARS] = 0;
        r += 31;
    }
}

BSoD::~BSoD(){
}

void BSoD::draw(){

    if (caRow[cur_row][cur_col] != ' '){
        caRow[cur_row][cur_col] = ' ';
    }
    else{
        cur_row++;
        r += 78;
        caRow[cur_row-1][cur_col] = GetChar(r + cur_col*cur_col, 33, 30);
        if (cur_row >= MAX_ROWS){
            cur_col = rand()%MAX_CHARS;
            cur_row = 0;
        }
    }

    ya2d_clear_screen(BLUE_COLOR);

    u32* random_data = (u32*)caRow;
    static char temp[255];

    common::printText(10, 18, "Exception caught!", WHITE, SIZE_MEDIUM);
    common::printText(20, 35, "Type - Address load/inst fetch", WHITE);

    snprintf(temp, 255, "EPC       - 0x%8.8X", random_data[0]);
    common::printText(20, 55, temp, WHITE);

    snprintf(temp, 255, "Cause     - 0x%8.8X", random_data[1]);
    common::printText(20, 75, temp, WHITE);

    snprintf(temp, 255, "Status    - 0x%8.8X", random_data[2]);
    common::printText(20, 95, temp, WHITE);

    snprintf(temp, 255, "BadVAddr  - 0x%8.8X", random_data[3]);
    common::printText(20, 115, temp, WHITE);

    int yoffset = 135;
    for(int i = 0; i < 32; i+=4){
        snprintf(temp, 255, "%s:0x%8.8X %s:0x%8.8X %s:0x%8.8X %s:0x%8.8X", regName[i], (i==0)?0:random_data[i+4],
                regName[i+1], random_data[i+5], regName[i+2], random_data[i+6], regName[i+3], (i==24)?ARK_CONFIG_MAGIC:random_data[i+7]);
        common::printText(20, yoffset, temp, WHITE, SIZE_MEDIUM);
        yoffset += 17;
    }
    
    yoffset = 50;
    common::printText(220, yoffset-20, "Core Dump", WHITE, SIZE_MEDIUM);
    for (int i=0; i<MAX_ROWS; i++){
        common::printText(225, yoffset, &(caRow[i][0]), WHITE, SIZE_LITTLE);
        yoffset += 10;
    }
}

bool BSoD::drawBackground(){
    return false;
}