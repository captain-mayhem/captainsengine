# Writes the engine's built-in shader files (Engine/shaders/) into a C++
# source as raw string literals, so the engine library carries them and
# applications needn't ship them.
# cmake -DSOURCE_DIR=<Engine/shaders> -DOUTPUT=<file.cpp> -P embed.cmake

file(GLOB files RELATIVE ${SOURCE_DIR} ${SOURCE_DIR}/*.glsl ${SOURCE_DIR}/*.vert ${SOURCE_DIR}/*.frag)
list(SORT files)
set(out "// Generated from Engine/shaders/ by embed.cmake. Don't edit.\n\n#include \"renderer/builtinshaders.h\"\n\nnamespace CGE{\nnamespace Gfx{\nnamespace Builtin{\n\nextern const ShaderFile files[] = {\n")
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
	string(APPEND out "  {\"engine/${name}\",\n${literal}},\n")
endforeach()
string(APPEND out "};\n\nextern const int fileCount = sizeof(files)/sizeof(files[0]);\n\n}\n}\n}\n")
file(WRITE ${OUTPUT} "${out}")
