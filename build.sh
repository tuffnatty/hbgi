#!/bin/sh

HBMK2="$HOME/build/core/bin/darwin/gcc/hbmk2 -q0 -w2 -es2 -debug -optim-"

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
        hbgi-callbacks.c
        hbgi-info.c
        hbgi-invoke.c
        hbgi-signal-closure.c
        hbgi-type.c
        "

cd hb
#$HBMK2 -hbdyn -o../hbgihb $HBGIHB_SOURCES || exit $?
$HBMK2 -hblib -o../hbgihb $HBGIHB_SOURCES || exit $?

cd ../gobject
#$HBMK2 -hbdyn -lhbgihb -o../hbgobject -I../hb -I../gi $(pkg-config --cflags --libs glib-2.0) $HBGOBJECT_SOURCES || exit $?
$HBMK2 -hblib -o../hbgobject -I../hb -I../gi $(pkg-config --cflags glib-2.0) $HBGOBJECT_SOURCES || exit $?

cd ../gi
#$HBMK2 -hbdyn -lhbgobject -o../hbgi -I../hb -I../gobject -I../gi $(pkg-config --cflags --libs gobject-introspection-1.0) $HBGI_SOURCES || exit $?
$HBMK2 -hblib -o../hbgi -I../hb -I../gobject -I../gi $(pkg-config --cflags gobject-introspection-1.0) $HBGI_SOURCES || exit $?

cd ..
#$HBMK2 -L. -lhbgi -gtcgi test.prg || exit $?
$HBMK2 -L. -lhbgihb -lhbgobject -lhbgi -gtcgi $(pkg-config --libs gobject-introspection-1.0) test.prg || exit $?
