/*
  Show use Webkit in Harbour using library HBGI
  If you press  mouse right button, show menu for navigate.
  2016 Rafa Carmona
*/

#define TRUE  .T.
#define FALSE .F.

MEMVAR gtk, gtkSource, gLib, WebKit

INIT PROCEDURE my_init()

   PUBLIC gtk := hbgi_import( "Gtk" )
   PUBLIC WebKit := hbgi_import( "WebKit" )

   RETURN

FUNCTION Main()

   LOCAL win, button, box, scroll, web, statusbar, contextid

   gtk:init( hb_AParams() )

   /* Create window */
   win := gtk:Window:new( gtk:WindowType:TOPLEVEL )

   /* Add Title at the window */
   win:set_title( "Harbour HBGI with WebKit" )

   /* Center Position window */
   win:set_position( gtk:WindowPosition:CENTER )

   /* Size by Default*/
   win:set_default_size( 800, 600 )

   /* Connect event delete-event for destroy window */
   win:connect( 'delete-event', {|| gtk:main_quit() } )

   /* Create Box vertical  */
   box := gtk:vbox:new( .F., 0 )

   /* Box add at Window */
   win:add( box )

   /* Create scroll*/
   scroll := gtk:scrolledWindow:New( NIL, NIL )

   /* Border 10*/
   scroll:set_border_width( 10 )

   /* Show ALWAYS scrollbar*/
   scroll:set_policy( gtk:PolicyType:AUTOMATIC, Gtk:PolicyType:ALWAYS )

   /* Add scroll at box*/
   box:add( scroll )

   /* Create WEBKIT! VERY EASY!! */
   web := Webkit:WebView:New()

   /* Put the web app into the webview */
   web:load_uri( "https://harbour.github.io/" )

   /* Add WebKit a scroll */
   scroll:add( web )

   /* Simple button , connect event clicked, exit app */
   button := gtk:button:new_with_label( "Exit" )
   button:connect( 'clicked', {|| gtk:main_quit() } )
   box:pack_start( button, .F., .F., 0 )

   /* Status Bar*/
   statusbar := gtk:statusbar:new()
   box:pack_end( statusbar, FALSE, TRUE, 0 )

   /*Push message in StatusBar*/
   contextid := statusbar:get_context_id( "Statusbar example" )
   statusbar:push( contextid, "WebKit example running with HBGI (c)2016 Rafa Carmona" )

   /* Show ALL widgets */
   win:show_all()

   /* Here, loop main events of GTK*/
   gtk:Main()
   win := NIL
   button := NIL
   Web := NIL
   scroll := NIL

   RETURN 0
