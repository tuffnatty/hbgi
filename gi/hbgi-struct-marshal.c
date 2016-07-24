/*
 * hbgi source code
 * Core code
 *
 * Copyright 2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-struct-marshal.c from pygobject
 * library:
 *
 * Copyright (C) 2011 John (J5) Palmieri <johnp@redhat.com>
 * Copyright (C) 2014 Simon Feltman <sfeltman@gnome.org>
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
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <hbapi.h>
#include <hbapierr.h>

#include <girepository.h>

#include "hbgi.h"
#include "hbgi-boxed.h"
#include "hbgi-struct-marshal.h"
#include "hbgtype.h"

PHB_ITEM
hbgi_arg_struct_to_hb_marshal (GIArgument *arg,
                               GIInterfaceInfo *interface_info,
                               GType g_type,
                               HB_USHORT hb_type,
                               GITransfer transfer,
                               gboolean is_allocated,
                               gboolean is_foreign)
{
    PHB_ITEM object = NULL;

    if (arg->v_pointer == NULL) {
        return NULL;
    }

    if (g_type_is_a (g_type, G_TYPE_VALUE)) {
        object = hbg_value_as_hbitem(arg->v_pointer, FALSE);
    } else if (is_foreign) {
        hb_errRT_BASE_SubstR( HBGI_ERR, 50057, __func__, "foreign struct", HB_ERR_ARGS_BASEPARAMS );
        /*object = hbgi_struct_foreign_convert_from_g_argument (interface_info,
                                                              transfer,
                                                              arg->v_pointer);
        */
    } else if (g_type_is_a (g_type, G_TYPE_BOXED)) {
        if (hb_type) {
            /* Force a boxed copy if we are not transfered ownership and the
             * memory is not caller allocated. */
            object = _hbgi_boxed_new(hb_type,
                                      arg->v_pointer,
                                      transfer == GI_TRANSFER_EVERYTHING);
        }
    } else if (g_type_is_a (g_type, G_TYPE_POINTER)) {
        hb_errRT_BASE_SubstR( HBGI_ERR, 50061, __func__, "pointer", HB_ERR_ARGS_BASEPARAMS );
        /*PyObject *py_type;
        if (py_type == NULL ||
                !PyType_IsSubtype ((PyTypeObject *) py_type, &PyGIStruct_Type)) {
            g_warn_if_fail (transfer == GI_TRANSFER_NOTHING);
            py_obj = pyg_pointer_new (g_type, arg->v_pointer);
        } else {
            py_obj = _pygi_struct_new ( (PyTypeObject *) py_type,
                                       arg->v_pointer,
                                       transfer == GI_TRANSFER_EVERYTHING);
        }
        */
    } else if (g_type_is_a (g_type, G_TYPE_VARIANT)) {
        hb_errRT_BASE_SubstR( HBGI_ERR, 50061, __func__, "variant", HB_ERR_ARGS_BASEPARAMS );
        /* Note: sink the variant (add a ref) only if we are not transfered ownership.
         * GLib.Variant overrides __del__ which will then call "g_variant_unref" for
         * cleanup in either case. */
#if 0
        if (py_type) {
            if (transfer == GI_TRANSFER_NOTHING) {
                g_variant_ref_sink (arg->v_pointer);
            }
            py_obj = _pygi_struct_new ((PyTypeObject *) py_type,
                                       arg->v_pointer,
                                       FALSE);
        }
#endif
    } else if (g_type == G_TYPE_NONE) {
        hb_errRT_BASE_SubstR( HBGI_ERR, 50050, __func__, "none", HB_ERR_ARGS_BASEPARAMS );
#if 0
        if (py_type) {
            py_obj = _pygi_struct_new ((PyTypeObject *) py_type,
                                       arg->v_pointer,
                                       transfer == GI_TRANSFER_EVERYTHING || is_allocated);
        }
#endif
    } else {
#if 0
        PyErr_Format (PyExc_NotImplementedError,
                      "structure type '%s' is not supported yet",
                      g_type_name (g_type));
#else
        hb_errRT_BASE_SubstR( HBGI_ERR, 50050, __func__, "boxed/struct/union", HB_ERR_ARGS_BASEPARAMS );
#endif
    }

    return object;
}

