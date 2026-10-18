add_executable(FBXTest
fbxtest.cpp
)

target_include_directories(FBXTest PRIVATE ${FBX_INCLUDE_DIR})
target_link_libraries(FBXTest PRIVATE fbxsdk)