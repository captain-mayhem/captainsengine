#!/bin/bash
export TOPDIR=extern
mkdir $TOPDIR
./unpack-jpeg.sh $TOPDIR
./unpack-lua.sh $TOPDIR
./unpack-xml.sh $TOPDIR
./unpack-zlib.sh $TOPDIR
./unpack-ode.sh $TOPDIR
