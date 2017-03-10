CC=cc

BUILD_CONFIG = -g -DDEBUG

CFLAGS= $(BUILD_CONFIG) -fPIC -c -Wall -Wextra 
CFLAGS+=-std=gnu99 -pedantic -D_XOPEN_SOURCE=700 -D _BSD_SOURCE -D_REENTRANT


CFLAGS+= -I/usr/local/include/GroundBase -I/usr/local/include/VisionKit/

LDFLAGS=  -lm -L/usr/local/lib/   -lGroundBase -lopencv_core -lopencv_highgui

SOURCES_SERVER= src/server.c
SOURCES_CLIENT= src/client.c

OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)
EXECUTABLE_CLIENT= Client

OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
EXECUTABLE_SERVER= Server

all: client server

client: $(SOURCES_CLIENT) $(EXECUTABLE_CLIENT)

server: $(SOURCES_SERVER) $(EXECUTABLE_SERVER)
    
$(EXECUTABLE_CLIENT): $(OBJECTS_CLIENT) 
	$(CC) $(OBJECTS_CLIENT) -o $@ $(LDFLAGS) 

$(EXECUTABLE_SERVER): $(OBJECTS_SERVER)
	$(CC) $(OBJECTS_SERVER) -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS_CLIENT)

fclean: clean
	rm -f $(EXECUTABLE_CLIENT)

re: clean all
