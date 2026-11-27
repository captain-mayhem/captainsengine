#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <io/MemReader.h>
#include <io/ZipReader.h>
#include <io/ZipWriter.h>

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

TEST(ZipReader, ListsItsEntries){
  CGE::ZipReader zip(TestEnv::dataFile("archive.zip"));
  std::vector<std::string> entries = zip.getEntries();
  ASSERT_EQ(entries.size(), 2u);
  EXPECT_EQ(entries[0], "hello.txt");
  EXPECT_EQ(entries[1], "dir/stored.bin");
  //listing doesn't disturb reading
  CGE::MemReader rdr = zip.openEntry("hello.txt");
  EXPECT_EQ(readAll(rdr), repeated("Hello from the zip\n", 20));
}

namespace{

std::vector<uint8_t> written(const std::vector<std::pair<std::string, std::string>>& entries, const std::string& password = ""){
  CGE::ZipWriter zip;
  for (auto& entry : entries)
    EXPECT_TRUE(zip.addEntry(entry.first, entry.second.data(), entry.second.size(), password));
  EXPECT_TRUE(zip.close());
  return zip.getData();
}

}

TEST(ZipWriter, WritesAnArchiveInMemory){
  std::vector<uint8_t> data = written({{"hello.txt", repeated("Hello from the zip\n", 20)}, {"dir/empty", ""}});
  ASSERT_FALSE(data.empty());
  CGE::ZipReader zip(data.data(), (uint32_t)data.size());
  std::vector<std::string> entries = zip.getEntries();
  ASSERT_EQ(entries.size(), 2u);
  EXPECT_EQ(entries[0], "hello.txt");
  EXPECT_EQ(entries[1], "dir/empty");
  CGE::MemReader rdr = zip.openEntry("hello.txt");
  EXPECT_EQ(readAll(rdr), repeated("Hello from the zip\n", 20));
  EXPECT_EQ(zip.openEntry("dir/empty").getSize(), 0u);
}

TEST(ZipWriter, EncryptsWithAPassword){
  std::string text = repeated("secret ", 100);
  std::vector<uint8_t> data = written({{"gfx/image.png", text}}, "pass");
  // the text isn't in the archive as it is
  std::string archive(data.begin(), data.end());
  EXPECT_EQ(archive.find("secret"), std::string::npos);
  CGE::ZipReader zip(data.data(), (uint32_t)data.size());
  CGE::MemReader rdr = zip.openEntry("gfx/image.png", "pass");
  EXPECT_EQ(readAll(rdr), text);
  EXPECT_EQ(zip.openEntry("gfx/image.png", "wrong").getSize(), 0u);
}

TEST(ZipWriter, ArchivesNest){
  std::vector<uint8_t> inner = written({{"inner.txt", "inside"}});
  CGE::ZipWriter outer;
  CGE::MemReader innerRdr(inner.data(), (unsigned)inner.size());
  EXPECT_TRUE(outer.addEntry("folder.zip", innerRdr));
  EXPECT_TRUE(outer.close());
  CGE::ZipReader zip((void*)outer.getData().data(), (uint32_t)outer.getData().size());
  CGE::MemReader rdr = zip.openEntry("folder.zip");
  std::string innerData = readAll(rdr);
  CGE::ZipReader innerZip(&innerData[0], (uint32_t)innerData.size());
  CGE::MemReader text = innerZip.openEntry("inner.txt");
  EXPECT_EQ(readAll(text), "inside");
}

TEST(ZipWriter, WritesAFile){
  std::string path = (std::filesystem::temp_directory_path()/"cge_test_written.zip").string();
  {
    CGE::ZipWriter zip(path);
    ASSERT_TRUE(zip.isWorking());
    EXPECT_TRUE(zip.addEntry("a.txt", "abc", 3));
  }
  {
    CGE::ZipReader zip(path);
    CGE::MemReader rdr = zip.openEntry("a.txt");
    EXPECT_EQ(readAll(rdr), "abc");
  }
  std::filesystem::remove(path);
}
