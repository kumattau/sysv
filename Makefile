CC	= gcc
CFLAGS	= -std=c99 -D_GNU_SOURCE -Wall -W -O3
SRC	= $(shell ls *.c)
BIN	= sysv
LIBS	= -lrt

all:
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LIBS)

clean:
	rm -f $(BIN)
