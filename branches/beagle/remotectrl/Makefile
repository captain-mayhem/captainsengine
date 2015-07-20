SYSROOT=/home/captain/oe-setup/angstrom-setup-scripts/build/tmp-angstrom_2008_1/sysroots/armv7a-angstrom-linux-gnueabi

INCLUDES=\
	 -I$(SYSROOT)/usr/include/gtk-2.0 \
	 -I$(SYSROOT)/usr/include/glib-2.0 \
	 -I$(SYSROOT)/usr/lib/glib-2.0/include \
	 -I$(SYSROOT)/usr/lib/gtk-2.0/include \
	 -I$(SYSROOT)/usr/include/cairo\
	 -I$(SYSROOT)/usr/include/pango-1.0\
	 -I$(SYSROOT)/usr/include/atk-1.0

SOURCES=main.cxx \
	activeobject.cxx \
	datapoolitem.cpp \
	datapool.cpp \
	gui.cxx \
	mpdclient.cxx

LIBS=-lpthread -lgtk-x11-2.0

all:
	arm-angstrom-linux-gnueabi-g++ $(INCLUDES) -o remotectrl $(SOURCES) $(LIBS)
