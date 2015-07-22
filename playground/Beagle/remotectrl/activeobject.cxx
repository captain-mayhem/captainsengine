#include "activeobject.hxx"

ActiveObject::ActiveObject() : m_is_running(false){
}

ActiveObject::~ActiveObject(){
}

void ActiveObject::run(){
	if (m_is_running)
		return;
	m_is_running = true;
	pthread_create(&m_thread, NULL, start_routine, this);
}

void ActiveObject::terminate(){
	if (m_is_running){
		m_is_running = false;
		pthread_join(m_thread, NULL);
	}
	else
		m_is_running = false;
}

bool ActiveObject::init(){
	return true;
}

void ActiveObject::deinit(){
}

void* ActiveObject::start_routine(void* arg){
	ActiveObject* that = (ActiveObject*)arg;
	if (!that->init())
		return NULL;
	while(that->m_is_running){
		that->loop_iteration();
	}
	that->deinit();
	return NULL;
}

void ActiveObject::sleep(int milliseconds){
	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = milliseconds*1000000;
	nanosleep(&tv, NULL);
}

