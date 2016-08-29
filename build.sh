#!/bin/sh -e

if ! which hbmk2 > /dev/null; then
   echo '! Error: hbmk2 missing from PATH'
   exit 1
fi

if ! pkg-config glib-2.0 || \
   ! pkg-config gobject-introspection-1.0; then
   echo '! Error: Required components missing: glib2, gobject-introspection'
   case "$(uname)" in
      *_NT*)
      echo '! Install using:'
      echo '  pacman -S mingw-w64-{i686,x86_64}-{gtk2,glib2,gobject-introspection}'
      ;;
   esac
   exit 1
fi

DEMOS="test demo/gtk2webkit demo/navigator demo/sourceview demo/webkit"

# static
hbmk2 gihb.hbp
hbmk2 gobject.hbp
hbmk2 gi.hbp
for F in $DEMOS; do
    hbmk2 $F.prg -o${F}_s -gtcgi hbgi.hbc
done

# dynamic
hbmk2 -hbdyn -shared '{win}-lhbmaindllp' @gi.hbp @gobject.hbp @gihb.hbp -ohbgidyn
for F in $DEMOS; do
    hbmk2 $F.prg -o${F}_d -gtcgi -env:HBGI_DYNAMIC=yes hbgi.hbc
done
