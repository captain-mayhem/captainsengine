#ifndef DATAPOOL_H
#define DATAPOOL_H

#include <map>
#include <list>

#include "datapoolitem.hxx"
#include "commands.hxx"


class DataPoolListener{
public:
  DataPoolListener() {pthread_mutex_init(&m_mutex, NULL);}
  
  virtual ~DataPoolListener() {pthread_mutex_destroy(&m_mutex);}
  
  virtual void eventOccurred(DataPoolEntry entry, DataPoolItem item)=0;
  
  void processEvents() {
    pthread_mutex_lock(&m_mutex);
    for (std::list<std::pair<DataPoolEntry,DataPoolItem> >::iterator iter = m_events.begin(); iter != m_events.end(); ++iter){
      eventOccurred(iter->first, iter->second);
      iter->second.dispose();
    }
    m_events.clear();
    pthread_mutex_unlock(&m_mutex);
  }
  
  void enqueueEvent(DataPoolEntry entry, DataPoolItem item) {
  	pthread_mutex_lock(&m_mutex);
	m_events.push_back(std::make_pair(entry, item));
  	pthread_mutex_unlock(&m_mutex);
  }
  
  void disposeEvents(){
    pthread_mutex_lock(&m_mutex);
    for (std::list<std::pair<DataPoolEntry,DataPoolItem> >::iterator iter = m_events.begin(); iter != m_events.end(); ++iter){
      iter->second.dispose();
    }
    m_events.clear();
    pthread_mutex_unlock(&m_mutex);
  }
protected:
  std::list<std::pair<DataPoolEntry,DataPoolItem> > m_events;
  pthread_mutex_t m_mutex;
};

class DataPool {
public:
  DataPool();
  ~DataPool();
  void put(DataPoolEntry entry, DataPoolItem item);
  DataPoolItem get(DataPoolEntry entry);
  void registerOnUpdate(DataPoolEntry entry, DataPoolListener* listener);
  void registerOnChange(DataPoolEntry entry, DataPoolListener* listener);
  void unregister(DataPoolListener* listener);
  void lock() {pthread_mutex_lock(&m_mutex);}
  void unlock() {pthread_mutex_unlock(&m_mutex);}
protected:
  std::map<DataPoolEntry, DataPoolItem> m_pool;
  std::map<DataPoolEntry, std::list<DataPoolListener*> > m_updateListeners;
  std::map<DataPoolEntry, std::list<DataPoolListener*> > m_changeListeners;
  pthread_mutex_t m_mutex;
};

#endif
