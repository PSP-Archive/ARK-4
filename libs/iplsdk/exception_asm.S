.set noreorder
.set noat

#include "controlreg.h"

.global nmi_entry
nmi_entry:

    ctc0    $v0, CTRL_EXCEPTION_V0
    cfc0    $v0, CTRL_NMI_HANDLER
    jr      $v0
    nop

.global nmi_entry_size
nmi_entry_size:
.dword nmi_entry_size - nmi_entry

.global exception_set_table
exception_set_table:
    ctc0    $a0, CTRL_EXCEPTION_TABLE
    jr      $ra
    nop

.global exception_set_nmi_handler
exception_set_nmi_handler:
    ctc0    $a0, CTRL_NMI_HANDLER
    jr      $ra
    nop

.global exception_set_ebase_handler
exception_set_ebase_handler:
    mtc0    $a0, $25
    jr      $ra
    nop

.global default_nmi_handler
default_nmi_handler:
    ctc0    $v1, CTRL_EXCEPTION_V1

    // we're going to call exception 31, representing the NMI. since cause is
    // shifted left 2 bits, then the actual value is 0x7C
    b       execute_table_with_cause_v0
    li      $v0, 0x7C

.global default_ebase_handler
default_ebase_handler:
    ctc0    $v0, CTRL_EXCEPTION_V0
    ctc0    $v1, CTRL_EXCEPTION_V1

    // get the exception cause. this starts at bit 2, and we are purposefully
    // not shifting it because we will later be indexing a pointer array with it
    mfc0    $v0, REG_CAUSE
    andi    $v0, $v0, 0x7C // $v0 cause * 4, effectively

    // fall through to execute_table_with_cause_v0

// ensure that v0, and v1 are suitable backed up prior to calling this function
execute_table_with_cause_v0:
    // get the exception table and execute the handler connected to it
    cfc0    $v1, CTRL_EXCEPTION_TABLE
    addu    $v0, $v0, $v1 // exception_table[cause]
    lw      $v0, 0($v0)
    ins     $v0, $0, 0, 2 // the bottom two bits hold flags
    jr      $v0
    nop
