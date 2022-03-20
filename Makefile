CC=gcc
CFLAGS=-g
LDFLAGS=-lpython3.10
PROJ=output

INCLUDE_DIR=/usr/include/python3.10
LINK_DIR=

all:
	$(CC) -o $(PROJ) ./*.c -I$(INCLUDE_DIR) $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f $(PROJ)

