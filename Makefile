#
# There's ABSOLUTELY NO WARRANTY, express or implied.
#
CC = gcc
CFLAGS = -c -g -O2 -Wall 
LDFLAGS = -L /usr/lib

CFLAGS += -I/usr/include/webkitgtk-1.0 -I/usr/include/libsoup-2.4
CFLAGS += -Wdeclaration-after-statement -fomit-frame-pointer --param allow-store-data-races=0 -Wno-deprecated-declarations -Wformat-extra-args  -Wunused-but-set-variable -Wdate-time

CFLAGS += `pkg-config --cflags gtk+-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 webkit-1.0`

SOURCES = leve.c
OBJECTS = leve.o
EXECUTABLE = leve

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) *~

