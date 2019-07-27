ICC		= gcc
CFLAGS		= -c -Wall -I . -std=gnu99 -lcurl
LDFLAGS		= -lSDLmain -lSDL -lcurl
SOURCES		= wz.c camera.c util.c viewer.c image.c
OBJECTS		= $(SOURCES:.c=.o)
EXECUTABLE1	= wz
EXECUTABLE2	= usbreset

all: 		$(SOURCES) $(EXECUTABLE1) $(EXECUTABLE2)
clean :
		rm -f *.o $(EXECUTABLE1) $(EXECUTABLE2)
	
$(EXECUTABLE1):	$(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(EXECUTABLE2):	usbreset.o 
		$(CC) $(EXECUTABLE2).c -o $@

.c.o:
		$(CC) $(CFLAGS) $< -o $@
