cp libavcodec/libavcodec.so /mnt/hgfs/pcdk/lib
cp libavformat/libavformat.so /mnt/hgfs/pcdk/lib
cp libavutil/libavutil.so /mnt/hgfs/pcdk/lib
cp libswresample/libswresample.so /mnt/hgfs/pcdk/lib
cp libswscale/libswscale.so /mnt/hgfs/pcdk/lib
arm-linux-androideabi-strip --strip-unneeded /mnt/hgfs/pcdk/lib/libavcodec.so
arm-linux-androideabi-strip --strip-unneeded /mnt/hgfs/pcdk/lib/libavformat.so
arm-linux-androideabi-strip --strip-unneeded /mnt/hgfs/pcdk/lib/libavutil.so
arm-linux-androideabi-strip --strip-unneeded /mnt/hgfs/pcdk/lib/libswresample.so
arm-linux-androideabi-strip --strip-unneeded /mnt/hgfs/pcdk/lib/libswscale.so

