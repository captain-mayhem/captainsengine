#!/bin/bash
tar -xzf tinyxml_2_5_3.tar.gz
export DIR=tinyxml
export OUTDIR=$1/libxml
mkdir $OUTDIR
cp $DIR/*.cpp $OUTDIR
cp $DIR/*.h $OUTDIR
rm -rf $DIR
