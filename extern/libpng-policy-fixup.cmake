# Injected into libpng's project() call via CMAKE_PROJECT_libpng_INCLUDE_BEFORE
# (wired up in extern/CMakeLists.txt).
#
# It runs after libpng's own cmake_minimum_required() - whose "...4.0" upper bound
# leaves policy CMP0194 unset - but before libpng enables the ASM language, which
# is the only window in which CMP0194 can be set for libpng's directory scope.
#
# libpng's project() enables ASM unconditionally.  On Windows the only candidate
# assembler is MSVC's cl, and CMake >= 4.1 warns about that through CMP0194.
# libpng adds no actual assembler sources here, so the OLD behaviour (accept cl,
# exactly as CMake <= 4.0 did) is harmless and silences the developer warning.
# Drop this once libpng no longer enables ASM unconditionally on Windows.
if(POLICY CMP0194)
	cmake_policy(SET CMP0194 OLD)
endif()
