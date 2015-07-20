#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/input.h>

#include "gui.hxx"
#include "mpdclient.hxx"
#include "datapool.hxx"
#include "commands.hxx"

GuiModule theGui;
MPDClient theMusic;
DataPool thePool;

bool key_press(char key, bool ctrl, bool alt, bool meta, bool shift)
{
	std::ostringstream tmp;
	tmp << "Pressed ";
	if (ctrl)
		tmp << "CTRL+";
	if (alt)
		tmp << "ALT+";
	if (meta)
		tmp << "META+";
	if (shift)
		tmp << "SHIFT+";
	tmp << (int)key << ", (0x" << std::hex << (int)key << ")\n" << std::dec;
	thePool.put(DP_DEBUG, tmp.str());
	printf("%s", tmp.str().c_str());
	//mpc control
	if (ctrl && shift && key==KEY_P){
		//system("mpc play");
		thePool.put(DP_COMMAND, MC_PLAY);
		//theMusic.play();
	}
	else if (ctrl && key==KEY_P){
		//system("mpc pause");
		thePool.put(DP_COMMAND, MC_PAUSE);
		//theMusic.pause();
	}
	else if (ctrl && shift && key==KEY_S){
		system("mpc stop");
	}
	else if (ctrl && key==KEY_F){
		//system("mpc next");
		thePool.put(DP_COMMAND, MC_NEXT);
		//theMusic.next();
	}
	else if (ctrl && key==KEY_B){
		//system("mpc prev");
		thePool.put(DP_COMMAND, MC_PREV);
		//theMusic.prev();
	}
	//system control
	else if (alt && key==KEY_F4){
		system("shutdown 0");
		return false;
	}
	else if (ctrl && key==KEY_T){
		//end program without shutdown
		return false;
	}
	else if (alt && key==KEY_ENTER){
		if (theGui.isRunning())
			theGui.terminate();
		else
			theGui.run();
	}
	return true;
}

void key_release(char key){
}

void input_device_loop()
{
	//setup ir connection
	int fd = open("/dev/input/by-id/usb-HOLTEK_PC_receiver-event-kbd", O_RDONLY | O_SYNC);
	if (fd == -1){
		printf("Infrared receiver not found - not connected?");
		//exit(EXIT_FAILURE);
	}

	fd_set readset;

	struct input_event events[64];
	bool ctrl_pressed = false;
	bool shift_pressed = false;
	bool alt_pressed = false;
	bool meta_pressed = false;
	int scancode = 0;
	bool finished = false;
	while (!finished){
		FD_ZERO(&readset);
		FD_SET(fd, &readset);
		select(fd+1, &readset, NULL, NULL, NULL);
		if (FD_ISSET(fd, &readset)){
			int size = read(fd, events, 64*sizeof(struct input_event))/sizeof(struct input_event);
			//some nice little debug output
			/*printf("Received %i events: ", size);
			for (int i = 0; i < size; ++i){
				printf("Reveived event type %i, code %i, value %i", events[i].type, events[i].code, events[i].value);
			}
			printf("\n");*/

			for (int i = 0; i < size; ++i){
				if (events[i].type == EV_KEY){
					if (events[i].code == KEY_LEFTCTRL){
						if (events[i].value == 1)
							ctrl_pressed = true;
						else
							ctrl_pressed = false;
					}
					else if (events[i].code == KEY_LEFTSHIFT){
						if (events[i].value == 1)
							shift_pressed = true;
						else
							shift_pressed = false;
					}
					else if(events[i].code == KEY_LEFTALT){
						if (events[i].value == 1)
							alt_pressed = true;
						else
							alt_pressed = false;
					}
					else if (events[i].code == KEY_LEFTMETA){
						if (events[i].value == 1)
							meta_pressed = true;
						else
							meta_pressed = false;
					}
					else if (events[i].value == 1){
						finished = !key_press(events[i].code, ctrl_pressed, alt_pressed, meta_pressed, shift_pressed);
					}
					else{
						key_release(events[i].code);
					}
				}
				else if (events[i].type == EV_MSC && events[i].code == MSC_SCAN){
					scancode = events[i].value;
				}
			}
		}
	}

	close(fd);
}

int main(int argc, char** argv){
	ActiveObject::sleep(1000);
	theMusic.run();
	input_device_loop();
	printf("Shutting down remotctrl\n");
	theGui.terminate();
	theMusic.terminate();
	printf("Exiting remotctrl\n");
	return 0;
}
