#include "exception.h"

#include <cpu.h>

#include <string.h>

// EXCEPTION_MAX is used extensively below. if for whatever reason this value
// needs to be changed, then ensure that everything consuming it still works
#define EXCEPTION_MAX               (32)
#define GET_TABLE_ENTRY_FLAGS(x)    ((uintptr_t)x & 0b11)
#define SET_TABLE_ENTRY_FLAGS(a, x) ((EXCEPTION_HANDLER *)((uintptr_t)a | x))
#define TABLE_ENTRY_FLAG_OCCUPIED   (1 << 0)

extern const unsigned char *nmi_entry;
extern size_t nmi_entry_size;

extern EXCEPTION_HANDLER default_nmi_handler;
extern EXCEPTION_HANDLER default_ebase_handler;

extern void exception_set_table(EXCEPTION_HANDLER **table);
extern void exception_set_nmi_handler(EXCEPTION_HANDLER handler);
extern void exception_set_ebase_handler(EXCEPTION_HANDLER handler);

static void inf_loop(void)
{
    while (1);
}

static EXCEPTION_HANDLER *s_exception_table[EXCEPTION_MAX] = { 0 };
static EXCEPTION_HANDLER *g_default_exception_handler = inf_loop;

static void rebuild_exception_table(void)
{
    for (size_t i = 0; i < EXCEPTION_MAX; ++i) {
        unsigned int flags = GET_TABLE_ENTRY_FLAGS(s_exception_table[i]);

        if (!(flags & TABLE_ENTRY_FLAG_OCCUPIED)) {
            s_exception_table[i] = g_default_exception_handler;
        }
    }
}

void exception_register_handler(enum ExceptionType cause, EXCEPTION_HANDLER *handler)
{
    unsigned int intr = cpu_suspend_interrupts();
    s_exception_table[cause] = SET_TABLE_ENTRY_FLAGS(handler, TABLE_ENTRY_FLAG_OCCUPIED);
    cpu_resume_interrupts(intr);
}

void exception_register_default_handler(EXCEPTION_HANDLER *handler)
{
    unsigned int intr = cpu_suspend_interrupts();
    g_default_exception_handler = handler;
    rebuild_exception_table();
    cpu_resume_interrupts(intr);
}

void exception_init(void)
{
    unsigned int intr = cpu_suspend_interrupts();
    rebuild_exception_table();

    exception_set_table(s_exception_table);
    exception_set_nmi_handler(default_nmi_handler);
    exception_set_ebase_handler(default_ebase_handler);

    memcpy((void *)0xBFC00000, &nmi_entry, nmi_entry_size);
    cpu_dcache_wb_inv_all();
    cpu_icache_inv_all();

    cpu_resume_interrupts(intr);
}
