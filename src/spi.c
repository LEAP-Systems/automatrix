
#include <stdio.h>
#include <assert.h>

#include "spi.h"

static volatile spi_t *spi_reg;

static void spi_print_registers() {
  assert(spi_reg != NULL);
  printf("CS:  \t 0x%08x\n", spi_reg->cs);
  printf("FIFO:\t 0x%08x\n", spi_reg->fifo);
  printf("CLK: \t 0x%08x\n", spi_reg->clk);
  printf("DLEN:\t 0x%08x\n", spi_reg->dlen);
  printf("LTOH:\t 0x%08x\n", spi_reg->ltoh);
  printf("DC:  \t 0x%08x\n", spi_reg->dc);
}

void spi_init(uint8_t device) {
  // map spi device register to userspace
  spi_reg = map_mem(SPI_DEV_PHYS_BASE(device), SPI_REGISTER_LEN);
  spi_print_registers();
  printf("SPI device %i initialized", device);
}