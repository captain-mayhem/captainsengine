export CC=emcc
export LD=llvm-link
export AR=emar

#-fasm \
CFLAGS="-O2 -Wall -pipe \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack \
  -DNDEBUG \
  "  
#EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
#EXTRA_CFLAGS="-mfloat-abi=softfp"
#EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 -Wl,--no-undefined"

FFMPEG_FLAGS="--prefix=/home/captain/ffmpeg/ffmpeg_emscripten/dist \
  --cc="emcc" \
  --ar="emar" \
  --ranlib="emranlib" \
  --target-os=none \
  --arch=x86_32 \
  --cpu=generic \
  --enable-cross-compile \
  --disable-shared \
  --disable-doc \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-ffserver \
  --disable-avdevice \
  --disable-avfilter \
  --disable-encoders  \
  --disable-muxers \
  --disable-filters \
  --disable-devices \
  --disable-everything \
  --enable-protocols  \
  --disable-bsfs \
  --disable-network \
  --enable-swscale  \
  --disable-asm \
  --enable-version3 \
  --disable-pthreads \
  --disable-w32threads \
  --disable-hwaccels \
  --disable-debug \
  --disable-indevs \
  --disable-outdevs \
 \
--enable-decoder=adpcm_ima_wav \
--enable-decoder=adpcm_sbpro_2 \
--enable-decoder=adpcm_sbpro_3 \
--enable-decoder=adpcm_sbpro_4 \
--enable-decoder=mp1float \
--enable-decoder=mp2 \
--enable-decoder=mp2float \
--enable-decoder=mp3 \
--enable-decoder=mp3adu \
--enable-decoder=mp3adufloat \
--enable-decoder=mp3float \
--enable-decoder=mp3on4 \
--enable-decoder=mp3on4float \
--enable-decoder=mpeg1video \
--enable-decoder=mpeg2video \
--enable-decoder=mpeg4 \
--enable-decoder=mpegvideo \
--enable-decoder=flv \
--enable-decoder=msmpeg4v1 \
--enable-decoder=msmpeg4v2 \
--enable-decoder=msmpeg4v3 \
--enable-decoder=msvideo1 \
--enable-decoder=mts2 \
--enable-decoder=pbm \
--enable-decoder=pcm_alaw \
--enable-decoder=pcm_f32be \
--enable-decoder=pcm_f32le \
--enable-decoder=pcm_f64be \
--enable-decoder=pcm_f64le \
--enable-decoder=pcm_lxf \
--enable-decoder=pcm_mulaw \
--enable-decoder=vorbis \
--enable-decoder=pcm_s16be \
--enable-decoder=pcm_s16be_planar \
--enable-decoder=pcm_s16le \
--enable-decoder=pcm_s16le_planar \
--enable-decoder=pcm_s24be \
--enable-decoder=pcm_s24daud \
--enable-decoder=pcm_s24le \
--enable-decoder=pcm_s24le_planar \
--enable-decoder=pcm_s32be \
--enable-decoder=pcm_s32le \
--enable-decoder=pcm_s32le_planar \
--enable-decoder=wavpack \
--enable-decoder=pcm_s8 \
--enable-decoder=pcm_s8_planar \
--enable-decoder=pcm_u16be \
--enable-decoder=wmalossless \
--enable-decoder=pcm_u16le \
--enable-decoder=wmapro \
--enable-decoder=pcm_u24be \
--enable-decoder=wmav1 \
--enable-decoder=pcm_u24le \
--enable-decoder=wmav2 \
--enable-decoder=pcm_u32be \
--enable-decoder=wmavoice \
--enable-decoder=pcm_u32le \
--enable-decoder=wmv1 \
--enable-decoder=pcm_u8 \
--enable-decoder=wmv2 \
--enable-decoder=pcm_zork \
--enable-decoder=wmv3 \
--enable-decoder=theora \
\
--enable-parser=mpegvideo \
--enable-parser=vorbis \
--enable-parser=mpegaudio \
\
--enable-demuxer=pcm_s16be \
--enable-demuxer=pcm_s16le \
--enable-demuxer=pcm_s24be \
--enable-demuxer=pcm_s24le \
--enable-demuxer=pcm_s32be \
--enable-demuxer=pcm_s32le \
--enable-demuxer=pcm_s8 \
--enable-demuxer=pcm_u16be \
--enable-demuxer=pcm_u16le \
--enable-demuxer=pcm_u24be \
--enable-demuxer=pcm_u24le \
--enable-demuxer=pcm_u32be \
--enable-demuxer=pcm_u32le \
--enable-demuxer=pcm_u8 \
--enable-demuxer=avi \
--enable-demuxer=m4v \
--enable-demuxer=mov \
--enable-demuxer=mp3 \
--enable-demuxer=mpegvideo \
--enable-demuxer=swf \
--enable-demuxer=ogg \
--enable-demuxer=pcm_alaw \
--enable-demuxer=pcm_f32be \
--enable-demuxer=pcm_f32le \
--enable-demuxer=pcm_f64be \
--enable-demuxer=pcm_f64le \
--enable-demuxer=wav \
--enable-demuxer=pcm_mulaw \
--enable-demuxer=xwma \
"

BLA="  --enable-decoders \
  --enable-demuxers \
  --enable-parsers \
" 
FLA=" \
  --disable-decoders \
  --enable-decoder=pcm \
  --enable-decoder=mp3 \
  --enable-decoder=vorbis \
  \
  --disable-demuxers \
  --disable-demuxer=sbg \
  --enable-demuxer=mp3 \
  --enable-demuxer=ogg \
  --enable-demuxer=wav \
  \
  --disable-parsers \
  --enable-parser=vorbis \
"

emconfigure ../ffmpeg/configure $FFMPEG_FLAGS 
#--extra-cflags="$CFLAGS $EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS"
