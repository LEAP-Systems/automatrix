
#include <stdio.h>
#include <assert.h>

#include "spi.h"

static volatile spi_t *spi_reg;
static spi_config_t spi_config;

static void spi_print_registers() {
  assert(spi_reg != NULL);
  printf("CS:  \t 0x%08x\n", spi_reg->cs);
  printf("FIFO:\t 0x%08x\n", spi_reg->fifo);
  printf("CLK: \t 0x%08x\n", spi_reg->clk);
  printf("DLEN:\t 0x%08x\n", spi_reg->dlen);
  printf("LTOH:\t 0x%08x\n", spi_reg->ltoh);
  printf("DC:  \t 0x%08x\n", spi_reg->dc);
}

void spi_init(spi_config_t *spi_config) {
  // set module global spi config
  spi_config = spi_config;
  // map spi device register to userspace
  spi_reg = map_mem(SPI_DEV_PHYS_BASE(spi_config->device), SPI_REGISTER_LEN);
  spi_reg->clk = spi_config->speed;
  spi_reg->cs |= SPI_CS_CPHA(1) | SPI_CS_CPOL(0);
  spi_print_registers();
  printf("SPI device %i initialized", spi_config->device);
}

/**
 * @brief Polling Mode SPI Transact
 * 
 * @param tx_buf uint8_t pointer to transmit data buffer
 * @param buf_size transmit data buffer size
 */
void spi_transact(uint8_t *tx_buf, size_t buf_size) {
  // buffer overflow guard
  assert(sizeof(tx_buf)/sizeof(uint8_t) >= buf_size);
  // start spi transfer
  spi_reg->cs |= SPI_CS_TA(1);
  for (int i = 0; i <= buf_size; i++) {
    // wait for tx fifo availability
    do { } while (spi_reg->cs & SPI_CS_TXD);
    // write buffer frame to fifo register for load
    spi_reg->fifo = tx_buf[i];
    // wait for spi frame transfer to complete
    do { } while (spi_reg->cs & SPI_CS_DONE);
  }
  // stop spi transfer
  spi_reg->cs |= SPI_CS_TA(0);
  printf("spi transaction successful");
}