CXX 		= g++
CXXFLAGS 	= --std=c++14 -Wall -g -O0 -DDEBUG
INCLUDES 	= -Ijsoncpp/dist/ -I/usr/local/include
LFLAGS 		=
LIBS 		= -lgphoto2 -lgphoto2_port

SOURCES		= main.cpp camera.cpp jsoncpp/dist/jsoncpp.cpp
OBJS 		= $(SOURCES:.cpp=.o)
EXECUTABLE 	= camera

.PHONY: depend clean

all: $(EXECUTABLE)
	@echo    $(EXECUTABLE) compiled successfully

$(EXECUTABLE): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(EXECUTABLE)

depend: $(SOURCES)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it