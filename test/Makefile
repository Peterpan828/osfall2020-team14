CC = arm-linux-gnueabi-gcc
CFLAGS = -I ../include
SRC = rotd.c selector.c trial.c
OBJ = $(SRC:.c=)
TARGETS = $(SRC:.c=)

.SUFFIXES : .c .o

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o
