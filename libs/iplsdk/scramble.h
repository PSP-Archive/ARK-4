#pragma once

#include <stddef.h>
#include <stdint.h>

void scramble(volatile uint32_t *dst, const uint32_t *src, size_t len, size_t ppn);
void descramble(void *dst, uintptr_t src, size_t len, size_t ppn);
