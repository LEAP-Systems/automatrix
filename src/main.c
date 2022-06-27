#include "mmap.h"
#include "spi.h"


int main(void) {
  // configure spi device
  spi_config_t spi_config;
  memset(&spi_config, 0, sizeof(spi_config));
  spi_config.device = 1;
  spi_config.mode = 0x1;
  spi_config.speed = 100000;
  spi_init((const spi_config_t *)&spi_config);
  uint8_t tx_buffer[8];
  spi_transact(tx_buffer);
}
