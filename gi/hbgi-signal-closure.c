/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-signal-closure.c from pygobject
 * library:
 *
 * Copyright (c) 2011  Laszlo Pandy <lpandy@src.gnome.org>
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

#include <glib.h>
#include <girepository.h>

#include <hbapierr.h>
#include <hbstack.h>
#define _HB_API_INTERNAL_ /* for hb_vmEval() */
#include <hbvm.h>

#include "hbgtype.h"

#include "hbgi-argument.h"
#include "hbgi-signal-closure.h"

/* Copied from glib */
static void
canonicalize_key (gchar *key)
{
    gchar *p;

    for (p = key; *p != 0; p++)
    {
        gchar c = *p;

        if (c != '-' &&
            (c < '0' || c > '9') &&
            (c < 'A' || c > 'Z') &&
            (c < 'a' || c > 'z'))
                *p = '-';
    }
}

static GISignalInfo *
_hbgi_lookup_signal_from_g_type (GType g_type,
                                 const gchar *signal_name)
{
    GIRepository *repository;
    GIBaseInfo *info;
    gssize n_infos;
    gssize i;
    GType parent;

    repository = g_irepository_get_default();
    info = g_irepository_find_by_gtype (repository, g_type);
    if (info != NULL) {
        n_infos = g_object_info_get_n_signals ( (GIObjectInfo *) info);
        for (i = 0; i < n_infos; i++) {
            GISignalInfo *signal_info;

            signal_info = g_object_info_get_signal ( (GIObjectInfo *) info, i);
            g_assert (info != NULL);

            if (strcmp (signal_name, g_base_info_get_name (signal_info)) == 0) {
                g_base_info_unref (info);
                return signal_info;
            }

            g_base_info_unref (signal_info);
        }

        g_base_info_unref (info);
    }

    parent = g_type_parent (g_type);
    if (parent > 0)
        return _hbgi_lookup_signal_from_g_type (parent, signal_name);

    return NULL;
}

static void
hbgi_signal_closure_invalidate(gpointer data,
                               GClosure *closure)
{
    HbGClosure *pc = (HbGClosure *)closure;
    //PyGILState_STATE state;
    HB_SYMBOL_UNUSED(data);

    //state = PyGILState_Ensure();
    hb_itemRelease(pc->callback);
    hb_itemRelease(pc->extra_args);
    hb_itemRelease(pc->swap_data);
    //PyGILState_Release(state);

    pc->callback = NULL;
    pc->extra_args = NULL;
    pc->swap_data = NULL;

    g_base_info_unref (((HbGISignalClosure *) pc)->signal_info);
    ((HbGISignalClosure *) pc)->signal_info = NULL;
}

static void
hbgi_signal_closure_marshal(GClosure *closure,
                            GValue *return_value,
                            guint n_param_values,
                            const GValue *param_values,
                            gpointer invocation_hint,
                            gpointer marshal_data)
{
    //PyGILState_STATE state;
    HbGClosure *pc = (HbGClosure *)closure;
    guint i;
    GISignalInfo *signal_info;
    gint n_sig_info_args;
    gint sig_info_highest_arg;
    HB_SYMBOL_UNUSED(invocation_hint);
    HB_SYMBOL_UNUSED(marshal_data);

    //state = PyGILState_Ensure();

    signal_info = ((HbGISignalClosure *)closure)->signal_info;
    n_sig_info_args = g_callable_info_get_n_args(signal_info);
    /* the first argument to a signal callback is instance,
       but instance is not counted in the introspection data */
    sig_info_highest_arg = n_sig_info_args + 1;
    g_assert_cmpint(sig_info_highest_arg, ==, n_param_values);

    if (HB_IS_SYMBOL(pc->callback)) {
        hb_vmPushSymbol(pc->callback);
        hb_vmPushNil();
    } else {
        hb_vmPushEvalSym();
        hb_vmPush(pc->callback);
    }
    for (i = 0; i < n_param_values; i++) {
        /* swap in a different initial data for connect_object() */
        if (i == 0 && G_CCLOSURE_SWAP_DATA(closure)) {
            g_return_if_fail(pc->swap_data != NULL);
            hb_vmPush(pc->swap_data);
        } else if (i == 0) {
            hb_vmPush(hbg_value_as_hbitem(&param_values[i], FALSE));
        } else if (i < sig_info_highest_arg) {
            GIArgInfo arg_info;
            GITypeInfo type_info;
            GITransfer transfer;
            GIArgument arg = { 0, };
            PHB_ITEM item = NULL;

            g_callable_info_load_arg(signal_info, i - 1, &arg_info);
            g_arg_info_load_type(&arg_info, &type_info);
            transfer = g_arg_info_get_ownership_transfer(&arg_info);

            arg = _hbgi_argument_from_g_value(&param_values[i], &type_info);
            item = _hbgi_argument_to_object(&arg, &type_info, transfer);

            if (item == NULL) {
                return;
                //goto out;
            }
            hb_vmPush(item);
        }
    }
    /* params passed to function may have extra arguments */
    if (pc->extra_args) {
        for (i = 0; i < hb_itemSize(pc->extra_args); i++) {
            hb_vmPush(hb_arrayGetItemPtr(pc->extra_args, i + 1));
        }
    }
    if (HB_IS_SYMBOL(pc->callback)) {
        hb_vmProc(n_param_values + hb_itemSize(pc->extra_args));
    } else {
        hb_vmEval(n_param_values + hb_itemSize(pc->extra_args));
    }
    /*if (ret == NULL) {
        if (pc->exception_handler)
            pc->exception_handler(return_value, n_param_values, param_values);
        else
            PyErr_Print();
        goto out;
    }*/

    if (return_value && hbg_value_from_hbitem(return_value, hb_stackReturnItem()) != 0) {
        hb_errRT_BASE_SubstR(EG_DATATYPE, 50501, "can't convert return value to desired type", "hbgi", HB_ERR_ARGS_BASEPARAMS);

        /*if (pc->exception_handler)
            pc->exception_handler(return_value, n_param_values, param_values);
        else
            PyErr_Print();*/
    }

 //out:
    //PyGILState_Release(state);
}

GClosure *
hbgi_signal_closure_new_real (PHB_ITEM instance,
                              const gchar *sig_name,
                              PHB_ITEM callback,
                              PHB_ITEM extra_args,
                              PHB_ITEM swap_data)
{
    GClosure *closure = NULL;
    HbGISignalClosure *hbgi_closure = NULL;
    GType g_type;
    GISignalInfo *signal_info = NULL;
    char *signal_name = g_strdup (sig_name);

    g_return_val_if_fail(callback != NULL, NULL);

    canonicalize_key(signal_name);

    g_type = hbg_type_from_object(instance);
    signal_info = _hbgi_lookup_signal_from_g_type (g_type, signal_name);

    if (signal_info == NULL)
        goto out;

    closure = g_closure_new_simple(sizeof(HbGISignalClosure), NULL);
    g_closure_add_invalidate_notifier(closure, NULL, hbgi_signal_closure_invalidate);
    g_closure_set_marshal(closure, hbgi_signal_closure_marshal);

    hbgi_closure = (HbGISignalClosure *)closure;

    hbgi_closure->signal_info = signal_info;
    hbgi_closure->hbg_closure.callback = hb_itemNew(callback);

    if (extra_args != NULL && !HB_IS_NIL(extra_args)) {
        /*if (!PyTuple_Check(extra_args)) {
            PyObject *tmp = PyTuple_New(1);
            PyTuple_SetItem(tmp, 0, extra_args);
            extra_args = tmp;
        }*/
        hbgi_closure->hbg_closure.extra_args = hb_itemNew(extra_args);
    }
    if (swap_data) {
        hbgi_closure->hbg_closure.swap_data = hb_itemNew(swap_data);
        closure->derivative_flag = TRUE;
    }

out:
    g_free (signal_name);

    return closure;
}
