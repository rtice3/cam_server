CXX 			:= g++
CXXFLAGS 		:= --std=c++14 -Wall -O0 -g
INCLUDES 		:= -I../jsoncpp/dist/ -I../mongoose -I/usr/local/include
LFLAGS 			:= 
LIBS 			:= -lgphoto2 -lgphoto2_port

SOURCES			:= $(wildcard *.cpp) ../mongoose/mongoose.c ../jsoncpp/dist/jsoncpp.cpp
OBJS 			:= $(SOURCES:.cpp=.o)
DEPFILE			:= Makefile.dep

TARGET 			:= camera.bin


.PHONY: all clean

all: $(TARGET)
	@echo $(TARGET) compiled successfully

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

depend $(DEPFILE):
	@echo Makefile - create dependencies for: $(SOURCES)
	@$(RM) $(DEPFILE)
	@$(CXX) -E -MM $(CXXFLAGS) $(INCLUDES) $(SOURCES) >> $(DEPFILE)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPFILE)
endif

clean: 
	$(RM) -f $(TARGET) $(DEPFILE) $(OBJS)