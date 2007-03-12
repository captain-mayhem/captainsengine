# File: Makefile

SHELL :=/bin/sh

# set OS variable
OS := $(shell uname -s)
CXX      := g++
CXXFLAGS := -g -Wall -I/usr/X11/include -I../../Engine
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
PROGRAM = HQServer

# Fetch all source files
SOURCE2 = bitset.cpp \
					gameobject.cpp \
					field.cpp \
					door.cpp \
					socket.cpp \
					script.cpp \
					creature.cpp \
					item.cpp \
					inventory.cpp \
					world.cpp \
					hero.cpp \
					monster.cpp \
					overlay.cpp \
					furniture.cpp \
					templates.cpp

S2PATH = $(patsubst %.cpp, ../HQClient/%.cpp,$(SOURCE2))
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
OBJECT2 = $(patsubst %.cpp,%.o,$(SOURCE2))

OBJ = $(OBJECTS) $(OBJECT2)
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

socket.o: ../HQClient/socket.cpp
	$(CXX) $(CXXFLAGS) -c -o socket.o ../HQClient/socket.cpp
bitset.o: ../HQClient/bitset.cpp
	$(CXX) $(CXXFLAGS) -c -o bitset.o ../HQClient/bitset.cpp
gameobject.o: ../HQClient/gameobject.cpp
	$(CXX) $(CXXFLAGS) -c -o gameobject.o ../HQClient/gameobject.cpp
field.o: ../HQClient/field.cpp
	$(CXX) $(CXXFLAGS) -c -o field.o ../HQClient/field.cpp
door.o: ../HQClient/door.cpp
	$(CXX) $(CXXFLAGS) -c -o door.o ../HQClient/door.cpp
script.o: ../HQClient/script.cpp
	$(CXX) $(CXXFLAGS) -c -o script.o ../HQClient/script.cpp
creature.o: ../HQClient/creature.cpp
	$(CXX) $(CXXFLAGS) -c -o creature.o ../HQClient/creature.cpp
item.o: ../HQClient/item.cpp
	$(CXX) $(CXXFLAGS) -c -o item.o ../HQClient/item.cpp
inventory.o: ../HQClient/inventory.cpp
	$(CXX) $(CXXFLAGS) -c -o inventory.o ../HQClient/inventory.cpp
world.o: ../HQClient/world.cpp
	$(CXX) $(CXXFLAGS) -c -o world.o ../HQClient/world.cpp
hero.o: ../HQClient/hero.cpp
	$(CXX) $(CXXFLAGS) -c -o hero.o ../HQClient/hero.cpp
monster.o: ../HQClient/monster.cpp
	$(CXX) $(CXXFLAGS) -c -o monster.o ../HQClient/monster.cpp
overlay.o: ../HQClient/overlay.cpp
	$(CXX) $(CXXFLAGS) -c -o overlay.o ../HQClient/overlay.cpp
furniture.o: ../HQClient/furniture.cpp
	$(CXX) $(CXXFLAGS) -c -o furniture.o ../HQClient/furniture.cpp
templates.o: ../HQClient/templates.cpp
	$(CXX) $(CXXFLAGS) -c -o templates.o ../HQClient/templates.cpp


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %cpp,%o,$<)

$(PROGRAM): $(OBJ)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJ) $(LIBS)

Makedepend:
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(SOURCES) $(S2PATH) > Makedepend

include Makedepend
