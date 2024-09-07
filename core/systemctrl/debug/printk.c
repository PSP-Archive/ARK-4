/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

/*
 *
 * printk.c
 *
 * printk for bootbox
 *
 * support format:
 *  %c
 *  %s
 *  %[[0]n]x
 *  %[[0]n]X
 *  %[[0]n]d
 *
 */

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <systemctrl_private.h>
#include <stdarg.h>
#include <string.h>
#include <ark.h>

#if DEBUG >= 3

// Initialize printk
int printkInit(const char* filename);

// Log info
int printk(char *fmt, ...)__attribute__((format (printf, 1, 2)));

// Output all info remaining in the memory to file
int printkSync(void);

// Lock printk
void printkLock(void);

// Unlock printk
void printkUnlock(void);

struct pthread_mlock_t {
    volatile unsigned long l;
    unsigned int c;
    int thread_id;
};

typedef struct pthread_mlock_t MLOCK_T;

static MLOCK_T lock;

static int itostr(char *buf, int in_data, int base, int upper, int sign)
{
    int res, len, i;
    unsigned int data;
    char *str;

    if(base==10 && sign && in_data<0){
        data = -in_data;
    }else{
        data = in_data;
    }

    str = buf;
    do{
        res = data%base;
        data = data/base;
        if(res<10){
            res += '0';
        }else{
            if(upper){
                res += 'A'-10;
            }else{
                res += 'a'-10;
            }
        }
        *str++ = res;
    }while(data);
    len = str-buf;

    /* reverse digital order */
    for(i=0; i<len/2; i++){
        res = buf[i];
        buf[i] = buf[len-1-i]; 
        buf[len-1-i] = res; 
    }

    return len;
}

/*
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 */

#define OUT_C(c) \
    if(str<end){ \
        *str++ = (c); \
    } else { \
        goto exit; \
    }

static char digital_buf[32];
int vsnprintf(char *buf, int size, char *fmt, va_list args)
{
    char ch, *s, *str, *end, *sstr;
    int zero_pad, left_adj, add_sign, field_width, sign;
    int i, base, upper, len;


    if(!buf || !fmt ||!size){
        return 0;
    }

    str = buf;
    end = buf+size;

    while(*fmt){
        if(*fmt!='%'){
            OUT_C(*fmt++);
            continue;
        }

        /* skip '%' */
        sstr = fmt;
        fmt++;

        /* %% */
        if(*fmt=='%'){
            OUT_C(*fmt++);
            continue;
        }

        /* get flag */
        zero_pad = ' ';
        left_adj = 0;
        add_sign = 0;
        while((ch=*fmt)){

            if(*fmt=='0'){
                zero_pad = '0';
            }else if(*fmt=='-'){
                left_adj = 1;
            }else if(*fmt=='#'){
            }else if(*fmt==' '){
                if(add_sign!='+')
                    add_sign = ' ';
            }else if(*fmt=='+'){
                add_sign = '+';
            }else{
                break;
            }
            fmt++;
        }

        /* get field width: m.n */
        field_width = 0;
        /* get m */
        while(*fmt && *fmt>'0' && *fmt<='9'){
            field_width = field_width*10+(*fmt-'0');
            fmt++;
        }
        if(*fmt && *fmt=='.'){
            fmt++;
            /* skip n */
            while(*fmt && *fmt>'0' && *fmt<='9'){
                fmt++;
            }
        }

        /* get format char */
        upper = 0;
        base = 0;
        sign = 0;
        len = 0;
        s = digital_buf;
        while((ch=*fmt)){
            fmt++;
            switch(ch){
            /* hexadecimal */
            case 'p':
            case 'X':
                upper = 1;
            case 'x':
                base = 16;
                break;

            /* decimal */
            case 'd':
            case 'i':
                sign = 1;
            case 'u':
                base = 10;
                break;

            /* octal */
            case 'o':
                base = 8;
                break;

            /* character */
            case 'c':
                digital_buf[0] = (unsigned char) va_arg(args, int);
                len = 1;
                break;

            /* string */
            case 's':
                s = va_arg(args, char *);
                if(!s) s = "<NUL>";
                len = strlen(s);
                break;

            /* float format, skip it */
            case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
                va_arg(args, double);
                s = NULL;
                break;

            /* length modifier */
            case 'l': case 'L': case 'h': case 'j': case 'z': case 't':
                /* skip it */
                continue;

            /* bad format */
            default:
                s = sstr;
                len = fmt-sstr;
                break;
            }
            break;
        }

        if(base){
            i = va_arg(args, int);
            if(base==10 && sign){
                if(i<0){
                    add_sign = '-';
                }
            }else{
                add_sign = 0;
            }

            len = itostr(digital_buf, i, base, upper, sign);
        }else{
            zero_pad = ' ';
            add_sign = 0;
        }

        if(s){
            if(len>=field_width){
                field_width = len;
                if(add_sign)
                    field_width++;
            }
            for(i=0; i<field_width; i++){
                if(left_adj){
                    if(i<len){
                        OUT_C(*s++);
                    }else{
                        OUT_C(' ');
                    }
                }else{
                    if(add_sign && (zero_pad=='0' || i==(field_width-len-1))){
                        OUT_C(add_sign);
                        add_sign = 0;
                        continue;
                    }
                    if(i<(field_width-len)){
                        OUT_C(zero_pad);
                    }else{
                        OUT_C(*s++);
                    }
                }
            }
        }
    }

    OUT_C(0);

exit:
    return str-buf;
}

static char printk_buf[256];
static const char *printk_output_fn;
static char default_path[ARK_PATH_SIZE] = {0};

static char printk_memory_log[1024*4] __attribute__((aligned(64)));
static char *printk_memory_log_ptr = printk_memory_log;

static void flushPrintkMemoryLog(int fd, int kout)
{
    int ret;

    sceIoWrite(fd, printk_memory_log, printk_memory_log_ptr - printk_memory_log);
    ret = sceIoWrite(kout, printk_memory_log, printk_memory_log_ptr - printk_memory_log);

    if (ret >= 0) {
        memset(printk_memory_log, 0, sizeof(printk_memory_log));
        printk_memory_log_ptr = printk_memory_log;
    }
}

static void appendToMemoryLog(int printed_len)
{
    if (printk_memory_log_ptr + printed_len < printk_memory_log + sizeof(printk_memory_log)) {
        memcpy(printk_memory_log_ptr, printk_buf, printed_len);
        printk_memory_log_ptr += printed_len;
    }
}

static int printkOpenOutput(void)
{
    int fd;

    if (printk_output_fn == NULL) {
        strcpy(default_path, ark_config->arkpath);
        strcat(default_path, "LOG.TXT");
        printk_output_fn = default_path;
    }

    fd = sceIoOpen(printk_output_fn, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);

    return fd;
}

static void printkOutput(int printed_len)
{
    int fd, kout;

    kout = sceKernelStdout();
    sceIoWrite(kout, printk_buf, printed_len);
    fd = printkOpenOutput();

    if (fd >= 0) {
        if (printk_memory_log_ptr > printk_memory_log) {
            flushPrintkMemoryLog(fd, kout);
        }

        sceIoWrite(fd, printk_buf, printed_len);
        sceIoClose(fd);
    } else {
        appendToMemoryLog(printed_len);
    }

    sceKernelDelayThread(10000);
}

int isCpuIntrEnabled(void)
{
    int ret;

    __asm__ volatile ("mfic    %0, $0\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            : "=r"(ret)
            );

    return ret;
}

int printkCached(char *fmt, ...)
{
    va_list args;
    int printed_len;
    u32 k1;

    if ( 0 )
        return 0;

    k1 = pspSdkSetK1(0);

    va_start(args, fmt);
    printed_len = vsnprintf(printk_buf, sizeof(printk_buf), fmt, args);
    va_end(args);
    printed_len--;
    appendToMemoryLog(printed_len);

    pspSdkSetK1(k1);

    return printed_len;
}

int printk(char *fmt, ...)
{
    va_list args;
    int printed_len;
    u32 k1;

    k1 = pspSdkSetK1(0);

    if (0 == isCpuIntrEnabled()) {
        // interrupt disabled, let's do the work quickly before the watchdog bites
        va_start(args, fmt);
        printed_len = vsnprintf(printk_buf, sizeof(printk_buf), fmt, args);
        va_end(args);
        printed_len--;
        appendToMemoryLog(printed_len);
    } else {
        printkLock();
        va_start(args, fmt);
        printed_len = vsnprintf(printk_buf, sizeof(printk_buf), fmt, args);
        va_end(args);
        printed_len--;
        printkOutput(printed_len);
        printkUnlock();
    }

    pspSdkSetK1(k1);

    return printed_len;
}

static unsigned long InterlockedExchange(unsigned long volatile *dst, unsigned long exchange)
{
    unsigned int flags = pspSdkDisableInterrupts();
    unsigned long origvalue = *dst;

    *dst = exchange;
    pspSdkEnableInterrupts(flags);

    return origvalue;
}

static inline int psp_mutex_lock(MLOCK_T *s)
{
    for (;;) {
        if (s->l != 0) {
            if (s->thread_id == sceKernelGetThreadId()) {
                ++s->c;
                return 0;
            }
        } else {
            if (!InterlockedExchange(&s->l, 1)) {
                s->thread_id = sceKernelGetThreadId();
                s->c = 1;
                return 0;
            }
        }

        sceKernelDelayThread(1000);
    }
    
    return 0;
}

static inline void psp_mutex_unlock(MLOCK_T *s)
{
    if (--s->c == 0) {
        s->thread_id = 0;
        InterlockedExchange(&s->l, 0);
    }
}

void printkLock(void)
{
    psp_mutex_lock(&lock);
}

void printkUnlock(void)
{
    psp_mutex_unlock(&lock);
}

int printkInit(const char *output)
{
    static char dynamic_path[ARK_PATH_SIZE];
    
    MLOCK_T *s = &lock;

    s->l = 0;
    s->c = 0;
    s->thread_id = sceKernelGetThreadId();

    if (output != NULL)
    {
        strcpy(dynamic_path, output);
        printk_output_fn = dynamic_path;
    }
    
    return 0;
}

int printkSync(void)
{
    int fd, kout;

    kout = sceKernelStdout();
    fd = printkOpenOutput();

    if (fd >= 0) {
        if (printk_memory_log_ptr > printk_memory_log) {
            flushPrintkMemoryLog(fd, kout);
        }

        sceIoClose(fd);
        sceKernelDelayThread(10000);
    }

    return 0;
}

#else
int printkCached(char *fmt, ...){return 0;}
int printk(char *fmt, ...){return 0;}
int printkInit(const char *output){return 0;}
int printkSync(void){return 0;}
#endif