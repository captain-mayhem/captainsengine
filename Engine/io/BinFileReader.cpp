
#include "BinFileReader.h"

using namespace CGE;

BinFileReader::BinFileReader(std::string filename){
  mStream.open(filename.c_str(), std::ios::binary);
}

BinFileReader::~BinFileReader(){
  mStream.close();
}

uint32 BinFileReader::readBytes(unsigned char* array, unsigned int length){
  mStream.read((char*)array,length);
  return length;
}

char BinFileReader::readChar(){
  char ret;
  mStream.read(&ret, sizeof(char));
  return ret;
}

unsigned char BinFileReader::readUChar(){
  unsigned char ret;
  mStream.read((char*)&ret, sizeof(unsigned char));
  return ret;
}

short BinFileReader::readShort(){
  short ret;
  mStream.read((char*)&ret, sizeof(short));
  return ret;
}

unsigned short BinFileReader::readUShort(){
  unsigned short ret;
  mStream.read((char*)&ret, sizeof(unsigned short));
  return ret;
}

int BinFileReader::readInt(){
  int ret;
  mStream.read((char*)&ret, sizeof(int));
  return ret;
}

unsigned int BinFileReader::readUInt(){
  unsigned int ret;
  mStream.read((char*)&ret, sizeof(unsigned int));
  return ret;
}

int64 BinFileReader::readLong(){
	int64 ret;
	mStream.read((char*)&ret, sizeof(int64));
	return ret;
}

uint64 BinFileReader::readULong(){
	uint64 ret;
	mStream.read((char*)&ret, sizeof(uint64));
	return ret;
}

bool BinFileReader::isWorking(){
  return !!mStream;
}

void BinFileReader::skip(int numberOfElements){
  mStream.seekg(numberOfElements, std::ios::cur);
}

std::string BinFileReader::readString(){
  return "";
}

std::string BinFileReader::readLine(){
  return "";
}

uint32 BinFileReader::getSize(){
  unsigned pos = mStream.tellg();
  mStream.seekg(0, std::ios::beg);
  unsigned start = mStream.tellg();
  mStream.seekg(0, std::ios::end);
  unsigned end = mStream.tellg();
  mStream.seekg(pos, std::ios::beg);
  return end - start;
}

