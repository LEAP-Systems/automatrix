/**
 * @file spi.h
 * @author ztnel (christian@leapsystems.online)
 * @brief BCM2711 spi definitions. All references are made from the BCM2711 datasheet:
 * https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf
 * @version 0.1
 * @date 2022-06-26
 * 
 * @copyright Copyright Leap Systems (c) 2022
 * 
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include "mmap.h"

/**
 * @brief 9.5. SPI Register Map
 * Note SPI1 and SPI2 are mini spi interfaces with reduced capability. See 2.3 Universal SPI Master
 */
#define SPI0_OFFSET             0x204000
#define SPI_PHYS_BASE           BCM2711_PERI_BASE + SPI0_OFFSET
#define SPI_DEV_PHYS_BASE(x)    ((x)*0x200 + SPI_PHYS_BASE)
#define SPI_REGISTER_LEN        0x18

/**
 * @brief Table 160. CS Register
 */
#define SPI_CS_POL2(x)          ((x) << 23)
#define SPI_CS_POL1(x)          ((x) << 22)
#define SPI_CS_POL0(x)          ((x) << 21)
#define SPI_CS_RXF              (1 << 20)   // RX FIFO full
#define SPI_CS_RXR              (1 << 19)   // RX FIFO 3/4 full. Cleared by reading sufficient data from RX FIFO or setting TA to 0 
#define SPI_CS_TXD              (1 << 18)   // TX FIFO has space for at least 1 byte
#define SPI_CS_RXD              (1 << 17)   // RX FIFO contains data
#define SPI_CS_DONE             (1 << 16)   // Transfer complete
#define SPI_CS_LEN(x)           ((x) << 13) // LoSSI enable
#define SPI_CS_ADCS(x)          ((x) << 11) // Automatically de-assert chip select at end of DMA transfer (determined by SPIDLEN) 
#define SPI_CS_INTR(x)          ((x) << 10) // Generate interrupt while RXR = 1
#define SPI_CS_INTD(x)          ((x) << 9)  // Generate interrupt when DONE = 1
#define SPI_CS_DMAEN(x)         ((x) << 8)  // DMA Enable: Peripheral generates data requests taken in four-byte words until the SPIDLEN has been reached.
/**
 * @brief Transfer Active
 * 
 * 0 = Transfer not active. /CS lines are all high (assuming CSPOL = 0).
 * RXR and DONE are 0. Writes to SPI_FIFO write data into bits 15:0 of SPIDLEN 
 * and bits 7:0 of SPICS allowing DMA data blocks to set mode before sending data.
 * 
 * 1 = Transfer active. /CS lines are set according to CS bits and CSPOL. 
 * Writes to SPI_FIFO write data to TX FIFO. TA is cleared by a dma_frame_end 
 * pulse from the DMA controller.
 */
#define SPI_CS_TA               (1 << 7)
#define SPI_CS_CSPOL(x)         ((x) << 6)  // Chip select polarity: 0 - Active Low | 1 - Active High
/**
 * @brief FIFO Clear [2 bits]
 * 
 * 0 0 = No action.
 * x 1 = Clear TX FIFO. One-shot operation.
 * 1 x = Clear RX FIFO. One-shot operation.
 * If CLEAR and TA are both set in the same operation, the FIFOs are cleared before the new frame is started. Read back as 0.
 */
#define SPI_CS_CLEAR(x)        ((x) << 4)
#define SPI_CS_CPOL(x)         ((x) << 3)  // clock polarity: 0 - Rest state of clock low | 1 - Rest state of clock high
/**
 * @brief Clock Phase
 * 
 * 0 = First SCLK transition at middle of data bit.
 * 1 = First SCLK transition at beginning of data bit.
 */
#define SPI_CS_CPHA(x)         ((x) << 2)
/**
 * @brief Chip Select [2 bits]
 * 
 * 0 0 = Chip select 0
 * 0 1 = Chip select 1
 * 1 0 = Chip select 3
 * 1 1 = Reserved
 */
#define SPI_CS_CS(x)           ((x) << 0)

/**
 * @brief Clock Divider Register [16 bits]
 * Table 162. CLK Register
 *
 * SCLK = Core Clock / CDIV. If CDIV is set to 0, the divisor is 65536. 
 * The divisor must be a multiple of 2. Odd numbers rounded down. The maximum 
 * SPI clock rate is of the APB clock.
 */
#define SPI_CLK_CDIV(x)        ((x) << 0)

/**
 * @brief Data Length Register [16 bits]
 * Table 163. DLEN Register 
 * 
 * The number of bytes to transfer. This field is only valid for DMA mode 
 * (DMAEN set) and controls how many bytes to transmit (and therefore receive).
 */
#define SPI_DLEN_LEN(x)        ((x) << 0)

/**
 * @brief DMA Control Register [32 bits]
 * Table 165. DC Register
 * 
 * This register controls the generation of the DREQ and Panic signals to an 
 * external DMA engine. The DREQ signals are generated when the FIFOs reach 
 * their defined levels and need servicing. The Panic signals instruct the 
 * external DMA engine to raise the priority of its AXI requests.
 */

/**
 * @brief DMA Read Panic Threshold [8 bits]
 * Generate the Panic signal to the RX DMA engine whenever the RX FIFO level is 
 * greater than this amount. Resets to 0x30
 */
#define SPI_DC_RPANIC(x)       ((x) << 24)

/**
 * @brief DMA Read Request Threshold [8 bits]
 * Generate a DREQ to the RX DMA engine whenever the RX FIFO level is greater 
 * than this amount (RX DREQ is also generated if the transfer has finished but
 * the RX FIFO isn’t empty). Resets to 0x20
 */
#define SPI_DC_RDREQ(x)        ((x) << 16)

/**
 * @brief DMA Write Panic Threshold [8 bits]
 * Generate the Panic signal to the TX DMA engine whenever the TX FIFO level is 
 * greater than this amount. Resets to 0x10
 */
#define SPI_DC_TPANIC(x)       ((x) << 8)

/**
 * @brief DMA Write Request Threshold [8 bits]
 * Generate a DREQ signal to the TX DMA engine whenever the TX FIFO level is 
 * less than or equal to this amount. Resets to 0x20
 */
#define SPI_DC_TDREQ(x)        ((x) << 0)

/**
 * @brief SPI Device Register struct
 * Table 159. SPI Register Map
 */
typedef struct spi_t {
  uint32_t cs;          // master control and status
  uint32_t fifo;        // master tx and rx fifos
  uint32_t clk;         // master clock divider
  uint32_t dlen;        // master data length
  uint32_t ltoh;        // LoSSI mode TOH
  uint32_t dc;          // dma dreq controls
} spi_t;

typedef struct spi_config_t {
  uint8_t device;       // spi device number
  uint8_t mode;         // polled = 0x1, interrupt = 0x2, dma = 0x3
  uint64_t speed;       // spi clock speed
} spi_config_t;

void spi_init(const spi_config_t *spi_config);
void spi_transact(const uint8_t *tx_buf, size_t buf_size);

#endif // SPI_H_