/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-callbacks.c from pygobject
 * library:
 *
 *   pygi-callbacks.c: PyGI C Callback Functions and Helpers
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

#include <string.h>

#include <hbapi.h>
#include <hbapierr.h>
#include <hbapiitm.h>

#include "hbgi.h"
#include "hbgi-callbacks.h"
#include "hbgi-closure.h"

static HbGICClosure *global_destroy_notify;

static void
_hbgi_destroy_notify_callback_closure (ffi_cif *cif,
                                       void *result,
                                       void **args,
                                       void *data)
{
    HbGICClosure *info = * (void**) (args[0]);
    HB_SYMBOL_UNUSED(cif);
    HB_SYMBOL_UNUSED(result);
    HB_SYMBOL_UNUSED(data);

    g_assert (info);

    _hbgi_invoke_closure_free (info);
}


HbGICClosure*
_hbgi_destroy_notify_create (void)
{
    if (!global_destroy_notify) {

        HbGICClosure *destroy_notify = g_slice_new0 (HbGICClosure);

        g_assert (destroy_notify);

        GIBaseInfo* glib_destroy_notify = g_irepository_find_by_name (NULL, "GLib", "DestroyNotify");
        g_assert (glib_destroy_notify != NULL);
        g_assert (g_base_info_get_type (glib_destroy_notify) == GI_INFO_TYPE_CALLBACK);

        destroy_notify->closure = g_callable_info_prepare_closure ( (GICallableInfo*) glib_destroy_notify,
                                                                    &destroy_notify->cif,
                                                                    _hbgi_destroy_notify_callback_closure,
                                                                    NULL);

        global_destroy_notify = destroy_notify;
    }

    return global_destroy_notify;
}


gboolean
_hbgi_scan_for_callbacks (GIFunctionInfo *function_info,
                          guint8        *callback_index,
                          guint8        *user_data_index,
                          guint8        *destroy_notify_index)
{
    guint i, n_args;

    *callback_index = G_MAXUINT8;
    *user_data_index = G_MAXUINT8;
    *destroy_notify_index = G_MAXUINT8;

    n_args = g_callable_info_get_n_args ( (GICallableInfo *) function_info);
    for (i = 0; i < n_args; i++) {
        GIDirection direction;
        GIArgInfo *arg_info;
        GITypeInfo *type_info;
        guint8 destroy, closure;
        GITypeTag type_tag;

        arg_info = g_callable_info_get_arg ( (GICallableInfo*) function_info, i);
        type_info = g_arg_info_get_type (arg_info);
        type_tag = g_type_info_get_tag (type_info);

        if (type_tag == GI_TYPE_TAG_INTERFACE) {
            GIBaseInfo* interface_info;
            GIInfoType interface_type;

            interface_info = g_type_info_get_interface (type_info);
            interface_type = g_base_info_get_type (interface_info);
            if (interface_type == GI_INFO_TYPE_CALLBACK &&
                    ! (strcmp (g_base_info_get_namespace ( (GIBaseInfo*) interface_info), "GLib") == 0 &&
                       (strcmp (g_base_info_get_name ( (GIBaseInfo*) interface_info), "DestroyNotify") == 0 ||
                       (strcmp (g_base_info_get_name ( (GIBaseInfo*) interface_info), "FreeFunc") == 0)))) {
                if (*callback_index != G_MAXUINT8) {
                    gchar *msg = g_strdup_printf("Function %s:%s has multiple callbacks, not supported",
                                                 g_base_info_get_namespace((GIBaseInfo *)function_info),
                                                 g_base_info_get_name((GIBaseInfo *)function_info));
                    hb_errRT_BASE_SubstR( HBGI_ERR, 50007, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                    g_free(msg);
                    g_base_info_unref (interface_info);
                    return FALSE;
                }
                *callback_index = i;
            }
            g_base_info_unref (interface_info);
        }
        destroy = g_arg_info_get_destroy (arg_info);
        
        closure = g_arg_info_get_closure (arg_info);
        direction = g_arg_info_get_direction (arg_info);

        if (destroy > 0 && destroy < n_args) {
            if (*destroy_notify_index != G_MAXUINT8) {
                gchar *msg = g_strdup_printf("Function %s:%s has multiple GDestroyNotify, not supported",
                                             g_base_info_get_namespace((GIBaseInfo *)function_info),
                                             g_base_info_get_name((GIBaseInfo *)function_info));
                hb_errRT_BASE_SubstR( HBGI_ERR, 50007, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                g_free(msg);
                return FALSE;
            }
            *destroy_notify_index = destroy;
        }

        if (closure > 0 && closure < n_args) {
            if (*user_data_index != G_MAXUINT8) {
                gchar *msg = g_strdup_printf("Function %s:%s has multiple user_data arguments, not supported",
                                             g_base_info_get_namespace((GIBaseInfo *)function_info),
                                             g_base_info_get_name((GIBaseInfo *)function_info));
                hb_errRT_BASE_SubstR( HBGI_ERR, 50007, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                g_free(msg);
                return FALSE;
            }
            *user_data_index = closure;
        }

        g_base_info_unref ( (GIBaseInfo*) arg_info);
        g_base_info_unref ( (GIBaseInfo*) type_info);
    }

    return TRUE;
}

gboolean
_hbgi_create_callback (GIBaseInfo  *function_info,
                       gboolean       is_method,
                       gboolean       is_constructor,
                       int            n_args,
                       guint8         callback_index,
                       guint8         user_data_index,
                       guint8         destroy_notify_index,
                       HbGICClosure **closure_out)
{
    GIArgInfo *callback_arg;
    GITypeInfo *callback_type;
    GICallbackInfo *callback_info;
    GIScopeType scope;
    gboolean found_hb_function;
    PHB_ITEM hb_function;
    guint8 i, hb_argv_pos;
    PHB_ITEM hb_user_data;
    gboolean allow_none;
    HB_USHORT hb_argc = hb_itemPCount();
    HB_SYMBOL_UNUSED(is_method);
    HB_SYMBOL_UNUSED(is_constructor);
    HB_SYMBOL_UNUSED(destroy_notify_index);

    callback_arg = g_callable_info_get_arg ( (GICallableInfo*) function_info, callback_index);
    scope = g_arg_info_get_scope (callback_arg);
    allow_none = g_arg_info_may_be_null (callback_arg);

    callback_type = g_arg_info_get_type (callback_arg);
    g_assert (g_type_info_get_tag (callback_type) == GI_TYPE_TAG_INTERFACE);

    callback_info = (GICallbackInfo*) g_type_info_get_interface (callback_type);
    g_assert (g_base_info_get_type ( (GIBaseInfo*) callback_info) == GI_INFO_TYPE_CALLBACK);

    /* Find the Harbour function passed for the callback */
    found_hb_function = FALSE;
    hb_function = NULL;
    hb_user_data = NULL;

    hb_argv_pos = 1;

    for (i = 0; i < n_args && i < hb_argc; i++) {
        if (i == callback_index) {
            hb_function = hb_param(hb_argv_pos, HB_IT_ANY);
            /* if we allow none then set the closure to NULL and return */
            if (allow_none && (hb_function == NULL || HB_IS_NIL(hb_function))) {
                *closure_out = NULL;
                goto out;
            }
            found_hb_function = TRUE;
        } else if (i == user_data_index) {
            hb_user_data = hb_param(hb_argv_pos, HB_IT_ANY);
        }
        hb_argv_pos++;
    }

    if (!found_hb_function
            || (hb_function == NULL || !(HB_IS_SYMBOL(hb_function) || HB_IS_BLOCK(hb_function)))) {
        gchar *msg = g_strdup_printf("Error invoking %s:%s: Unexpected value "
                      "for argument '%s'",
                      g_base_info_get_namespace ( (GIBaseInfo*) function_info),
                      g_base_info_get_name ( (GIBaseInfo*) function_info),
                      g_base_info_get_name ( (GIBaseInfo*) callback_arg));
        g_base_info_unref ( (GIBaseInfo*) callback_info);
        g_base_info_unref ( (GIBaseInfo*) callback_type);
        hb_errRT_BASE_SubstR( HBGI_ERR, 50007, __func__, msg, HB_ERR_ARGS_BASEPARAMS );
        g_free(msg);
        return FALSE;
    }

    /** Now actually build the closure **/
    *closure_out = _hbgi_make_native_closure ( (GICallableInfo *) callback_info,
                                               g_arg_info_get_scope (callback_arg),
                                               hb_function,
                                               hb_user_data);
out:
    g_base_info_unref ( (GIBaseInfo*) callback_info);
    g_base_info_unref ( (GIBaseInfo*) callback_type);

    return TRUE;
}
