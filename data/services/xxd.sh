#!/bin/sh

SOURCE_DIR=$1
SOURCE_FILE=$2

cd $SOURCE_DIR
xxd -i $SOURCE_FILE