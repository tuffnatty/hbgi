/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGTYPE_H
#define HBGTYPE_H

#include <glib-object.h>

#include <hbapi.h>

GClosure *
hbg_closure_new(PHB_ITEM callback, PHB_ITEM extra_args, PHB_ITEM swap_data);

gint
hbg_enum_get_value(GType enum_type, PHB_ITEM obj, gint *val);

gint
hbg_flags_get_value(GType flag_type, PHB_ITEM obj, gint *val);

PHB_ITEM
hbg_param_gvalue_as_hbitem(const GValue* gvalue,
                             gboolean copy_boxed,
			     const GParamSpec* pspec);

GType
_hbg_type_from_name(const gchar *name);

GType
hbg_type_from_object(PHB_ITEM obj);

GType
hbg_type_from_object_strict(PHB_ITEM obj, gboolean strict);

HB_USHORT
hbg_type_get_custom(const gchar *name);

PHB_ITEM
hbg_type_wrapper_new(GType type);

PHB_ITEM
hbg_value_as_hbitem(const GValue *value, gboolean copy_boxed);

int
hbg_value_from_hbitem(GValue *value, PHB_ITEM obj);

#endif
