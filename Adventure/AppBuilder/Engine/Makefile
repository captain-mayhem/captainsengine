CXX := g++
CC := gcc

SRCS := $(wildcard *.cpp *.c)
OBJS = $(SRCS:.cpp=.o) $(SRCS:.c=.o)

ANTLRSRCS := $(wildcard antlr/src/*.c)
ANTLROBJS = $(ANTLRSRCS:.c=.o)

CXXFLAGS = -g `wx-config --cxxflags`
PREFIX = -I/usr/local
INCLUDES = -I./ $(PREFIX)/include/wx-2.8/ \
					 $(PREFIX)/lib/wx/include/gtk2-ansi-release-2.8/\
					 -I./antlr/include

LDFLAGS =
LDLIBS = `wx-config --libs` -lGL -lwx_gtk2_gl-2.8

DEPFILE = Make.depend
DEPDIR = dep
OBJDIR = .

TARGET = AppBuilder
DESTDIR = .

.PHONY: all clean distclean

all: $(TARGET)
	@echo Done

$(TARGET) : $(OBJS) $(ANTLROBJS)
	$(CXX) $(LDFLAGS) $(OBJDIR)/*.o antlr/src/*.o $(LDLIBS) -o $@

pcdkLexer.h pcdkParser.h pdckParser.c pcdkLexer.c: pcdk.g
	java -cp antlrworks-1.2.3.jar org.antlr.Tool pcdk.g

.cpp.o:
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $(OBJDIR)/$@

antlr/src/%.o: antlr/src/%.c
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $(OBJDIR)/$@ 

.c.o:
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -x c++ $(INCLUDES) -c $< -o $(OBJDIR)/$@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) -r $(DEPDIR)
	$(RM) $(TARGET)

depend $(DEPFILE):
	$(RM) $(DEPFILE)
	$(CXX) $(CXXFLAGS) -E -MM $(INCLUDES) $(SRCS) >> $(DEPFILE)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEPFILE)
endif

