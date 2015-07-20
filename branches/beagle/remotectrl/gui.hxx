#ifndef GUI_MODULE
#define GUI_MODULE

#include "activeobject.hxx"
#include "datapool.hxx"

#include <string>

#include <gtk/gtk.h>

class GuiModule : public ActiveObject, public DataPoolListener{
public:
	GuiModule();
	virtual ~GuiModule();
	virtual void eventOccurred(DataPoolEntry, DataPoolItem);
protected:
	virtual bool init();
	virtual bool loop_iteration();
	virtual void deinit();

	void setTime(int playtime, int currenttime);

	GtkWidget* m_window;
	GtkWidget* m_debug;

	GtkWidget* m_filename;
	GtkWidget* m_artist;
	GtkWidget* m_title;
	GtkWidget* m_album;
	GtkWidget* m_time;

	int m_playtime;
	int m_currenttime;
};

#endif

