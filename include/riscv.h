#ifndef __RISCV_H__
#define __RISCV_H__

#include "runtime.h"

int32_t lw(uint32_t address, int32_t kte);
int32_t lb(uint32_t address, int32_t kte);
int32_t lbu(uint32_t address, int32_t kte);
int32_t lh(uint32_t address, int32_t kte);
int32_t lhu(uint32_t address, int32_t kte);
int32_t lui(uint32_t address, int32_t kte);

void sw(uint32_t address, int32_t kte, int32_t dado);
void sb(uint32_t address, int32_t kte, int8_t dado);
void sh(uint32_t address, int32_t kte, int8_t dado);

void ecall();

// Auxiliary function for signed and unsigned extension
int32_t sext(uint8_t byte);
int32_t uext(uint8_t byte);

#endif
