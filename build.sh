#!/bin/sh

HARBOUR_HOME="$HOME/build/harbour-git"
HARBOUR_PLAT="darwin/gcc"
HBMK2="$HARBOUR_HOME/bin/$HARBOUR_PLAT/hbmk2 -q0 -w2 -es2 -debug -optim-"

# If HBGI_DYN is set to "no", build everything statically; helps debugging
HBGI_DYN="no"

HBGIHB_SOURCES="
        hbgihb.c
        "

HBGOBJECT_SOURCES="
        hbgobject.c
        hbginterface.c
        hbgpointer.c
        hbgtype.c
        "

HBGI_SOURCES="
        hbgicore.c
        hbgi-argument.c
        hbgi-boxed.c
        hbgi-callbacks.c
        hbgi-info.c
        hbgi-invoke.c
        hbgi-signal-closure.c
        hbgi-struct-marshal.c
        hbgi-type.c
        "

HBGOBJECT_FLAGS="$(pkg-config --cflags --libs glib-2.0)"
HBGI_FLAGS="$(pkg-config --cflags --libs gobject-introspection-1.0)"

for F in $HBGIHB_SOURCES; do HBGIHB_SOURCES_PREFIXED="$HBGIHB_SOURCES_PREFIXED hb/$F"; done
for F in $HBGOBJECT_SOURCES; do HBGOBJECT_SOURCES_PREFIXED="$HBGOBJECT_SOURCES_PREFIXED gobject/$F"; done
for F in $HBGI_SOURCES; do HBGI_SOURCES_PREFIXED="$HBGI_SOURCES_PREFIXED gi/$F"; done

if [ $HBGI_DYN == "no" ]; then

cd hb
$HBMK2 -hblib -o../hbgihb $HBGIHB_SOURCES || exit $?

cd ../gobject
$HBMK2 -hblib -o../hbgobject -I../hb -I../gi $HBGOBJECT_FLAGS $HBGOBJECT_SOURCES || exit $?

cd ../gi
$HBMK2 -hblib -o../hbgi -I../hb -I../gobject -I../gi $HBGI_FLAGS $HBGI_SOURCES || exit $?

cd ..

$HBMK2 -L. -lhbgihb -lhbgobject -lhbgi -gtcgi $HBGI_FLAGS test.prg || exit $?

else

$HBMK2 -hbdyn -ohbgi -g $HBGIHB_SOURCES_PREFIXED $HBGOBJECT_SOURCES_PREFIXED $HBGI_SOURCES_PREFIXED -Ihb -Igobject -Igi $HBGOBJECT_FLAGS $HBGI_FLAGS || exit $?

#$HBMK2 -L. -lhbgi -gtcgi test.prg || exit $?
$HBMK2 -L. -lhbgi -L$HARBOUR_HOME/lib/$HARBOUR_PLAT -gtcgi test.prg || exit $?

fi
