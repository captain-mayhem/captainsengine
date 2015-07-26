#include "datapoolitem.hxx"

#include <cstdlib>
#include <cstring>

DataPoolItem::DataPoolItem() : m_data(NULL), m_size(0){}

DataPoolItem::DataPoolItem(int val) : m_data(NULL), m_size(sizeof(val)){
  m_data = new unsigned char[m_size];
  put(m_data, val);
}

DataPoolItem::DataPoolItem(const std::string& val){
  m_size = getSize(val);
  m_data = new unsigned char[m_size];
  put(m_data, val);
}

DataPoolItem::DataPoolItem(const char* val){
  m_size = strlen(val);
  m_data = new unsigned char[m_size];
  memcpy(m_data, val, m_size);
}

DataPoolItem::~DataPoolItem(){
}

void DataPoolItem::read(int& val){
  get(m_data, (size_t&)val);
}

void DataPoolItem::read(std::string& val){
	get(m_data, val);
}

unsigned char* DataPoolItem::put(unsigned char* data, size_t val){
  data[0] = (unsigned char)((val >> 24) & 0xFF);
  data[1] = (unsigned char)(val >> 16) & 0xFF;
  data[2] = (unsigned char)(val >> 8) & 0xFF;
  data[3] = (unsigned char)(val >> 0) & 0xFF;
  return data + sizeof(val);
}

unsigned char* DataPoolItem::put(unsigned char* data, const std::string& val){
  data = put(data, val.size());
  memcpy(data, val.c_str(), val.size());
  return data+val.size();
}

unsigned char* DataPoolItem::get(unsigned char* data, size_t& val){
  val = ((size_t)data[0]) << 24;
  val |= ((size_t)data[1]) << 16;
  val |= ((size_t)data[2]) << 8;
  val |= ((size_t)data[3]) << 0;
  return data + sizeof(val);
}

unsigned char* DataPoolItem::get(unsigned char* data, std::string& val){
  size_t length;
  data = get(data, length);
  val.resize(length);
  memcpy(&val[0], data, length);
  return data+length;
}

void DataPoolItem::dispose(){
  delete [] m_data;
  m_data = NULL;
}

DataPoolItem DataPoolItem::clone(){
  DataPoolItem ret;
  ret.m_size = m_size;
  ret.m_data = new unsigned char[m_size];
  memcpy(ret.m_data, m_data, m_size);
  return ret;
}
  
bool DataPoolItem::operator==(const DataPoolItem& dpi) const{
	if (m_size != dpi.m_size)
		return false;
	return memcmp(m_data, dpi.m_data, m_size) == 0;
}
