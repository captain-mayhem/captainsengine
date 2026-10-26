# pl_mpeg (Dominic Szablewski, MIT; https://github.com/phoboslab/pl_mpeg,
# commit c871f2b of 2025-12-30): MPEG-1 video, MP2 audio and the MPEG-PS
# demuxer in one header; the engine compiles its implementation
add_library(pl_mpeg INTERFACE)
target_include_directories(pl_mpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
