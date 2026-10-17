ADD_LIBRARY(xml STATIC
  tinyxmlparser.cpp
  tinystr.cpp
  tinyxml.cpp
  tinyxmlerror.cpp
)
target_include_directories(xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
cge_extern_defaults(xml)

INSTALL(TARGETS xml ARCHIVE DESTINATION Engine/lib)
