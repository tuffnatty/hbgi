/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGPOINTER_H
#define HBGPOINTER_H

#include <hbapi.h>

#include <glib.h>

extern GQuark hbgpointer_type_key;
extern HB_USHORT HbGPointer_Type;

#define HBGPOINTER_IVAR_GTYPE 1
#define HBGPOINTER_IVAR_POINTER 2
#define HBGPOINTER_IVAR_COUNT 2

void
hbgobject_pointer_register_types(void);

#endif
