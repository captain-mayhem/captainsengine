SET(PRE gui)

SET(Engine_gui_SRCS
	${PRE}/console.cpp
	${PRE}/dialog.cpp
	${PRE}/dropdown.cpp
	${PRE}/filedialog.cpp
	${PRE}/gui.cpp
	${PRE}/listbox.cpp
	${PRE}/messagebox.cpp)

INSTALL(FILES 	
	${PRE}/console.h 
	${PRE}/dialog.h 
	${PRE}/dropdown.h 
	${PRE}/filedialog.h 
	${PRE}/gui.h 
	${PRE}/listbox.h 
	${PRE}/messagebox.h 
DESTINATION Engine/include/${PRE})