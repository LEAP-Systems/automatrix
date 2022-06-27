CC=gcc
CFLAGS += -g -W -Wall -std=c99 -Wall -Wextra -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs

main: mmap.o spi.o

clean:
	rm -f ./*.o ./main
