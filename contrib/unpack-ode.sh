#!/bin/bash
tar -xzf ode-0.11.tar.gz
export DIR=ode-0.11
export OUTDIR=$1/ode
mkdir $OUTDIR
cp -r $DIR/include/ode $OUTDIR/ode
cp $DIR/ode/src/*.cpp $OUTDIR
cp $DIR/ode/src/*.c $OUTDIR
cp $DIR/ode/src/*.h $OUTDIR
cp -r $DIR/ode/src/joints $OUTDIR
cp $DIR/OPCODE/*.cpp $OUTDIR
cp $DIR/OPCODE/*.h $OUTDIR
cp -r $DIR/OPCODE/Ice $OUTDIR
rm -rf $DIR
