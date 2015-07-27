export TOOLCHAIN=/home/captain/android/toolchain
export PATH=$TOOLCHAIN/bin:$PATH
export CC=arm-linux-androideabi-gcc
export LD=arm-linux-androideabi-ld
export AR=arm-linux-androideabi-ar

#-fasm \
CFLAGS="-O3 -Wall -mthumb -pipe -fpic \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack \
  -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ \
  -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ \
  -DANDROID -DNDEBUG \
  -DHAVE_LOG2F=0 -DHAVE_LOG2=0"
  
#EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
EXTRA_CFLAGS="-mfloat-abi=softfp"
EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 -Wl,--no-undefined"

FFMPEG_FLAGS="--prefix=/home/captain/ffmpeg/ffmpeg_android \
  --target-os=linux \
  --arch=arm \
  --enable-cross-compile \
  --cross-prefix=arm-linux-androideabi- \
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
  --disable-asm \
  --enable-version3"

../ffmpeg/configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS $EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS"
