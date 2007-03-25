# File: Makefile

SHELL :=/bin/sh

# set OS variable
OS := $(shell uname -s)
CXX      := g++
CXXFLAGS := -g -Wall -I/usr/X11/include
LDFLAGS  = 
GL_LIBS  = -lGL -lGLU #/usr/lib/libGL.a -lGLU
X11_LIBS = /usr/X11R6/lib/libX11.a /usr/X11R6/lib/libXext.a /usr/X11R6/lib/libXxf86vm.a
SYS_LIBS = lib/liblua.a lib/liblualib.a lib/libjpeg.a lib/libcal3d.a -ldl -lpthread -lrt
DOXYGEN  = doxygen
DEP_FLAGS= -MM

# ----- Define the Compiler Environment -----
CXXFLAGS += -c -DUNIX
LIBS = $(GL_LIBS) $(X11_LIBS) $(SYS_LIBS)

# The name of the application and binary
PROGRAM = Engine-$(OS)

DIRS = gui \
			 input \
			 math \
			 mesh \
			 renderer \
			 system \
			 window

# Fetch all source files
SOURCE2 = #socket.cpp 
#S2PATH = $(patsubst %.cpp, ../heroquest/%.cpp,$(SOURCE2))
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
#OBJECT2 = $(patsubst %.cpp,%.o,$(SOURCE2))
TEMP = $(wildcard gui/*.cpp) \
	$(wildcard input/*.cpp) \
	$(wildcard math/*.cpp) \
	$(wildcard mesh/*.cpp) \
	$(wildcard renderer/*.cpp) \
	$(wildcard system/*.cpp) \
	$(wildcard window/*.cpp)
OBJECT2 = $(patsubst %.cpp,%.o,$(TEMP))


OBJ = $(OBJECTS) $(OBJECT2) 
all: $(PROGRAM)

doc: #doxygen.conf $(wildcard *.hh)
	$(DOXYGEN) doxygen.conf

clean:
	rm -f $(PROGRAM)
	rm -f *.o
	rm -rf *~ ii_files
	rm -f gui/*.o gui/*~
	rm -f input/*.o input/*~
	rm -f math/*.o math/*~
	rm -f mesh/*.o mesh/*~
	rm -f renderer/*.o renderer/*~
	rm -f system/*.o system/*~
	rm -f window/*.o window/*~
	rm -f Makedepend
	rm -rf doc

allclean: clean
	rm -f *~

#socket.o: ../heroquest/socket.cpp
#	$(CXX) $(CXXFLAGS) -c -o socket.o ../heroquest/socket.cpp

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %cpp,%o,$<)

$(PROGRAM): $(OBJ)
#	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJ) $(LIBS)
	ar -cvq libengine.a $(OBJ)
	ranlib libengine.a

Makedepend:
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(SOURCES) $(S2PATH) > Makedepend

include Makedepend
