#include <string>

#include <gtest/gtest.h>

#include <io/ZipReader.h>

#include "testenv.h"

// archive.zip (tests/data/make_fixtures.py): hello.txt (deflated, 20 lines)
// and dir/stored.bin (stored, the bytes 0..15). On Windows ZipReader maps the
// file into memory; elsewhere it reads it through Assets.

namespace{

std::string readAll(CGE::MemReader& rdr){
  std::string ret(rdr.getSize(), '\0');
  if (!ret.empty())
    rdr.readBytes((unsigned char*)&ret[0], (unsigned)ret.size());
  return ret;
}

std::string repeated(const std::string& line, int count){
  std::string ret;
  for (int i = 0; i < count; ++i)
    ret += line;
  return ret;
}

}

TEST(ZipReader, ReadsDeflatedEntry){
  CGE::ZipReader zip(TestEnv::dataFile("archive.zip"));
  CGE::MemReader rdr = zip.openEntry("hello.txt");
  EXPECT_EQ(readAll(rdr), repeated("Hello from the zip\n", 20));
}

TEST(ZipReader, ReadsStoredEntryInFolder){
  CGE::ZipReader zip(TestEnv::dataFile("archive.zip"));
  CGE::MemReader rdr = zip.openEntry("dir/stored.bin");
  ASSERT_EQ(rdr.getSize(), 16u);
  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(rdr.readUChar(), i);
}

TEST(ZipReader, EntriesCanBeReadAgainInAnyOrder){
  CGE::ZipReader zip(TestEnv::dataFile("archive.zip"));
  {
    CGE::MemReader rdr = zip.openEntry("dir/stored.bin");
    EXPECT_EQ(rdr.getSize(), 16u);
  }
  {
    CGE::MemReader rdr = zip.openEntry("hello.txt");
    EXPECT_EQ(readAll(rdr), repeated("Hello from the zip\n", 20));
  }
  {
    CGE::MemReader rdr = zip.openEntry("dir/stored.bin");
    EXPECT_EQ(rdr.readUChar(), 0);
    EXPECT_EQ(rdr.readUChar(), 1);
  }
}

TEST(ZipReader, MissingEntryIsEmpty){
  CGE::ZipReader zip(TestEnv::dataFile("archive.zip"));
  EXPECT_EQ(zip.openEntry("missing.txt").getSize(), 0u);
  //also when asked for a second time (the reader caches entry positions)
  EXPECT_EQ(zip.openEntry("missing.txt").getSize(), 0u);
}

TEST(ZipReader, MissingArchive){
  CGE::ZipReader zip;
  EXPECT_FALSE(zip.openFile(TestEnv::dataFile("missing.zip")));
}
