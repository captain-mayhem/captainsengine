include_directories(${FBX_INCLUDE_DIR})

add_executable(FBXTest
fbxtest.cpp
)

target_link_libraries(FBXTest fbxsdk)