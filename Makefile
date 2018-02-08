CC     = gcc
CFLAGS = -Wall
LIBS   = -lX11
INCLUDES = -I "/usr/include"

SRC_FILES = \

mouse_emulator: mouse_emulator.o 
	$(CC) $(CFLAGS) $? $(LDFLAGS) $(LIBS) -o $@

mouse_emulator.o: mouse_emulator.c $(SRC_FILES)
	$(CC) -c $(INCLUDES) mouse_emulator.c

clean:
	rm -f mouse_emulator mouse_emulator.o
