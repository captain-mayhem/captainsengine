#ifndef MPD_CLIENT_H
#define MPD_CLIENT_H

#include "activeobject.hxx"
#include "datapool.hxx"

#include <string>

class MPDClient : public ActiveObject, public DataPoolListener{
public:
	//bool connect();
	void play();
	void pause();
	void next();
	void prev();
	virtual void eventOccurred(DataPoolEntry, DataPoolItem);
protected:
	virtual bool init();
	virtual bool loop_iteration();
	virtual void deinit();

	void send(const std::string& cmd);
	std::string receive();
	void checkStatus();
	std::map<std::string,std::string> convertToMap(const std::string& str);

	int m_socket;
};

#endif

