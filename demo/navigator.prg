/*
 
 Example use Glade and WebKit 
 Easy Navigator from Harbour
 (c)2016 Rafa Carmona
 
*/
memvar gtk, WebKit

init procedure my_init()
public gtk := hbgi_import("Gtk")
public WebKit := hbgi_import("WebKit")
return

function Main()
  local win, glade, scroll, Web, imgmenu_quit, entry_url , btn1,btn2

  gtk:init( hb_aParams() )

  glade := gtk:Builder():New( )
  glade:add_from_file( "./navigator.ui" )

  win := glade:get_object("window1")
  win:connect('delete-event', {|| gtk:main_quit() })

  scroll := glade:get_object("scrolledwindow1")

  web := Webkit:WebView:New()
  // Put the web app into the webview
  web:load_uri("https://harbour.github.io/" )
  web:connect( "onload-event", {|| entry_url:Set_Text( web:get_uri() )} )
  scroll:add( web )

  imgmenu_quit := glade:get_object("imagemenuitem1" )
  imgmenu_quit:connect('activate', {|| gtk:main_quit() })
 
  entry_url := glade:get_object("entry1" )
  entry_url:connect('activate', {|| web:load_uri( entry_url:get_Buffer():get_text() ) } )
  
  btn1 := glade:get_object("button1" )
  btn1:connect( "clicked", {|| web:go_back() } )
  
  btn2 := glade:get_object("button2" )
  btn2:connect( "clicked", {|| web:go_forward() } )
  
  win:show_all()
  gtk:main()

return 0




