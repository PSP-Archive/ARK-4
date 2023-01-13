#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "hacker.h"
#include "common.h"
#include "gfx.h"

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))


static void generateGoto(int i, int r, char* code){
    snprintf(code, MAX_CHARS, "GOTO %p", (i*r)/31);
}

static void generateALUop(int i, int r, char* code){
    static char* alu_ops[] = {
        (char*)"and", (char*)"or", (char*)"xor", (char*)"sll", (char*)"slr",
        (char*)"add", (char*)"sub", (char*)"mul", (char*)"div", (char*)"pow",
    };
    int op = (i*code[0]+r) % NELEMS(alu_ops);
    u32 r1 = ((u32)code*(u32)r) % 32;
    u32 r2 = ((u32)r/(u32)code) % 32;
    u32 r3 = ((i*r)/(u32)code) % 32;

    snprintf(code, MAX_CHARS, "%s $r%d, $r%d, $r%d", alu_ops[op], r1, r2, r3);
}

static void generateFunctionCall(int i, int r, char* code){
    u32 p = r/(i+1);
    snprintf(code, MAX_CHARS, "call sub_%d()", p);
}

static void generateForLoop(int i, int r, char* code){
    u32 m = r%(i+1) + 7;
    snprintf(code, MAX_CHARS, "for (int i=0; i<%d; i++)", m);
}

static void generateIfCondition(int i, int r, char* code){
    static char* if_cmp[] = {
        (char*)"==", (char*)"!=",
        (char*)">",  (char*)">=",
        (char*)"<",  (char*)"<=",
    };
    int op = (i*code[0]+r) % NELEMS(if_cmp);
    u32 r1 = ((u32)code*(u32)r) % 32;
    u32 r2 = ((u32)r/(u32)code) % 32;
    snprintf(code, MAX_CHARS, "if ( $r%d %s $r%d )", r1, if_cmp[op], r2);
}

static void generatePointer(int i, int r, char* code){
    u32 e = ((u32)code+r) / (i+1);
    snprintf(code, MAX_CHARS, "%p = %d", e, i);
}

static void generateSyscall(int i, int r, char* code){
    u32 e = ((u32)code+r) % 255;
    snprintf(code, MAX_CHARS, "SYSCALL %d", e);
}


void generateHackerCode(int i, int r, char* code){
    static void* functions[] = {
        (void*)&generateGoto,
        (void*)&generateALUop,
        (void*)&generateALUop,
        (void*)&generateALUop,
        (void*)&generateALUop,
        (void*)&generateALUop,
        (void*)&generateIfCondition,
        (void*)&generateIfCondition,
        (void*)&generateIfCondition,
        (void*)&generateFunctionCall,
        (void*)&generateForLoop,
        (void*)&generatePointer,
        (void*)&generateSyscall,
    };

    int f = r % NELEMS(functions);
    void (*generator)(int, int, char*) = (void (*)(int, int, char*))(functions[f]);
    generator(i, r, code);
}

Hacker::Hacker(){
    r = rand();
    cur_row = 0;
    memset(caRow, 0, sizeof(caRow));
}

Hacker::~Hacker(){
}

void Hacker::draw(){

    if (cur_row < MAX_ROWS-1){
        cur_row++;
    }
    else{
        cur_row = 0;
        r = rand();
    }

    r += 7;
    generateHackerCode(cur_row, r, &(caRow[cur_row][0]));
    
    int yoffset = 10;
    int xoffset = 10;
    for (int i=0; i<MAX_ROWS; i++){
        common::printText(xoffset, yoffset, &(caRow[i][0]), GREEN, SIZE_MEDIUM);
        yoffset += 20;
        if (yoffset >= 272){
            yoffset = 10;
            xoffset += 160;
        }
    }
}

bool Hacker::drawBackground(){
    return false;
}
