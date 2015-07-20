IF(${OS} STREQUAL "wince")
ADD_DEFINITIONS(-D_MSC_VER=10)
ENDIF(${OS} STREQUAL "wince")

ADD_LIBRARY(xml STATIC
  tinyxmlparser.cpp
  xmltest.cpp
  tinystr.cpp
  tinyxml.cpp
  tinyxmlerror.cpp
)

INSTALL(TARGETS xml ARCHIVE DESTINATION Engine/lib)

