#!/bin/sh

PACKAGE=lua-espeak
VERSION=1.36r1

DIRNAME=$PACKAGE-$VERSION
TGZNAME=$DIRNAME.tar.gz

lua doc.lua > doc/index.html

rm -f $TGZNAME
mkdir $DIRNAME

cp -r COPYING $DIRNAME
cp -r Makefile $DIRNAME
cp -r README $DIRNAME
cp -r ChangeLog $DIRNAME
cp -r luaespeak.c $DIRNAME
cp -r test_espeak.lua $DIRNAME

cp -r demos $DIRNAME
rm -rf $DIRNAME/demos/.svn

cp -r doc $DIRNAME
rm -rf $DIRNAME/doc/.svn

tar -czf $TGZNAME $DIRNAME
rm -rf $DIRNAME
tar -tzf $TGZNAME

