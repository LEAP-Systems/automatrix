
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "mmap.h"

// NOTE: size_t on BCM2711 is an unsigned int
void *map_mem(uint32_t base, size_t size) {
  int mem_fd;
  printf("Requested base=0x%08x, Requested size=%u\n", base, size);
  // ensure the base memory is inline with the page size
  const uint32_t offset = base % PAGE_SIZE;
  base = base - offset;
  size = size + offset;
  if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
    perror("Failed to open /dev/mem");
    return NULL;
  }
  printf("base=0x%08x, offset=%i, size=%u\n", base, offset, size);
  // allow read and writes on the content
  void *mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, base);
  printf("base=0x%08x, mem=%p\n", base, mem);
  close(mem_fd);
  if (mem == MAP_FAILED) {
    perror("mmap error");
    exit(-1);
  }
  return (uint8_t *)mem + offset;
}

void unmap_mem(void *addr, size_t size) {
  const intptr_t offset = (intptr_t)addr % PAGE_SIZE;
  addr = (uint8_t *)addr - offset;
  size = size + offset;
  if (munmap(addr, size) != 0) {
    perror("munmap error: ");
    exit(-1);
  }
}
