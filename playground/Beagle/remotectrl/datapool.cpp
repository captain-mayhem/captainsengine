#include "datapool.hxx"

DataPool::DataPool(){
	pthread_mutex_init(&m_mutex, NULL);
}

DataPool::~DataPool(){
	for (std::map<DataPoolEntry,DataPoolItem>::iterator iter = m_pool.begin(); iter != m_pool.end(); ++iter){
		iter->second.dispose();
	}
	pthread_mutex_destroy(&m_mutex);
}

void DataPool::put(DataPoolEntry entry, DataPoolItem item){
	pthread_mutex_lock(&m_mutex);
	std::map<DataPoolEntry,DataPoolItem>::iterator search = m_pool.find(entry);
	bool changed = true;
	if (search != m_pool.end()){
		if (item == search->second)
			changed = false;
		search->second.dispose();
	}
	else{
		if (m_updateListeners.find(entry) == m_updateListeners.end())
			m_updateListeners[entry] = std::list<DataPoolListener*>();
		if (m_changeListeners.find(entry) == m_changeListeners.end())
			m_changeListeners[entry] = std::list<DataPoolListener*>();
	}
	m_pool[entry] = item;
	std::map<DataPoolEntry,std::list<DataPoolListener*> >::iterator iter = m_updateListeners.find(entry);
	if (iter != m_updateListeners.end()){
		for (std::list<DataPoolListener*>::iterator listeners =iter->second.begin(); listeners != iter->second.end(); ++listeners){
			(*listeners)->enqueueEvent(entry, item.clone());
		}
	}
	if (changed){
		std::map<DataPoolEntry,std::list<DataPoolListener*> >::iterator iter = m_changeListeners.find(entry);
		if (iter != m_changeListeners.end()){
			for (std::list<DataPoolListener*>::iterator listeners =iter->second.begin(); listeners != iter->second.end(); ++listeners){
				(*listeners)->enqueueEvent(entry, item.clone());
			}
		}
	}
	pthread_mutex_unlock(&m_mutex);
}

DataPoolItem DataPool::get(DataPoolEntry entry){
	pthread_mutex_lock(&m_mutex);
	DataPoolItem ret = m_pool[entry];
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

void DataPool::registerOnUpdate(DataPoolEntry entry, DataPoolListener* listener){
	pthread_mutex_lock(&m_mutex);
	m_updateListeners[entry].push_back(listener);
	pthread_mutex_unlock(&m_mutex);
}

void DataPool::registerOnChange(DataPoolEntry entry, DataPoolListener* listener){
	pthread_mutex_lock(&m_mutex);
	m_changeListeners[entry].push_back(listener);
	pthread_mutex_unlock(&m_mutex);
}

void DataPool::unregister(DataPoolListener* listener){
	pthread_mutex_lock(&m_mutex);
	for (std::map<DataPoolEntry, std::list<DataPoolListener*> >::iterator iter = m_updateListeners.begin(); iter != m_updateListeners.end(); ++iter){
		for (std::list<DataPoolListener*>::iterator listy = iter->second.begin(); listy != iter->second.end(); ++listy){
			if (*listy == listener){
				listy = iter->second.erase(listy);
				if (listy == iter->second.end())
					break;
			}
		}
	}
	for (std::map<DataPoolEntry, std::list<DataPoolListener*> >::iterator iter = m_changeListeners.begin(); iter != m_changeListeners.end(); ++iter){
		for (std::list<DataPoolListener*>::iterator listy = iter->second.begin(); listy != iter->second.end(); ++listy){
			if (*listy == listener){
				listy = iter->second.erase(listy);
				if (listy == iter->second.end())
					break;
			}
		}
	}
	pthread_mutex_unlock(&m_mutex);
}
