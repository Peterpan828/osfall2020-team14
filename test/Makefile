CC = arm-linux-gnueabi-gcc
CFLAGS = -I ../include
SRC = load_balance.c trial.c
OBJ = $(SRC:.c=.o)
TARGETS = $(SRC:.c=)

.SUFFIXES : .c .o

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o
