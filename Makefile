CXX = g++
CXXFLAGS = -std=c++14 -Wall

INCLUDEDIR = jsoncpp/dist/

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
