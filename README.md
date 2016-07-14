hbgi
====

hbgi - Harbour bindings for GObject Introspection

These are bindings for the GObject Introspection library, to be used in Harbour. It is in no way complete, but already usable for writing moderately complex programs. It is almost fully based on pygobject - the similar bindings for Python - and can be viewed as the successor to xbgtk (http://xbgtk.sourceforge.net) - the older Harbour bindings for GTK+.

Please report any issues on GitHub issues page at:
  https://github.com/tuffnatty/hbgi/issues
  
Requirements
============

  * C compiler (tested with GCC on OS X)
  * Harbour 3.0.0 or higher
  * GObject Introspection

Copyright Information
=====================
This software is covered by the GNU Lesser General Public License
(version 2.1, or if you choose, a later version).  Basically just don't
say you wrote bits you didn't.

Documentation and Examples
==========================
As the project is modeled after Python bindings for GObject Introspection, I think the best source for examples are Python examples - with the exception that they must be translated to Harbour, but this is mostly straightforward.
Where to find the examples:

  * http://lazka.github.io/pgi-docs/#Gtk-3.0 - full API reference
  * http://python-gtk-3-tutorial.readthedocs.io/en/latest/ - tutorials
  * https://developer.gnome.org/gnome-devel-demos/stable/py.html.en - some demo apps
  * https://git.gnome.org/browse/pygobject/tree/demos/gtk-demo - gtk-demo app

Compilation
===========

So far only a Unix-based build script is available. You need hbmk2 from Harbour on your PATH. To build, it should be as simple as running:

    $ ./build.sh
