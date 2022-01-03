#include <stdint.h>
#include <stdbool.h>
#include "os.h"

#include "semaphore-fifo.h"

static uint32_t fifo[FIFO_SIZE];

static volatile uint32_t *putPt;
static volatile uint32_t *getPt;

static int32_t currentSize;
static int32_t roomLeft;
static int32_t fifoMutex;

void SemaphoreFifo_Init(void)
{
    putPt = getPt = &fifo[0];
    currentSize = 0;
    roomLeft = FIFO_SIZE;
    fifoMutex = 1;
}

void SemaphoreFifo_Put(uint32_t data)
{
    OS_SemaphoreWait(&roomLeft);
    OS_SemaphoreWait(&fifoMutex);

    *putPt = data;
    putPt++;
    if (putPt == &fifo[FIFO_SIZE])
    {
        // wrap
        putPt = &fifo[0];
    }

    OS_SemaphoreSignal(&fifoMutex);
    OS_SemaphoreSignal(&currentSize);
}

uint32_t SemaphoreFifo_Get(void)
{
    OS_SemaphoreWait(&currentSize);
    OS_SemaphoreWait(&fifoMutex);

    uint32_t data = *getPt;
    getPt++;
    if (getPt == &fifo[FIFO_SIZE])
    {
        // wrap
        getPt = &fifo[0];
    }

    OS_SemaphoreSignal(&fifoMutex);
    OS_SemaphoreSignal(&roomLeft);
    return data;
}
