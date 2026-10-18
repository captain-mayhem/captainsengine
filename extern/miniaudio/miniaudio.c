/*
 *   miniaudio implementation translation unit.
 *
 *   miniaudio (https://miniaud.io) is a single-file audio playback/capture
 *   library (public domain / MIT-0).  This file is the one place the
 *   implementation is compiled; everything else just includes "miniaudio.h"
 *   for the declarations.
 *
 *   guit3 only needs playback, so the capture/decoding/encoding/resource
 *   pieces that pull in extra dependencies are compiled out here to keep the
 *   object small and the dependency surface minimal.  The TADS decoders
 *   (WAV/OGG/MP3) feed raw PCM straight into an ma_device, so miniaudio's own
 *   decoders are not used.
 */

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
