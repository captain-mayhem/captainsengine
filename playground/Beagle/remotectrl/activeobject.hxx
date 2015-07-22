#ifndef ACTIVE_OBJECT_HXX
#define ACTIVE_OBJECT_HXX

#include <pthread.h>

class ActiveObject{
public:
	ActiveObject();
	virtual ~ActiveObject();
	void run();
	void terminate();
	bool isRunning() {return m_is_running;}
	static void sleep(int milliseconds);
protected:
	virtual bool init();
	virtual bool loop_iteration()=0;
	virtual void deinit();

	static void* start_routine(void* arg);
	pthread_t m_thread;
	bool m_is_running;
};

#endif

