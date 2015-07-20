#!/bin/bash
tar -xzf jpeg.tar.gz
export DIR=jpeg-6b
export OUTDIR=$1/jpeg
mkdir $OUTDIR
cp $DIR/*.c $OUTDIR
cp $DIR/*.h $OUTDIR
rm -rf $DIR
