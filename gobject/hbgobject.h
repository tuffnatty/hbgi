/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGOBJECT_H
#define HBGOBJECT_H

#include <hbapi.h>
#include <hboo.ch>

#include "hbgihb.h"

#include <glib-object.h>

#include "hbgi.h"

#define HBGOBJECT_ERR 40000

#define HBGOBJECT_IVAR_GTYPE 1

typedef void (*HbClosureExceptionHandler)(GValue *ret, guint n_param_values, const GValue *params);
typedef PHB_ITEM (*fromvaluefunc)(const GValue *value);
typedef int (*tovaluefunc)(GValue *value, PHB_ITEM obj);

typedef struct {
   GClosure closure;
   PHB_ITEM callback, extra_args, swap_data;
   HbClosureExceptionHandler exception_handler;
} HbGClosure;

typedef struct {
   HB_USHORT type;
   GSList *closures;
} HbGObjectData;

typedef enum {
       HBGOBJECT_USING_TOGGLE_REF = 1 << 0
} HbGObjectFlags;

extern HB_USHORT HbGObject_Type;

extern GQuark hbgobject_instance_data_key;
extern GType HB_TYPE_ITEM;

HB_USHORT
hbgobject_lookup_class(GType gtype);

PHB_ITEM
hbgobject_new(GObject *obj);

PHB_ITEM
hbgobject_new_sunk(GObject *obj);


static inline HbGObjectData *
hbg_object_peek_inst_data(GObject *obj)
{
   return ((HbGObjectData *) g_object_get_qdata(obj, hbgobject_instance_data_key));
}

#define hbgobject_get(o) ((GObject *)hb_arrayGetPtr((o), HBGI_IVAR_GOBJECT))

#define hbg_boxed_get(v,t)        ((t *)hb_arrayGetPtr((v), HBGI_IVAR_GOBJECT))
#define hbg_boxed_get_ptr(v)      hb_arrayGetPtr((v), HBGI_IVAR_GOBJECT)
#define hbg_boxed_set_ptr(v,p)    hb_arraySetPtr((v), HBGI_IVAR_GOBJECT, (p))

#define hbg_pointer_get(v,t)      ((t *)(hb_arrayGetPtr((v), HBGPOINTER_IVAR_POINTER)))
#define hbg_pointer_gtype(v)      ((GType)(hb_arrayGetNLL((v), HBGPOINTER_IVAR_GTYPE)))

#define HBGOBJECT_REGISTER_GTYPE(type, name, gtype)      \
  {                                                         \
    PHB_ITEM o = hbg_type_wrapper_new(gtype);              \
    hbgi_hb_clsAddData(type, "__gtype__", HB_OO_MSG_ACCESS, 0, HBGOBJECT_IVAR_GTYPE, o); \
    hb_itemRelease(o);                                           \
  }

#endif
