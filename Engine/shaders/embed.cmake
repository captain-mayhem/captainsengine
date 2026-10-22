# Writes shader files into a C++ source as raw string literals, so a library
# or program carries them and needn't ship them.
#
# The engine's built-in shaders (Engine/shaders/):
#   cmake -DSOURCE_DIR=<Engine/shaders> -DOUTPUT=<file.cpp> -P embed.cmake
# An application's shaders (cge_embed_shaders in Engine/CMakeLists.txt):
#   cmake -DSOURCE_DIR=<dir> -DOUTPUT=<file.cpp> -DPREFIX=<prefix/> -DFUNCTION=<name> -P embed.cmake
# The second form defines void <name>(), which hands the files to
# CGE::Gfx::ShaderLibrary::addFiles under the paths <prefix/><file name>.

if (NOT DEFINED PREFIX)
	set(PREFIX "engine/")
endif()

file(GLOB files RELATIVE ${SOURCE_DIR} ${SOURCE_DIR}/*.glsl ${SOURCE_DIR}/*.vert ${SOURCE_DIR}/*.frag)
list(SORT files)
set(entries "")
foreach(name ${files})
	file(READ ${SOURCE_DIR}/${name} text)
	string(REPLACE "\r\n" "\n" text "${text}")
	string(FIND "${text}" ")cge\"" bad)
	if (NOT bad EQUAL -1)
		message(FATAL_ERROR "${name} contains the raw string delimiter )cge\"")
	endif()
	# MSVC limits a single string literal to about 16 KB; adjacent ones are joined
	string(LENGTH "${text}" length)
	set(literal "")
	set(pos 0)
	while (pos LESS length)
		string(SUBSTRING "${text}" ${pos} 8000 chunk)
		string(APPEND literal "R\"cge(${chunk})cge\"\n")
		math(EXPR pos "${pos}+8000")
	endwhile()
	if (literal STREQUAL "")
		set(literal "\"\"")
	endif()
	string(APPEND entries "  {\"${PREFIX}${name}\",\n${literal}},\n")
endforeach()

if (DEFINED FUNCTION)
	set(out "// Generated from ${SOURCE_DIR} by embed.cmake. Don't edit.\n\n#include <renderer/shaderlibrary.h>\n\nnamespace{\n\nconst CGE::Gfx::ShaderFile files[] = {\n${entries}};\n\n}\n\nvoid ${FUNCTION}(){\n  CGE::Gfx::ShaderLibrary::addFiles(files, (int)(sizeof(files)/sizeof(files[0])));\n}\n")
else()
	set(out "// Generated from Engine/shaders/ by embed.cmake. Don't edit.\n\n#include \"renderer/builtinshaders.h\"\n\nnamespace CGE{\nnamespace Gfx{\nnamespace Builtin{\n\nextern const ShaderFile files[] = {\n${entries}};\n\nextern const int fileCount = sizeof(files)/sizeof(files[0]);\n\n}\n}\n}\n")
endif()
file(WRITE ${OUTPUT} "${out}")
