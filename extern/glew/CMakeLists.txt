ADD_LIBRARY(glew STATIC
src/glew.c
)
target_include_directories(glew PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(glew PUBLIC GLEW_STATIC)
find_package(OpenGL REQUIRED)
target_link_libraries(glew PUBLIC OpenGL::GL)
cge_extern_defaults(glew)

INSTALL(TARGETS glew DESTINATION Engine/lib COMPONENT CGE)
INSTALL(FILES include/GL/glew.h DESTINATION Engine/include/GL COMPONENT CGE)
