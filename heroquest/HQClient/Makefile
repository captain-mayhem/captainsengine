# File: Makefile

SHELL :=/bin/sh

# set OS variable
OS := $(shell uname -s)
CXX      := g++
CXXFLAGS := -g -Wall -I/usr/X11/include -I../../Engine -D_CLIENT_
LDFLAGS  = 
GL_LIBS  = -lGL -lGLU
ENGINE_LIBS = ../../Engine/libengine.a
X11_LIBS = /usr/X11R6/lib/libX11.a /usr/X11R6/lib/libXext.a /usr/X11R6/lib/libXxf86vm.a
SYS_LIBS = -L../../Engine ../../Engine/lib/liblua.a ../../Engine/lib/liblualib.a ../../Engine/lib/libjpeg.a -ldl -lpthread -lrt
DOXYGEN  = doxygen
DEP_FLAGS= -MM

# ----- Define the Compiler Environment -----
CXXFLAGS += -c -DUNIX
LIBS = $(GL_LIBS) $(ENGINE_LIBS) $(X11_LIBS) $(SYS_LIBS)

# The name of the application and binary
PROGRAM = HQClient

# Fetch all source files
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

OBJ = $(OBJECTS) #$(OBJECT2) 
all: $(PROGRAM)

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

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %cpp,%o,$<)

$(PROGRAM): $(OBJ)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJ) $(LIBS)

Makedepend:
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(SOURCES) > Makedepend

include Makedepend
