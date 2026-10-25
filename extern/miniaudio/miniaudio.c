/*
 *   miniaudio implementation translation unit.
 *
 *   miniaudio (https://miniaud.io) is a single-file audio playback/capture
 *   library (public domain / MIT-0).  This file is the one place the
 *   implementation is compiled; everything else just includes "miniaudio.h"
 *   for the declarations.
 *
 *   The engine's audio module (Engine/audio) uses the decoders, the node
 *   graph and the high-level engine. The parts it doesn't need are compiled
 *   out by the miniaudio target's public definitions (CMakeLists.txt), so
 *   the implementation and every user of the header see the same
 *   structures.
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
