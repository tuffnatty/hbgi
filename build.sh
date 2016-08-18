#!/bin/sh -e

# hbmk2 must be in PATH

# Windows - MSYS2 packages required:
#    pacman -S mingw-w64-{i686,x86_64}-{gtk2,gobject-introspection}

# static
hbmk2 gihb.hbp
hbmk2 gobject.hbp
hbmk2 gi.hbp
hbmk2 test.prg -otest_s -gtcgi hbgi.hbc

# dynamic
hbmk2 -hbdyn -shared '{win}-lhbmaindllp' @gi.hbp @gobject.hbp @gihb.hbp -ohbgidyn
hbmk2 test.prg -otest_d -gtcgi -env:HBGI_DYNAMIC=yes hbgi.hbc
