#ifndef CGE_TESTENV_H
#define CGE_TESTENV_H

#include <string>

namespace TestEnv{

//! the folder with the test files (tests/data), without trailing separator
inline std::string dataDir(){
  return CGE_TEST_DATA_DIR;
}

//! a test file's path in that folder
inline std::string dataFile(const std::string& name){
  return dataDir() + "/" + name;
}

}

#endif
