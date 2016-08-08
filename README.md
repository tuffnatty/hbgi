hbgi
====

hbgi - Harbour bindings for GObject Introspection

These are bindings for the GObject Introspection library, to be used in Harbour. It is in no way complete, but already usable for writing moderately complex applications. It is almost fully based on pygobject - the similar bindings for Python - and can be viewed as the successor to [xbgtk](http://xbgtk.sourceforge.net) - the older Harbour bindings for GTK+.

Please report any issues on GitHub issues page at:
  https://github.com/tuffnatty/hbgi/issues
  
Requirements
============

  * C compiler (tested with GCC on OS X and Linux)
  * [Harbour](https://github.com/harbour/core) 3.0.0 or higher
  * [GObject Introspection](https://wiki.gnome.org/Projects/GObjectIntrospection)
  * [libffi](https://sourceware.org/libffi/)

Copyright Information
=====================
This software is covered by the GNU Lesser General Public License
(version 2.1, or if you choose, a later version).  Basically just don't
say you wrote bits you didn't.

Documentation and Examples
==========================
hbgi lacks documentation and examples. As the project is modeled after Python bindings for GObject Introspection, I think the best source for docs and examples are Python examples - with the exception that they must be translated to Harbour, but this is mostly straightforward.

hbgi examples
-------------
  * Examples by [Rafał Jopek](https://github.com/RJopek) at http://harbour.edu.pl/lib/hbgi/menu.html

Python GObject Introspection examples
-------------------------------------
  * https://developer.gnome.org/gnome-devel-demos/stable/py.html.en - some demo apps
  * https://git.gnome.org/browse/pygobject/tree/demos/gtk-demo - gtk-demo app

Python GObject Introspection documentation
------------------------------------------
  * http://lazka.github.io/pgi-docs/#Gtk-3.0 - full API reference
  * http://python-gtk-3-tutorial.readthedocs.io/en/latest/ - tutorials

Compilation
===========

So far only a Unix-based build script is available. You need `hbmk2` from Harbour on your `PATH`. To build, it should be as simple as running:

    $ ./build.sh

TODO List
=========
  * Properties getter/setter support via :props
  * Inheritance of Harbour classes from GObject classes
  * More examples
  * Documentation (https://github.com/lazka/pgi-docgen can be used I guess)
  * ...
