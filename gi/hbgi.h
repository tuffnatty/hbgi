/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 */

#ifndef HBGI_H
#define HBGI_H

#include <hbapi.h>
#include <hbapiitm.h>

#include <glib-object.h>

#define HBGI_ERR 50000

#define HBGI_IVAR_GTYPE 1
#define HBGI_IVAR_INFO 2
#define HBGI_IVAR_GOBJECT 3
#define HBGI_IVAR_METHOD_INFO 4
#define HBGI_IVAR_FREE_ON_DEALLOC 5
#define HBGI_IVAR_COUNT 5

#define hbg_object_get(o) hb_arrayGetPtr((o), HBGI_IVAR_GOBJECT)

struct HbGI_API {
   HB_USHORT (*type_import_by_g_type)(GType g_type);
   GClosure * (*signal_closure_new)(PHB_ITEM instance, const gchar *sig_name, PHB_ITEM callback, PHB_ITEM extra_args, PHB_ITEM swap_data);
};

static struct HbGI_API *HbGI_API = NULL;

static int
_hbgi_import(void)
{
   PHB_ITEM pAPI;

   if (HbGI_API != NULL) {
      return 1;
   }
   if (hb_memvarGet(pAPI = hb_itemNew(NULL), hb_dynsymFindSymbol("HBGI_API")) == HB_SUCCESS) {
      HbGI_API = hb_itemGetPtr(pAPI);
   }
   hb_itemRelease(pAPI);
   if (HbGI_API == NULL) {
      return -1;
   }
   return 0;
}

static inline HB_USHORT
hbgi_type_import_by_g_type(GType g_type)
{
    if (_hbgi_import() < 0) {
        return 0;
    }
    return HbGI_API->type_import_by_g_type(g_type);
}

static inline GClosure *
hbgi_signal_closure_new(PHB_ITEM instance, const gchar *sig_name, PHB_ITEM callback, PHB_ITEM extra_args, PHB_ITEM swap_data)
{
    if (_hbgi_import() < 0) {
        return 0;
    }
    return HbGI_API->signal_closure_new(instance, sig_name, callback, extra_args, swap_data);
}

#endif
