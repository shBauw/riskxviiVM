TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -std=c11 -fno-exceptions -Os -fvisibility=hidden
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) -Os -o $@ $(OBJ) conversions.c errors.c

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $<

run:
	./$(TARGET)

test:
	echo what are we testing?!

clean:
	rm -f *.o *.obj $(TARGET)