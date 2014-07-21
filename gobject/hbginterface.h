/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGINTERFACE_H
#define HBGINTERFACE_H

#include <hbapi.h>

#include <glib.h>

extern GQuark hbginterface_type_key;
extern HB_USHORT HbGInterface_Type;

void
hbgobject_interface_register_types(void);

#endif
