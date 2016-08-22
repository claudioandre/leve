#
# There's ABSOLUTELY NO WARRANTY, express or implied.
#
CC = gcc
LN = ln -s
CP = cp
MKDIR = /bin/mkdir -p

CFLAGS = -c -g -O2 -Wall 
LDFLAGS = -L /usr/lib

CFLAGS += -I/usr/include/webkitgtk-1.0 -I/usr/include/libsoup-2.4
CFLAGS += -Wdeclaration-after-statement -fomit-frame-pointer --param allow-store-data-races=0 -Wno-deprecated-declarations -Wformat-extra-args  -Wunused-but-set-variable -Wdate-time

CFLAGS += `pkg-config --cflags gtk+-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 webkit-1.0`

SOURCES = leve.c
OBJECTS = leve.o
EXECUTABLE = leve
EXTRA = leve.png

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm -f links
	$(LN) $(EXECUTABLE) links

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) *~ links

install:
	$(MKDIR) $(DESTDIR)/bin
	$(CP) $(EXECUTABLE) $(DESTDIR)/bin
	$(CP) $(EXTRA) $(DESTDIR)/bin
	$(CP) links $(DESTDIR)/bin

