SET(PRE io)

SET(Engine_io_SRCS
	${PRE}/BinFileReader.cpp
	${PRE}/eventqueue.cpp
	${PRE}/MemReader.cpp
	${PRE}/Tracing.cpp
)

SET(Engine_io_HDRS
	${PRE}/BinFileReader.h
	${PRE}/eventqueue.h
	${PRE}/MemReader.h
	${PRE}/Reader.h
	${PRE}/Tracing.h
)

INSTALL(FILES ${Engine_io_HDRS} DESTINATION Engine/include/${PRE})