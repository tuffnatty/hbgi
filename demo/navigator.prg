/*
 * Example of Glade and WebKit usage
 * Easy Navigator from Harbour
 * (c) 2016 Rafa Carmona
*/

MEMVAR Gtk, WebKit

INIT PROCEDURE my_init()

   PUBLIC Gtk := hbgi_import( "Gtk" )
   PUBLIC WebKit := hbgi_import( "WebKit" )

   RETURN


FUNCTION Main()

   LOCAL win, glade, scroll, web, imgmenu_quit, entry_url, btn1, btn2

   Gtk:init( hb_AParams() )

   glade := Gtk:Builder:new()
   glade:add_from_file( "./navigator.ui" )

   win := glade:get_object( "window1" )
   win:connect( 'delete-event', {|| Gtk:main_quit() } )

   scroll := glade:get_object( "scrolledwindow1" )

   web := WebKit:WebView:new()

   /* Put a web app into the webview */
   web:load_uri( "https://harbour.github.io/" )
   web:connect( "onload-event", {|| entry_url:set_text( web:get_uri() ) } )
   scroll:add( web )

   imgmenu_quit := glade:get_object( "imagemenuitem1" )
   imgmenu_quit:connect( 'activate', {|| Gtk:main_quit() } )

   entry_url := glade:get_object("entry1" )
   entry_url:connect( 'activate', {|| web:load_uri( entry_url:get_buffer():get_text() ) } )

   btn1 := glade:get_object( "button1" )
   btn1:connect( "clicked", {|| web:go_back() } )

   btn2 := glade:get_object( "button2" )
   btn2:connect( "clicked", {|| web:go_forward() } )

   win:show_all()
   Gtk:main()

   RETURN 0
