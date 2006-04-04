# *************** Graphics Programming and Applications (GraPA) ***************
# Assignment 5 - "Hero Quest"
#
# File: Makefile

SHELL :=/bin/sh

# set OS variable
OS := $(shell uname -s)
ifeq "$(OS)" "IRIX64"
OS := IRIX
endif

# choose compiler flags
# --- IRIX ---
ifeq "$(OS)" "IRIX"
  CXX := CC
  ifeq "$(IMMD9_ENV)" ""
    # --- CIP pool (faui05x) ---
    CXXFLAGS := -g -o32 -Dtrue=1 -Dfalse=0 -LANG:std
    LDFLAGS  = -o32 -LANG:std
    QTDIR    = /proj/i9grapa/qt.IRIX
    DOXYGEN  = /proj/i9grapa/bin.IRIX/doxygen
    # Hack for CC
    LM_LICENSE_FILE = 1700@symphony
    export LM_LICENSE_FILE
  else
    # --- IMMD IX ---
    CXXFLAGS := -g -n32 -MDupdate Makedepend -LANG:std
    LDFLAGS  = -n32 -LANG:std
    QTDIR    = /share/pckg/qt23
    DOXYGEN  = doxygen
    # Hack for doxygen
    LD_LIBRARYN32_PATH = /share/pckg/qt-egcs/lib:/share/pckg/egcs/lib
    export LD_LIBRARYN32_PATH
  endif
  QT_LIBS  = -lqt 
  GL_LIBS  = -lGL -lGLU
  X11_LIBS = -lXmu -lXext -lX11
  SYS_LIBS = -lm -ljpeg
  DEP_FLAGS=
endif
# --- Linux ---
ifeq "$(OS)" "Linux"
  # Modify this for your environment!
  CXX      := g++
  CXXFLAGS := -g -Wall -I/usr/X11/include `sdl-config --cflags`
  LDFLAGS  = 
  # Wherever you have installed your Qt
  QTDIR    = /usr
  QT_INCL  = /usr/include/qt3
  # This is the place to look at in the CIP-pool
  # QTDIR    = /proj/i9lib
  # QT_LIBS  = -L/usr/lib 
  QT_LIBS  = `sdl-config --libs` 
  GL_LIBS  = -lGL -lGLU
  X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11
  SYS_LIBS = libjpeg.a ../liblua.a ../liblualib.a
  DOXYGEN  = doxygen
  DEP_FLAGS= -MM
endif
# --- CIP Sun (faui02x) ---
ifeq "$(OS)" "SunOS"
  CXX      := g++
  CXXFLAGS := -g -Wall
  LDFLAGS  =
  QTDIR    = /local/KDE/
  QT_LIBS  = -lqgl -lqt
  GL_LIBS  = -L/usr/openwin/lib -ldga -lGL -lGLU
  X11_LIBS =  -lXmu -lXext -lX11
  LD_LIBRARY_PATH = /proj/i9grapa/lib.SunOS:/usr/openwin/lib
  export LD_LIBRARY_PATH
  SYS_LIBS = -ljpeg
  DOXYGEN  = /proj/i9grapa/bin.SunOS/doxygen
  DEP_FLAGS=
endif                                                                           

# ----- Define the Compiler Environment -----
CXXFLAGS += -c -I$(QT_INCL) -D_CLIENT_
LIBS = $(QT_LIBS) $(GL_LIBS) $(X11_LIBS) $(SYS_LIBS)

# The name of the application and binary
PROGRAM = HeroQuest-$(OS)

# Fetch all source files
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
MOCSRCS = $(patsubst %.qt.cpp,%.qt_moc.cpp,$(filter %.qt.cpp,$(SOURCES)))
MOCOBJS = $(patsubst %.cpp,%.o,$(MOCSRCS))

OBJ = $(OBJECTS) $(MOCOBJS)

all: $(PROGRAM)

doc: #doxygen.conf $(wildcard *.H)
	$(DOXYGEN) doxygen.conf

clean:
	rm -f $(PROGRAM)
	rm -f *.o
	rm -f *.qt_moc.*
	rm -rf *~ ii_files
	rm -f Makedepend
	rm -rf doc

allclean: clean
	rm -f *~

%.qt_moc.cpp: %.qt.hh
	$(QTDIR)/bin/moc $< > $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

$(PROGRAM): $(OBJ)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJ) $(LIBS)

Makedepend:
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) $(SOURCES) > Makedepend

include Makedepend
