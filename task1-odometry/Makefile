default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := Task_1_Odometry_Pi 
all : $(BINARIES)

LIBS := -lrt -lncurses #-lach 

Task_1_Odometry_Pi: Task_1_Odometry_Pi.o
	gcc -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) *.o
