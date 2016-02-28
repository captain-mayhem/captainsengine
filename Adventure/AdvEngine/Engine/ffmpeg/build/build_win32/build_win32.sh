
CFLAGS=""
  
#EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
EXTRA_CFLAGS="-O2"
EXTRA_LDFLAGS="-static-libgcc -static"

FFMPEG_FLAGS="--prefix=/home/captain/ffmpeg/ffmpeg_win32 \
  --target-os=mingw32 \
  --arch=x86 \
  --enable-cross-compile \
  --cross-prefix=i686-w64-mingw32- \
  --pkg-config=pkg-config \
  --enable-memalign-hack \
  --enable-shared \
  --disable-symver \
  --disable-doc \
  --disable-ffplay \
  --disable-ffmpeg \
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
  --enable-parsers \
  --enable-demuxers \
  --disable-demuxer=sbg \
  --enable-decoders \
  --enable-bsfs \
  --enable-network \
  --enable-swscale  \
  --enable-asm \
  --enable-version3"

../ffmpeg/configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS $EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS"
