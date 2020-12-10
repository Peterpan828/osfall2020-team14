CC = arm-linux-gnueabi-gcc
CFLAGS = -I ../include
SRC = gpsupdate.c file_loc.c writetest.c
OBJ = $(SRC:.c=)
TARGETS = $(SRC:.c=)

.SUFFIXES : .c .o

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o
