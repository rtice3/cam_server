CC				:= gcc
CXX 			:= g++
CFLAGS			:= -Wall -O0 -g
CXXFLAGS 		:= --std=c++14 $(CFLAGS) -DDEBUG
INCLUDES 		:= -I../jsoncpp/dist/ -I../mongoose -I/usr/local/include
LFLAGS 			:= 
LIBS 			:= -lgphoto2 -lgphoto2_port

CSOURCES		:= ../mongoose/mongoose.c
CPPSOURCES		:= $(wildcard *.cpp) ../jsoncpp/dist/jsoncpp.cpp
OBJS			:= $(CSOURCES:%.c=%.o) $(CPPSOURCES:%.cpp=%.o)

DEPFILE			:= Makefile.dep

TARGET 			:= camera.bin


.PHONY: all clean

all: $(TARGET)
	@echo $(TARGET) compiled successfully

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

depend $(DEPFILE):
	@echo Makefile - create dependencies for: $(CSOURCES) $(CPPSOURCES)
	@$(RM) $(DEPFILE)
	@$(CC) -E -MM $(CFLAGS) $(INCLUDES) $(CSOURCES) >> $(DEPFILE)
	@$(CXX) -E -MM $(CXXFLAGS) $(INCLUDES) $(CPPSOURCES) >> $(DEPFILE)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPFILE)
endif

clean: 
	$(RM) -f $(TARGET) $(DEPFILE) $(OBJS)