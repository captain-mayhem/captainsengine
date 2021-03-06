# File: Makefile

SHELL :=/bin/sh

# set OS variable
OS := $(shell uname -s)
CXX      := g++
CXXFLAGS := -g -Wall -I/usr/X11/include -I../../Engine
LDFLAGS  = 
ENGINE_LIBS = ../../Engine/libengine.a
GL_LIBS  = -lGL -lGLU
X11_LIBS = /usr/X11R6/lib/libX11.a /usr/X11R6/lib/libXext.a /usr/X11R6/lib/libXxf86vm.a
SYS_LIBS = -L../../Engine ../../Engine/lib/liblua.a ../../Engine/lib/liblualib.a ../../Engine/lib/libjpeg.a -ldl -lpthread -lrt
DOXYGEN  = doxygen
DEP_FLAGS= -MM

# ----- Define the Compiler Environment -----
CXXFLAGS += -c -DUNIX
LIBS = $(GL_LIBS) $(ENGINE_LIBS) $(X11_LIBS) $(SYS_LIBS)

# The name of the application and binary
PROGRAM = HQEdit

# Fetch all source files
SOURCE2 = bitset.cpp \
					gameobject.cpp \
					field.cpp \
					door.cpp
S2PATH = $(patsubst %.cpp, ../HQClient/%.cpp,$(SOURCE2))
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
OBJECT2 = $(patsubst %.cpp,%.o,$(SOURCE2))

OBJ = $(OBJECTS) $(OBJECT2)
all: $(PROGRAM)

levels:
	HQEdit basic/maze.map
	HQEdit basic/trial.map

doc: #doxygen.conf $(wildcard *.hh)
	$(DOXYGEN) doxygen.conf

clean:
	rm -f $(PROGRAM)
	rm -f *.o
	rm -rf *~ ii_files
	rm -f Makedepend
	rm -rf doc

allclean: clean
	rm -f *~

bitset.o: ../HQClient/bitset.cpp
	$(CXX) $(CXXFLAGS) -c -o bitset.o ../HQClient/bitset.cpp
gameobject.o: ../HQClient/gameobject.cpp
	$(CXX) $(CXXFLAGS) -c -o gameobject.o ../HQClient/gameobject.cpp
field.o: ../HQClient/field.cpp
	$(CXX) $(CXXFLAGS) -c -o field.o ../HQClient/field.cpp
door.o: ../HQClient/door.cpp
	$(CXX) $(CXXFLAGS) -c -o door.o ../HQClient/door.cpp


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %cpp,%o,$<)

$(PROGRAM): $(OBJ)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJ) $(LIBS)

Makedepend:
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(SOURCES) $(S2PATH) > Makedepend

include Makedepend
