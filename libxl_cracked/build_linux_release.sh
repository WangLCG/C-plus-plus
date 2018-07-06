#!/bin/bash
WORKING_DIR=`pwd`
SRC_DIR=$WORKING_DIR/src
BUILD_RELEASE_DIR=$WORKING_DIR/build_release
if [ ! -d $BUILD_RELEASE_DIR ]; then
	mkdir -p $BUILD_RELEASE_DIR
fi
cd $BUILD_RELEASE_DIR
cmake $SRC_DIR
make
