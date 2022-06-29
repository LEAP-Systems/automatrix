#include <stdio.h>
#include <string.h>

#include "mmap.h"
#include "spi.h"


int main(void) {
  // configure spi device
  spi_config_t spi_config;
  memset(&spi_config, 0, sizeof(spi_config));
  spi_config.device = 0;
  spi_config.mode = 0x1;
  spi_config.speed = 1000;
  printf("Size of spi_config: %i\n", sizeof(spi_config));
  spi_init((const spi_config_t *)&spi_config);
  const uint8_t tx_buffer[5] = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };
  spi_transact((const uint8_t *)&tx_buffer, sizeof(tx_buffer));
}
