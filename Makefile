CC=cc

BUILD_CONFIG = -g -DDEBUG

CFLAGS= $(BUILD_CONFIG) -fPIC -c -Wall -Wextra 
CFLAGS+=-std=gnu99 -pedantic -D_XOPEN_SOURCE=700 -D _BSD_SOURCE -D_REENTRANT


CFLAGS+= -I/usr/local/include/GroundBase -I/usr/local/include/VisionKit/

LDFLAGS=  -lm -L/usr/local/lib/   -lGroundBase -lopencv_core -lopencv_highgui


SOURCES= src/client.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE= Client

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(EXECUTABLE)

re: clean all
