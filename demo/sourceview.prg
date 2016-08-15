/*
  Show use GtkViewSource in Harbour using library HBGI
  2016 Rafa Carmona
*/

MEMVAR Gtk, GtkSource, GLib, WebKit

INIT PROCEDURE my_init()

   PUBLIC Gtk := hbgi_import( "Gtk" )
   PUBLIC GtkSource := hbgi_import( "GtkSource" )

   RETURN


FUNCTION Main()

   LOCAL win, button, box, scroll, sourceview, statusbar, contextid

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

   /* Create Widget GtkSourceView */
   sourceview := GtkSource:View:new()

   /* Show line numbers */
   sourceview:set_show_line_numbers( .T. )

   /* Set highlight current line */
   sourceview:set_highlight_current_line( .T. )

   /* Add sourceview to scroll */
   scroll:add( sourceview )

   /* Simple button, connect 'clicked' signal to exit app */
   button := Gtk:Button:new_with_label( "Exit" )
   button:connect( 'clicked', {|| Gtk:main_quit() } )
   box:pack_start( button, .F., .F., 0 )

   /* Status Bar*/
   statusbar := Gtk:StatusBar:new()
   box:pack_end( statusbar, .F., .T., 0 )

   /* Push message in StatusBar */
   contextid := statusbar:get_context_id( "Statusbar example" )
   statusbar:push( contextid, "GtkSourceView example running with HBGI (c)2016 Rafa Carmona" )

   /* Show ALL widgets */
   win:show_all()

   /* Here, start the main event loop of GTK+ */
   Gtk:main()

   win := NIL
   button := NIL
   sourceview := NIL
   scroll := NIL

   RETURN 0
