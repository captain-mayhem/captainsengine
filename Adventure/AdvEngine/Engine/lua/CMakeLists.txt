ADD_LIBRARY(cge SHARED
lua_bindings.cpp
lua_bindings.h
cge_io.cpp
cge_io.h
)

TARGET_LINK_LIBRARIES(cge PRIVATE AdventureBase)