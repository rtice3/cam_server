CC=g++

CFLAGS=-std=c++14 -Wall
LDFLAGS=
INCLUDEDIR=jsoncpp/dist


SOURCES=main.cpp camera.cpp jsoncpp/dist/jsoncpp.cpp
OBJECTS=$(SOURCES:.cpp=.o)


EXECUTABLE=camera


all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
    $(CC) $(CFLAGS) -I$(INCLUDEDIR) $< -o $@