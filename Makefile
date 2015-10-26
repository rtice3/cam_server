CXX = g++
INCLUDEDIR = jsoncpp/dist/
LIBS = -lgphoto2 -lgphoto2_port
CXXFLAGS = --std=c++14 -Wall -I$(INCLUDEDIR) $(LIBS)

EXECUTABLE = camera

main: main.o camera.o jsoncpp.o
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) main.o camera.o jsoncpp.o

main.o: main.cpp camera.h
	$(CXX) $(CXXFLAGS) -c main.cpp

camera.o: camera.h

jsoncpp.o: jsoncpp/dist/jsoncpp.cpp jsoncpp/dist/json/json.h
	$(CXX) $(CXXFLAGS) -c jsoncpp/dist/jsoncpp.cpp

clean:
	rm camera *.o
