#include "gui.hxx"

#include <cstdio>
#include <cstdlib>

#include <unistd.h>

extern DataPool thePool;

GuiModule::GuiModule(){
	setenv("DISPLAY", ":0.0", 0);
}

GuiModule::~GuiModule(){
}

bool GuiModule::init(){
	printf("INIT\n");
	thePool.registerOnChange(DP_DEBUG, this);
	thePool.registerOnChange(DP_CURRENT_METADATA, this);
	thePool.registerOnChange(DP_CURRENT_POSITION, this);

	int argc = 1;
	char* arg1 = (char*)"remotectrl";
	char** argv = new char*[1];
	argv[0] = arg1;
	gtk_init(&argc, &argv);
	delete argv;

	m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_window), "Captain's Media Center");
	gtk_window_set_default_size(GTK_WINDOW(m_window), 800, 600);
	//gtk_window_set_default_size(GTK_WINDOW(m_window), 1190, 680);
	gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(m_window), 10);
	
	GtkWidget* col1 = gtk_vbox_new(TRUE, 1);
	gtk_container_add(GTK_CONTAINER(m_window), col1);
	
	PangoFontDescription* font = pango_font_description_new();
	pango_font_description_set_family_static(font, "DejaVuSans");
	pango_font_description_set_size(font, 10*PANGO_SCALE);

	std::string debugmsg;
	thePool.get(DP_DEBUG).read(debugmsg);
	m_debug = gtk_label_new(debugmsg.c_str());
	gtk_widget_modify_font(m_debug, font);

	pango_font_description_set_size(font, 16*PANGO_SCALE);
	
	std::map<std::string, std::string> metadata;
	thePool.get(DP_CURRENT_METADATA).read(metadata);
	m_filename = gtk_label_new(metadata["file"].c_str());
	gtk_widget_modify_font(m_filename, font);
	m_artist = gtk_label_new(metadata["Artist"].c_str());
	gtk_widget_modify_font(m_artist, font);
	m_title = gtk_label_new(metadata["Title"].c_str());
	gtk_widget_modify_font(m_title, font);
	m_album = gtk_label_new(metadata["Album"].c_str());
	gtk_widget_modify_font(m_album, font);
	m_time = gtk_label_new("");
	gtk_widget_modify_font(m_time, font);
	int currtime; thePool.get(DP_CURRENT_POSITION).read(currtime);
	setTime(atoi(metadata["Time"].c_str()), currtime);

	pango_font_description_free(font);

	gtk_box_pack_start(GTK_BOX(col1), m_filename, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(col1), m_artist, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(col1), m_title, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(col1), m_album, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(col1), m_time, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(col1), m_debug, TRUE, TRUE, 0);
	
	gtk_widget_show_all(m_window);

	//g_signal_connect_swapped(G_OBJECT(m_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	return true;
}

bool GuiModule::loop_iteration(){
	//gtk_main();
	processEvents();
	while(gtk_events_pending()){
		gtk_main_iteration();
	}
	sleep(50);
	return true;
}

void GuiModule::deinit(){
	printf("DEINIT\n");
	thePool.unregister(this);
	disposeEvents();
	//gtk_widget_hide(m_window);
	//gtk_widget_destroy(m_window);
	//g_object_unref(G_OBJECT(m_window));
	gtk_widget_destroy(m_window);
	loop_iteration();
}

void GuiModule::setTime(int playtime, int currenttime){
	if (playtime != -1)
		m_playtime = playtime;
	if (currenttime != -1)
		m_currenttime = currenttime;

	char tmp[128];
	int minutes = m_playtime/60;
	int seconds = m_playtime-minutes*60;

	int currmin = m_currenttime/60;
	int currsec = m_currenttime-currmin*60;
	sprintf(tmp, "%i:%02d/%i:%02d", currmin, currsec, minutes, seconds);
	gtk_label_set_text(GTK_LABEL(m_time), tmp);
}

void GuiModule::eventOccurred(DataPoolEntry entry, DataPoolItem item){
	switch(entry){
		case DP_DEBUG:
		{
			std::string text; item.read(text);
			gtk_label_set_text(GTK_LABEL(m_debug), text.c_str());
		}
		break;
		case DP_CURRENT_METADATA:
		{
			std::map<std::string,std::string> metadata;
			item.read(metadata);
			gtk_label_set_text(GTK_LABEL(m_filename), metadata["file"].c_str());
			gtk_label_set_text(GTK_LABEL(m_artist), metadata["Artist"].c_str());
			gtk_label_set_text(GTK_LABEL(m_title), metadata["Title"].c_str());
			gtk_label_set_text(GTK_LABEL(m_album), metadata["Album"].c_str());
			setTime(atoi(metadata["Time"].c_str()), -1);
			printf("Now playing: %s\n", metadata["file"].c_str());
		}
		case DP_CURRENT_POSITION:
		{
			int time; item.read(time);
			setTime(-1, time);
		}
	}
}

