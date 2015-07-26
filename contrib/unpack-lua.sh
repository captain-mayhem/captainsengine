#!/bin/bash
tar -xzf lua-5.0.2.tar.gz
export DIR=lua-5.0.2
export OUTDIR=$1/lua
mkdir $OUTDIR
cp $DIR/src/*.c $OUTDIR
cp $DIR/src/lib/*.c $OUTDIR
cp $DIR/src/*.h $OUTDIR
cp $DIR/include/*.h $OUTDIR
rm -rf $DIR
