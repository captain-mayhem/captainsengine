#include "mpdclient.hxx"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "datapool.hxx"
#include "commands.hxx"

extern DataPool thePool;

bool MPDClient::init(){
	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(6600);
	if (::connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		return false;
	}
	std::string hello = receive();
	printf("%s\n", hello.c_str());
	thePool.registerOnUpdate(DP_COMMAND, this);
	thePool.registerOnChange(DP_CURRENT_SONGID, this);
	return true;
}

bool MPDClient::loop_iteration(){
	/*int bytes = send(m_socket, "currentsong\n", 13, 0);
	//int bytes = send(m_socket, "status\n", 7, 0);
	//int bytes = send(m_socket, "idle\n", 5, 0);
	if (bytes < 0)
		printf("MPD: send failed\n");
	std::string song = receive();
	if (song != "OK\n")
		printf("%s", song.c_str());*/
	//sleep(1);
	processEvents();
	checkStatus();
	sleep(50);
	return true;
}

void MPDClient::deinit(){
	thePool.unregister(this);
	disposeEvents();
	close(m_socket);
}


void MPDClient::send(const std::string& cmd){
	::send(m_socket, cmd.c_str(), cmd.size(), 0);
}

std::string MPDClient::receive(){
	fd_set readset;
	FD_ZERO(&readset);
	FD_SET(m_socket, &readset);
	select(m_socket+1, &readset, NULL, NULL, NULL);
	
	std::string result;
	char buff[1024];
	int bytes = 0;
	do{
		bytes = recv(m_socket, buff, 1023, 0);
		buff[bytes] = '\0';
		result += std::string(buff);
	} while(bytes == 1023);

	return result;
}
	
void MPDClient::play(){
	send("play\n");
	std::string res = receive();
	//printf("play (%s)\n", res.c_str());
}

void MPDClient::pause(){
	send("pause 1\n");
	std::string res = receive();
	//printf("pause (%s)\n", res.c_str());
}

void MPDClient::next(){
	send("next\n");
	receive();
}

void MPDClient::prev(){
	send("previous\n");
	receive();
}

void MPDClient::checkStatus(){
	send("status\n");
	std::map<std::string,std::string> status = convertToMap(receive());
	std::map<std::string,std::string>::iterator iter;
	if ((iter = status.find("songid")) != status.end()){
		int id = atoi(iter->second.c_str());
		thePool.put(DP_CURRENT_SONGID, id);
	}
	if ((iter = status.find("time")) != status.end()){
		int time = atoi(iter->second.c_str());
		thePool.put(DP_CURRENT_POSITION, time);
	}
}

std::map<std::string,std::string> MPDClient::convertToMap(const std::string& str){
	std::map<std::string,std::string> res;
	size_t frompos = 0;
	size_t topos = str.find('\n', frompos);
	while (topos != std::string::npos){
		std::string line = str.substr(frompos, topos-frompos);
		size_t kv_pos = line.find(':');
		if (kv_pos != std::string::npos){
			std::string key = line.substr(0, kv_pos);
			std::string value = line.substr(kv_pos+2);
			//std::cout << key << ": " << value << "\n";
			res[key] = value;
		}
		frompos = topos+1;
		topos = str.find('\n', frompos);
	}
	return res;
}

void MPDClient::eventOccurred(DataPoolEntry entry, DataPoolItem item){
	switch(entry){
		case DP_COMMAND:
		{
			Command cmd;
			item.read((int&)cmd);
			//printf("Command %i\n", cmd);
			switch (cmd){
				case MC_PLAY:
					play();
					break;
				case MC_PAUSE:
					pause();
					break;
				case MC_NEXT:
					next();
					break;
				case MC_PREV:
					prev();
					break;
			}
			break;
		}
		case DP_CURRENT_SONGID:
		{
			int id; item.read(id);
			char tmp[64];
			sprintf(tmp, "playlistid %i\n", id);
			send(tmp);
			std::string res = receive();
			//printf("Info: %s", res.c_str());
			std::map<std::string,std::string> metadata = convertToMap(res);
			thePool.put(DP_CURRENT_METADATA, metadata);
			break;
		}
	}
}

