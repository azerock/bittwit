CC=gcc

CFLAGS+= -Wall -g3 -O0
LDFLAGS+=

GTK_CFLAGS+= `pkg-config --cflags gtk+-2.0`
GTK_LDFLAGS+= `pkg-config --libs gtk+-2.0`

OBJ_BASE= serial.o
OBJ_GTK= main.o gui.o

# Header dependencies
main.o gui.o: gui.h
main.o serial.o: serial.h

.PHONY: all clean

all: bittwit

clean:
	rm -f *.o *~

bittwit: $(OBJ_BASE) $(OBJ_GTK)
	$(CC) -o $@ $^ $(LDFLAGS) $(GTK_LDFLAGS) $(LIBS)

.SECONDEXPANSION:
$(OBJ_GTK): $$(patsubst %.o,%.c,$$@)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c -o $@ $(@:%.o=%.c)
