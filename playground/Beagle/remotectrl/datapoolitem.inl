template <typename T>
DataPoolItem::DataPoolItem(const std::vector<T>& val){
  size_t size = val.size();
  m_size = sizeof(size);
  for (unsigned i = 0; i < size; ++i){
    m_size += getSize(val[i]);
  }
  m_data = new unsigned char[m_size];
  unsigned char* fill = put(m_data, val.size());
  for (unsigned i = 0; i < size; ++i){
    fill = put(fill, val[i]);
  }
}

template<typename T>
void DataPoolItem::read(std::vector<T>& val){
  size_t size;
  unsigned char* data = get(m_data, size);
  val.reserve(size);
  for (unsigned i = 0; i < size; ++i){
    T elem;
    data = get(data, elem);
    val.push_back(elem);
  }
}


template <typename S, typename T>
DataPoolItem::DataPoolItem(const std::map<S,T>& val){
  size_t size = val.size();
  m_size = sizeof(size);
  for (typename std::map<S,T>::const_iterator iter = val.begin(); iter != val.end(); ++iter){
    m_size += getSize(iter->first);
    m_size += getSize(iter->second);
  }
  m_data = new unsigned char[m_size];
  unsigned char* fill = put(m_data, val.size());
  for (typename std::map<S,T>::const_iterator iter = val.begin(); iter != val.end(); ++iter){
    fill = put(fill, iter->first);
    fill = put(fill, iter->second);
  }
}

template<typename S, typename T>
void DataPoolItem::read(std::map<S,T>& val){
  size_t size;
  unsigned char* data = get(m_data, size);
  for (unsigned i = 0; i < size; ++i){
    S key;
    T value;
    data = get(data, key);
    data = get(data, value);
    val[key] = value;
  }
}

