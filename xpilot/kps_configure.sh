#! /bin/sh

export CC=gcc
#export CC=g++

# don't use these with g++
export CFLAGS_C=" \
-pedantic \
-Wmissing-declarations \
-Wnested-externs \
-Wsequence-point \
-Wbad-function-cast \
"

export CFLAGS=" \
-O2 \
-g \
-Wall \
-Wmissing-prototypes \
-Wstrict-prototypes \
-Wredundant-decls \
-Wshadow \
-Wswitch \
-Wparentheses \
-Wendif-labels \
-Wundef \
-Wmissing-braces \
-Wcomment \
-Wunused \
-Wreturn-type \
-Wpointer-arith \
-Wsign-compare \
-Wcast-align \
-Wcast-qual \
-Wuninitialized \
-Wimplicit \
-Winline \
-Wchar-subscripts \
-Wtrigraphs \
-Wformat=2 \
-Wwrite-strings \
-Wimport \
-Wdisabled-optimization \
"

if [ "x$CC" != "xg++" ]; then
    CFLAGS="$CFLAGS $CFLAGS_C"
fi

./configure --prefix=$HOME
