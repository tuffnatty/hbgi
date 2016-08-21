memvar gtk

init procedure my_init()
  public gtk := hbgi_import("Gtk")
  return

function Main()
  local win, button

  gtk:init(hb_aParams())

  win := gtk:Window:new(gtk:WindowType:POPUP)
  win:connect('delete-event', {|| gtk:main_quit() })
  button := gtk:Button:new_with_label(E"Click me\nRight now")
  button:connect('clicked', {|| gtk:main_quit() })
  win:add(button)

  win:show_all()
  gtk:main()
  win := NIL
  button := NIL
  return 0
