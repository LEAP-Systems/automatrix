#ifndef MMAP_H_
#define MMAP_H_

#include <stdint.h>
#include <stddef.h>

/*
 * Raspberry Pi 4 peripheral mapping documentation:
 * https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf
 */
#define BCM2711_PERI_BASE       0xFE000000
#define BCM2711_PERI_BUS_BASE   0x7E000000
#define PAGE_SIZE               4096

void *map_mem(uint32_t base, size_t size);
void unmap_mem(void *addr, size_t size);

#endif // MMAP_H_