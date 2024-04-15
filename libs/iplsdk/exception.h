#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>

enum ExceptionType
{
    EXCEPTION_IRQ = 0,
    EXCEPTION_SYSCALL = 8
};

typedef void (EXCEPTION_HANDLER)(void);

void exception_init(void);

void exception_register_handler(enum ExceptionType cause, EXCEPTION_HANDLER *handler);
void exception_register_default_handler(EXCEPTION_HANDLER *handler);

#ifdef __cplusplus
}
#endif //__cplusplus
