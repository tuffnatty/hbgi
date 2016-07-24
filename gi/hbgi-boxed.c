/*
 * hbgi source code
 * Core code
 *
 * Copyright 2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-boxed.c from pygobject
 * library:
 *
 * Copyright (C) 2009 Simon van der Linden <svdlinden@src.gnome.org>
 *
 *   pygi-boxed.c: wrapper to handle registered structures.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <hbapi.h>
#include <hbapierr.h>
#include <hbapiitm.h>

#include "hbgihb.h"

#include <girepository.h>

#include "hbgobject.h"
#include "hbgi.h"
#include "hbgi-boxed.h"

#if 0
static void
_boxed_dealloc (PyGIBoxed *self)
{
    GType g_type;

    PyObject_GC_UnTrack ( (PyObject *) self);

    PyObject_ClearWeakRefs ( (PyObject *) self);

    if ( ( (PyGBoxed *) self)->free_on_dealloc) {
        if (self->slice_allocated) {
            g_slice_free1 (self->size, ( (PyGBoxed *) self)->boxed);
        } else {
            g_type = pyg_type_from_object ( (PyObject *) self);
            g_boxed_free (g_type, ( (PyGBoxed *) self)->boxed);
        }
    }

    Py_TYPE( (PyGObject *) self)->tp_free ( (PyObject *) self);
}
#endif

void *
_hbgi_boxed_alloc (GIBaseInfo *info, gsize *size_out)
{
    gsize size;
    gchar *msg;

    /* FIXME: Remove when bgo#622711 is fixed */
    if (g_registered_type_info_get_g_type (info) == G_TYPE_VALUE) {
        size = sizeof (GValue);
    } else {
        switch (g_base_info_get_type (info)) {
            case GI_INFO_TYPE_UNION:
                size = g_union_info_get_size ( (GIUnionInfo *) info);
                break;
            case GI_INFO_TYPE_BOXED:
            case GI_INFO_TYPE_STRUCT:
                size = g_struct_info_get_size ( (GIStructInfo *) info);
                break;
            default:
                msg = g_strdup_printf(
                              "info should be Boxed or Union, not '%d'",
                              g_base_info_get_type (info));
                hb_errRT_BASE_SubstR(EG_DATATYPE, 50080, __func__, msg, HB_ERR_ARGS_BASEPARAMS);
                g_free(msg);
                return NULL;
        }
    }

    if( size_out != NULL)
        *size_out = size;

    return g_slice_alloc0 (size);
}

#if 0
static PyObject *
_boxed_new (PyTypeObject *type,
            PyObject     *args,
            PyObject     *kwargs)
{
    static char *kwlist[] = { NULL };

    GIBaseInfo *info;
    gsize size = 0;
    gpointer boxed;
    PyGIBoxed *self = NULL;

    if (!PyArg_ParseTupleAndKeywords (args, kwargs, "", kwlist)) {
        return NULL;
    }

    info = _pygi_object_get_gi_info ( (PyObject *) type, &PyGIBaseInfo_Type);
    if (info == NULL) {
        if (PyErr_ExceptionMatches (PyExc_AttributeError)) {
            PyErr_Format (PyExc_TypeError, "missing introspection information");
        }
        return NULL;
    }

    boxed = _pygi_boxed_alloc (info, &size);
    if (boxed == NULL) {
        PyErr_NoMemory();
        goto out;
    }

    self = (PyGIBoxed *) _pygi_boxed_new (type, boxed, TRUE);
    if (self == NULL) {
        g_slice_free1 (size, boxed);
        goto out;
    }

    self->size = size;
    self->slice_allocated = TRUE;

out:
    g_base_info_unref (info);

    return (PyObject *) self;
}

static int
_boxed_init (PyObject *self,
             PyObject *args,
             PyObject *kwargs)
{
    /* Don't call PyGBoxed's init, which raises an exception. */
    return 0;
}

PYGLIB_DEFINE_TYPE("gi.Boxed", PyGIBoxed_Type, PyGIBoxed);
#endif

PHB_ITEM
_hbgi_boxed_new (HB_USHORT hb_type,
                 gpointer      boxed,
                 gboolean      free_on_dealloc)
{
    PHB_ITEM self;
    GType gtype;

    if (!boxed) {
        return NULL;
    }

    /*if (!hb_clsIsParent(hb_type, "HbgiBoxed")) {
        hb_errRT_BASE_SubstR(EG_DATATYPE, 50080, __func__, "must be a subtype of HbgiBoxed", HB_ERR_ARGS_BASEPARAMS);
        return NULL;
    }*/

    self = hb_itemNew(hbgi_hb_clsInst(hb_type));
    gtype = (GType)hb_itemGetNLL(hb_arrayGetItemPtr(self, HBGI_IVAR_GTYPE));

    //hb_arraySetNLL(self, HBGI_IVAR_GTYPE, gtype); - already done in hbgi_hb_clsInst?
    hbg_boxed_set_ptr (self, boxed);
    hb_arraySetL(self, HBGI_IVAR_FREE_ON_DEALLOC, free_on_dealloc);

    /*
    self->size = 0;
    self->slice_allocated = FALSE;
    */

    return self;
}
