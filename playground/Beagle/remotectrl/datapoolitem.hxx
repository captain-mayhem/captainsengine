#ifndef DATAPOOL_ITEM_H
#define DATAPOOL_ITEM_H

#include <string>
#include <vector>
#include <map>

class DataPoolItem{
public:
  DataPoolItem();
  DataPoolItem(int val);
  DataPoolItem(const std::string& val);
  DataPoolItem(const char* val);
  template <typename T>
  DataPoolItem(const std::vector<T>& val);
  template <typename S, typename T>
  DataPoolItem(const std::map<S,T>& val);
  ~DataPoolItem();

  void dispose();
  DataPoolItem clone();
  bool operator==(const DataPoolItem& dpi) const;

  void read(int& val);
  void read(std::string& val);
  template<typename T>
  void read(std::vector<T>& val);
  template<typename S, typename T>
  void read(std::map<S,T>& val);
protected:
  inline size_t getSize(int val) {return sizeof(val);}
  inline size_t getSize(const std::string& val) {return sizeof(int)+val.size();}
  unsigned char* put(unsigned char* data, size_t val);
  unsigned char* put(unsigned char* data, const std::string& val);
  unsigned char* get(unsigned char* data, size_t& val);
  unsigned char* get(unsigned char* data, std::string& val);
  unsigned char* m_data;
  size_t m_size;
};

#include "datapoolitem.inl"

#endif
