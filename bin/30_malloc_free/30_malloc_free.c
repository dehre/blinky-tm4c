//******************************************************************************
//
// A simplified heap manager that handles just fixed size blocks.
//
// Initialization must be performed before the heap can be used.
// `Heap_Init` partitions the heap into blocks and links them all together.
// `BLOCK_SIZE` is the number of 8-bit bytes in each block.
// All blocks allocated and released with this memory manager will be of
//   this fixed size.
//
// The variable `freePt` points to a linear linked list of free blocks.
// Initially these free blocks are contiguous and in order, but as the manager
//   is used, the positions and order of the free blocks can vary.
// It will be the pointers that will link the free blocks together.
//
// To allocate a block, the manager just removes one block from the free list.
// The `Heap_Allocate` function will fail and return a NULL pointer when the
//   heap becomes empty.
// The `Heap_Release` function returns a block to the free list.
// This system does not check to verify a released block actually was
//   previously allocated.
//
// The following symbols, created by the linker, are used to determine the
//   addresses and size in memory of stack and heap:
//
//     extern uint32_t __STACK_TOP;
//     extern uint32_t _sys_memory;
//     extern uint32_t __SYSMEM_SIZE;
//
// Useful resources about linker scripts:
// * https://blog.thea.codes/the-most-thoroughly-commented-linker-script
// * https://github.com/wntrblm/Castor_and_Pollux/blob/main/firmware/scripts/samd21g18a.ld
// * https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/518115/tiva-c-ccs-ver-6-1-2-c-dynamic-allocation
// * https://downloads.ti.com/docs/esd/SPNU118N/Content/SPNU118N_HTML/linker_description.html#STDZ0752477
// * https://software-dl.ti.com/ccs/esd/documents/sdto_cgt_Linker-Command-File-Primer.html
// * https://stackoverflow.com/questions/8398755/access-symbols-defined-in-the-linker-script-by-application
//
// For reference:
//   book "Real-Time Operating Systems for ARM Cortex-M Microcontrollers"
//   page 254.
// Date: 06-01-2022
//
//******************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/debug.h>
#include "uart-init.h"
#include <utils/uartstdio.h>

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define NULL 0
#define BLOCK_SIZE 80 // size of each memory block in bytes

//
// Linker Symbols
//
extern uint32_t __STACK_TOP;
extern uint32_t _sys_memory;
extern uint32_t __SYSMEM_SIZE;

static uint32_t stackStartAddress = (uint32_t)&__STACK_TOP;
static uint32_t heapStartAddress = (uint32_t)&_sys_memory;
static uint32_t heapSize = (uint32_t)&__SYSMEM_SIZE;
#define heapEndAddress (heapStartAddress + heapSize)

static uint32_t *freePt;

void Heap_Init(void);
void *Heap_Allocate(void);
void Heap_Release(void *pt);

int main(void)
{
    UART_Init();
    UARTprintf("Stack address: 0x%x\n", stackStartAddress);
    UARTprintf("Heap  address: 0x%x\n", heapStartAddress);
    UARTprintf("Heap  size:    0x%x (%d bytes)\n", heapSize, heapSize);
    UARTprintf("Heap  blocks:  %d\n\n", (heapSize / BLOCK_SIZE));

    Heap_Init();

    char *strA = Heap_Allocate();
    strcpy(strA, "Hello... ");

    char *strB = Heap_Allocate();
    strcpy(strB, "world!");

    strncat(strA, strB, strlen(strB));
    UARTprintf("strA: %s\n\n", strA);

    Heap_Release(strA);
    Heap_Release(strB);
    while (1)
    {
    }
}

void Heap_Init(void)
{
    freePt = (uint32_t *)heapStartAddress;

    // Pointer arithmetic on uint32_t increments the memory address 4 bytes
    //   each time, so the value of BLOCK_SIZE, in bytes, must be divided by 4.
    uint32_t *iteratingPt;
    for (iteratingPt = freePt;
         iteratingPt < (uint32_t *)heapEndAddress;
         iteratingPt += (BLOCK_SIZE / 4))
    {
        *iteratingPt = (uint32_t)(iteratingPt + (BLOCK_SIZE / 4));
    }

    *iteratingPt = NULL;
}

void *Heap_Allocate(void)
{
    uint32_t *pt = freePt;
    if (pt != NULL)
    {
        freePt = (uint32_t *)*pt;
    }
    return pt;
}

void Heap_Release(void *pt)
{
    uint32_t *oldFreePt = freePt;
    freePt = (uint32_t *)pt;
    *freePt = (uint32_t)oldFreePt;
}
