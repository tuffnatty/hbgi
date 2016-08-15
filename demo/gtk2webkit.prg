/*
  Demo for Webkit usage in Harbour using HBGI library
  If you press mouse right button, it will show a menu for navigation.
  2016 Rafa Carmona
*/

MEMVAR Gtk, GtkSource, GLib, WebKit

INIT PROCEDURE my_init()

   /* Require GTK+ version 2.0 as my installation of WebKitGtk requires this version */
   PUBLIC Gtk := hbgi_import( "Gtk", "2.0" )
   PUBLIC WebKit := hbgi_import( "WebKit" )

   RETURN


FUNCTION Main()

   LOCAL win, button, box, scroll, web, statusbar, contextid

   Gtk:init( hb_AParams() )

   /* Create window */
   win := Gtk:Window:new( Gtk:WindowType:TOPLEVEL )

   /* Add title at the window */
   win:set_title( "Harbour HBGI with WebKit" )

   /* Position window at center */
   win:set_position( Gtk:WindowPosition:CENTER )

   /* Size by default */
   win:set_default_size( 800, 600 )

   /* Connect 'delete-event' signal to quit the main loop */
   win:connect( 'delete-event', {|| Gtk:main_quit() } )

   /* Create vertical Box */
   box := Gtk:VBox:new( .F., 0 )

   /* Box add at Window */
   win:add( box )

   /* Create scroll */
   scroll := Gtk:ScrolledWindow:New( NIL, NIL )

   /* Border 10 */
   scroll:set_border_width( 10 )

   /* Show ALWAYS scrollbar*/
   scroll:set_policy( Gtk:PolicyType:AUTOMATIC, Gtk:PolicyType:ALWAYS )

   /* Add scroll to box */
   box:add( scroll )

   /* Create WebKit! VERY EASY!! */
   web := WebKit:WebView:new()

   /* Put a web app into the webview */
   web:load_uri( "https://harbour.github.io/" )

   /* Add WebKit to the scroll window */
   scroll:add( web )

   /* Simple button, connect 'clicked' signal to exit app */
   button := Gtk:Button:new_with_label( "Exit" )
   button:connect( 'clicked', {|| Gtk:main_quit() } )
   box:pack_start( button, .F., .F., 0 )

   /* Status Bar*/
   statusbar := Gtk:StatusBar:new()
   box:pack_end( statusbar, .F., .T., 0 )

   /* Push message in StatusBar */
   contextid := statusbar:get_context_id( "Statusbar example" )
   statusbar:push( contextid, "WebKit example running with HBGI (c)2016 Rafa Carmona" )

   /* Show ALL widgets */
   win:show_all()

   /* Here, start the main event loop of GTK+ */
   Gtk:main()

   win := NIL
   button := NIL
   web := NIL
   scroll := NIL

   RETURN 0
