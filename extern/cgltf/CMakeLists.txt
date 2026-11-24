# cgltf 1.15 (Johannes Kuhlmann, MIT; https://github.com/jkuhlmann/cgltf,
# release v1.15 of 2025-02-09): a glTF 2.0 parser (.gltf and .glb) in one
# header; the engine compiles its implementation (mesh/gltf.cpp)
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
