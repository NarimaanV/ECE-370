default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := Project_3_Pi 
all : $(BINARIES)

LIBS := -lrt -lncurses #-lach 

Project_3_Pi: Project_3_Pi.o
	gcc -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) *.o
