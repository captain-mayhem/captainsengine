SET(PRE io)

SET(Engine_io_SRCS
	${PRE}/BinFileReader.cpp
	${PRE}/eventqueue.cpp
	${PRE}/GZipReader.cpp
	${PRE}/MemReader.cpp
  ${PRE}/MemWriter.cpp
	${PRE}/TraceManager.cpp
	${PRE}/Tracing.cpp
	${PRE}/ZipReader.cpp
  ${PRE}/ZipWriter.cpp
)

SET(Engine_io_HDRS
	${PRE}/BinFileReader.h
	${PRE}/eventqueue.h
	${PRE}/GZipReader.h
	${PRE}/MemReader.h
  ${PRE}/MemWriter.h
	${PRE}/Reader.h
	${PRE}/TraceManager.h
	${PRE}/Tracing.h
  ${PRE}/Writer.h
	${PRE}/ZipReader.h
  ${PRE}/ZipWriter.h
)

INSTALL(FILES ${Engine_io_HDRS} DESTINATION Engine/include/${PRE})