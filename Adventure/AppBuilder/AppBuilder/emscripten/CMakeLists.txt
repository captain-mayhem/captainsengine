ADD_EXECUTABLE(empers.js
  main.cpp
)

SET_TARGET_PROPERTIES(empers.js PROPERTIES LINK_FLAGS "-O2 -s FULL_ES2=1 -s OUTLINING_LIMIT=10000 -s TOTAL_MEMORY=268435456 -s ASM_JS=1 -s EXPORTED_FUNCTIONS=\"['_main', '_fsMounted', '_fileWritten', '_fileSynced']\"")
