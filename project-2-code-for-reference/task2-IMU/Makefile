default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := Task_2_IMU_Pi 
all : $(BINARIES)

LIBS := -lrt -lncurses #-lach 

Task_2_IMU_Pi: Task_2_IMU_Pi.o
	gcc -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) *.o
