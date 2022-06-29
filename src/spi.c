#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "spi.h"

static volatile spi_t *spi_reg;
static const spi_config_t *_spi_config;

static void spi_print_registers(void) {
  assert(spi_reg != NULL);
  printf("SPI Device %i\n", _spi_config->device);
  printf("\tCS:  \t 0x%08x\n", spi_reg->cs);
  printf("\tFIFO:\t 0x%08x\n", spi_reg->fifo);
  printf("\tCLK: \t 0x%08x\n", spi_reg->clk);
  printf("\tDLEN:\t 0x%08x\n", spi_reg->dlen);
  printf("\tLTOH:\t 0x%08x\n", spi_reg->ltoh);
  printf("\tDC:  \t 0x%08x\n", spi_reg->dc);
}

void spi_init(const spi_config_t *spi_config) {
  // set module global spi config
  _spi_config = spi_config;
  printf("spi device: %u\n", _spi_config->device);
  printf("spi mode: %u\n", _spi_config->mode);
  printf("spi speed: %llu\n", _spi_config->speed);
  // map spi device register to userspace
  spi_reg = map_mem(SPI_DEV_PHYS_BASE(_spi_config->device), SPI_REGISTER_LEN);
  spi_reg->clk = _spi_config->speed;
  spi_reg->cs |= SPI_CS_CPHA(0) | SPI_CS_CPOL(0);
  if (_spi_config->mode == 1) {
    // disable dma requests
    spi_reg->cs |= SPI_CS_DMAEN(0);
  }
  spi_print_registers();
  printf("SPI device %i initialized\n", _spi_config->device);
}

/**
 * @brief Polling Mode SPI Transact
 * 
 * @param tx_buf uint8_t pointer to transmit data buffer
 * @param buf_size transmit data buffer size
 */
void spi_transact(const uint8_t *tx_buf, size_t buf_size) {
  // start spi transfer
  spi_reg->cs |= SPI_CS_TA;
  for (size_t i = 0; i <= buf_size; i++) {
    // wait for tx fifo availability
    printf("Waiting for tx fifo availability\n");
    do { 
      printf("spi cs: %08x\n", spi_reg->cs);
      usleep(100);
    } while ((spi_reg->cs & SPI_CS_TXD) == 0);
    // write buffer frame to fifo register for load
    spi_reg->fifo |= tx_buf[i];
    printf("tx_buf[%u]: %u\n",i, tx_buf[i]);
    spi_print_registers();
    // wait for spi frame transfer to complete
    printf("Waiting for spi frame transfer to complete\n");
    do {
      printf("spi cs: %08x\n", spi_reg->cs);
      usleep(100);
    } while ((spi_reg->cs & SPI_CS_DONE) == 0);
  }
  // stop spi transfer
  spi_reg->cs &= ~SPI_CS_TA;
  printf("spi transaction successful\n");
  spi_print_registers();
}