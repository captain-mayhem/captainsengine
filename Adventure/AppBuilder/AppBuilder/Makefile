SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS = -g `wx-config --cxxflags`
PREFIX = -I/usr/local
INCLUDES = -I./ $(PREFIX)/include/wx-2.8/ \
					 $(PREFIX)/lib/wx/include/x11univ-ansi-release-2.8/

LDFLAGS =
LDLIBS = `wx-config --libs`

DEPFILE = Make.depend
DEPDIR = dep
OBJDIR = obj

TARGET = AppBuilder
DESTDIR = .

.PHONY: all clean distclean

all: depend $(TARGET)
	@echo Done

$(TARGET) : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJDIR)/*.o $(LDLIBS) -o $@

.cpp.o:
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $(OBJDIR)/$@

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

