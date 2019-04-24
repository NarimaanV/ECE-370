default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := udp-sender 
all : $(BINARIES)

LIBS := -lrt #-lach 

udp-sender: src/udp-sender.o
	gcc -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) src/*.o
