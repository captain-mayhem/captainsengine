#!/bin/bash
tar -xzf zlib-1.2.3.tar.gz
export DIR=zlib-1.2.3
export OUTDIR=$1/zlib
mkdir $OUTDIR
cp $DIR/*.c $OUTDIR
cp $DIR/*.h $OUTDIR
cp $DIR/contrib/minizip/*.c $OUTDIR
cp $DIR/contrib/minizip/*.h $OUTDIR
rm -rf $DIR
