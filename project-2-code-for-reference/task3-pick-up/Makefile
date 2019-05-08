default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := Task_3_Pick_Up_Pi 
all : $(BINARIES)

LIBS := -lrt -lncurses #-lach 

Task_3_Pick_Up_Pi: Task_3_Pick_Up_Pi.o
	gcc -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) *.o
