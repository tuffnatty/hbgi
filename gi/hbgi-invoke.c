/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-invoke.c from pygobject
 * library:
 *
 * Copyright (C) 2005-2009 Johan Dahlin <johan@gnome.org>
 *
 *   pygi-invoke.c: main invocation function
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
#include <hbapicls.h>
#include <hbapierr.h>
#include <hbapiitm.h>
#include <hbstack.h>

#include "hbgihb.h"

#include <glib.h>
#include <girepository.h>

#include "hbgi.h"
#include "hbgi-argument.h"
#include "hbgi-boxed.h"
#include "hbgi-callbacks.h"
#include "hbgi-closure.h"
#include "hbgi-invoke.h"

#include "hbgobject.h"

struct invocation_state
{
    gboolean is_method;
    gboolean is_constructor;

    gsize n_args;
    gsize n_in_args;
    gsize n_out_args;
    gsize n_backup_args;
    gsize n_hb_args;
    gsize n_aux_in_args;
    gsize n_aux_out_args;
    gsize n_return_values;

    guint8 callback_index;
    guint8 user_data_index;
    guint8 destroy_notify_index;
    HbGICClosure *closure;

    glong error_arg_pos;

    GIArgInfo **arg_infos;
    GITypeInfo **arg_type_infos;
    GITypeInfo *return_type_info;
    GITypeTag return_type_tag;

    GIArgument **args;
    gboolean *args_is_auxiliary;

    GIArgument *in_args;
    GIArgument *out_args;
    GIArgument *out_values;
    GIArgument *backup_args;
    GIArgument return_arg;

    PHB_ITEM return_value;

    GType      implementor_gtype;

    /* hack to avoid treating C arrays as GArrays during free
     * due to overly complicated array handling
     * this will be removed when the new invoke branch is merged
     */
    gboolean c_arrays_are_wrapped;
};

static gboolean
_initialize_invocation_state (struct invocation_state *state,
                              GIFunctionInfo *info)
{
    if (g_base_info_get_type (info) == GI_INFO_TYPE_FUNCTION) {
        GIFunctionInfoFlags flags = g_function_info_get_flags (info);

        state->is_method = (flags & GI_FUNCTION_IS_METHOD) != 0;
        state->is_constructor = (flags & GI_FUNCTION_IS_CONSTRUCTOR) != 0;
        state->implementor_gtype = 0;
    } else {
        hb_errRT_BASE_SubstR( HBGI_ERR, 50005, __func__, "invoke not implemented for non-function callables", HB_ERR_ARGS_BASEPARAMS );
        /*PHB_ITEM obj;

        state->is_method = TRUE;
        state->is_constructor = FALSE;

        obj = PyDict_GetItemString (kwargs, "gtype");
        if (obj == NULL) {
            PyErr_SetString (PyExc_TypeError,
                             "need the GType of the implementor class");
            return FALSE;
        }

        state->implementor_gtype = pyg_type_from_object (obj);
        if (state->implementor_gtype == 0)
            return FALSE;*/
    }

    /* Count arguments. */
    state->n_args = g_callable_info_get_n_args ( (GICallableInfo *) info);
    state->n_in_args = 0;
    state->n_out_args = 0;
    state->n_backup_args = 0;
    state->n_aux_in_args = 0;
    state->n_aux_out_args = 0;

    /* Check the argument count. */
    state->n_hb_args = hb_pcount();

    state->error_arg_pos = -1;

    state->arg_infos = g_slice_alloc0 (sizeof (gpointer) * state->n_args);
    state->arg_type_infos = g_slice_alloc0 (sizeof (gpointer) * state->n_args);
    state->args_is_auxiliary = g_slice_alloc0 (sizeof (gboolean) * state->n_args);

    state->return_value = NULL;
    state->closure = NULL;
    state->return_type_info = NULL;
    state->args = NULL;
    state->in_args = NULL;
    state->out_args = NULL;
    state->out_values = NULL;
    state->backup_args = NULL;

    /* HACK: this gets marked FALSE whenever a C array in the args is
     *       not wrapped by a GArray
     */
    state->c_arrays_are_wrapped = TRUE;

    return TRUE;
}

static gboolean
_prepare_invocation_state (struct invocation_state *state,
                           GIFunctionInfo *function_info)
{
    gsize i;

    if (!_hbgi_scan_for_callbacks (function_info,
                                   &state->callback_index, &state->user_data_index,
                                   &state->destroy_notify_index))
        return FALSE;

    if (state->callback_index != G_MAXUINT8) {

        if (!_hbgi_create_callback (function_info,
                                    state->is_method,
                                    state->is_constructor,
                                    state->n_args,
                                    state->callback_index,
                                    state->user_data_index,
                                    state->destroy_notify_index, &state->closure))
            return FALSE;

        state->args_is_auxiliary[state->callback_index] = FALSE;
        if (state->destroy_notify_index != G_MAXUINT8) {
            state->args_is_auxiliary[state->destroy_notify_index] = TRUE;
            state->n_aux_in_args += 1;
        }
    }

    if (state->is_method) {
        /* The first argument is the instance. */
        state->n_in_args += 1;
    }

    /* We do a first (well, second) pass here over the function to scan for special cases.
     * This is currently array+length combinations, GError and GValue.
     */
    for (i = 0; i < state->n_args; i++) {
        GIDirection direction;
        GITransfer transfer;
        GITypeTag arg_type_tag;

        state->arg_infos[i] = g_callable_info_get_arg ( (GICallableInfo *) function_info,
                                                        i);

        state->arg_type_infos[i] = g_arg_info_get_type (state->arg_infos[i]);

        direction = g_arg_info_get_direction (state->arg_infos[i]);
        transfer = g_arg_info_get_ownership_transfer (state->arg_infos[i]);
        arg_type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

        if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT) {
            state->n_in_args += 1;
        }
        if (direction == GI_DIRECTION_INOUT) {
            state->n_backup_args += 1;
        }
        if (direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT) {
            state->n_out_args += 1;
        }

        switch (arg_type_tag) {
            case GI_TYPE_TAG_ARRAY:
            {
                gint length_arg_pos;

                length_arg_pos = g_type_info_get_array_length (state->arg_type_infos[i]);

                if (length_arg_pos < 0) {
                    break;
                }

                /* For array lengths, we're going to delete the length argument;
                 * so remove the extra backup we just added above */
                if (direction == GI_DIRECTION_INOUT) {
                    state->n_backup_args -= 1;
                }

                g_assert (length_arg_pos < (gint)state->n_args);
                state->args_is_auxiliary[length_arg_pos] = TRUE;

                if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT) {
                    state->n_aux_in_args += 1;
                }
                if (direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT) {
                    state->n_aux_out_args += 1;
                }

                break;
            }
            case GI_TYPE_TAG_ERROR:
                g_warn_if_fail (state->error_arg_pos < 0);
                state->error_arg_pos = i;
                break;
            default:
                break;
        }
    }

    state->return_type_info = g_callable_info_get_return_type ( (GICallableInfo *) function_info);
    state->return_type_tag = g_type_info_get_tag (state->return_type_info);

    if (state->return_type_tag == GI_TYPE_TAG_ARRAY) {
        gint length_arg_pos;
        length_arg_pos = g_type_info_get_array_length (state->return_type_info);

        if (length_arg_pos >= 0) {
            g_assert (length_arg_pos < (gint)state->n_args);
            state->args_is_auxiliary[length_arg_pos] = TRUE;
            state->n_aux_out_args += 1;
        }
    }

    state->n_return_values = state->n_out_args - state->n_aux_out_args;
    if (state->return_type_tag != GI_TYPE_TAG_VOID) {
        state->n_return_values += 1;
    }

    {
        gsize n_hb_args_expected;
        gsize hb_args_pos;

        n_hb_args_expected = state->n_in_args
                             - (state->is_method ? 1 : 0)
                             //+ (state->is_constructor ? 1 : 0)
                             - state->n_aux_in_args
                             - (state->error_arg_pos >= 0 ? 1 : 0);

        if (state->n_hb_args != n_hb_args_expected) {
            gchar *msg = g_strdup_printf("%s() takes exactly %zd argument(s) (%zd given)",
                                         g_base_info_get_name((GIBaseInfo *)function_info),
                                         n_hb_args_expected, state->n_hb_args);
            hb_errRT_BASE_SubstR( HBGI_ERR, 50007, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
            g_free(msg);
            return FALSE;
        }

        /* Check argument typestate-> */
        hb_args_pos = 1;
        /*if (state->is_constructor || state->is_method) {
            py_args_pos += 1;
        }*/

        for (i = 0; i < state->n_args; i++) {
            GIDirection direction;
            GITypeTag type_tag;
            PHB_ITEM hb_arg;
            gint retval;
            gboolean allow_none;

            direction = g_arg_info_get_direction (state->arg_infos[i]);
            type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

            if (direction == GI_DIRECTION_OUT
                    || state->args_is_auxiliary[i]
                    || type_tag == GI_TYPE_TAG_ERROR) {
                continue;
            }

            g_assert (hb_args_pos <= state->n_hb_args);
            hb_arg = hb_itemParam(hb_args_pos);

            allow_none = g_arg_info_may_be_null (state->arg_infos[i]);

            retval = _hbgi_g_type_info_check_object (state->arg_type_infos[i],
                                                     hb_arg,
                                                     allow_none);
            hb_itemRelease(hb_arg);

            if (retval < 0) {
                return FALSE;
            } else if (!retval) {
                gchar *msg = g_strdup_printf("argument %zd", hb_args_pos);
                hb_errRT_BASE_SubstR( HBGI_ERR, 50008, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                g_free(msg);
                return FALSE;
            }

            hb_args_pos++;
        }

        g_assert (hb_args_pos == state->n_hb_args + 1);
    }

    state->args = g_slice_alloc0 (sizeof (gpointer) * state->n_args);
    state->in_args = g_slice_alloc0 (sizeof (GIArgument) * state->n_in_args);
    state->out_args = g_slice_alloc0 (sizeof (GIArgument) * state->n_out_args);
    state->out_values = g_slice_alloc0 (sizeof (GIArgument) * state->n_out_args);
    state->backup_args = g_slice_alloc0 (sizeof (GIArgument) * state->n_backup_args);

    /* Bind args so we can use an unique index. */
    {
        gsize in_args_pos;
        gsize out_args_pos;

        in_args_pos = state->is_method ? 1 : 0;
        out_args_pos = 0;

        for (i = 0; i < state->n_args; i++) {
            GIDirection direction;
            GIBaseInfo *info;
            gboolean is_caller_allocates;

            direction = g_arg_info_get_direction (state->arg_infos[i]);
            is_caller_allocates = g_arg_info_is_caller_allocates (state->arg_infos[i]);

            switch (direction) {
                case GI_DIRECTION_IN:
                    g_assert (in_args_pos < state->n_in_args);
                    state->args[i] = &state->in_args[in_args_pos];
                    in_args_pos += 1;
                    break;
                case GI_DIRECTION_INOUT:
                    g_assert (in_args_pos < state->n_in_args);
                    g_assert (out_args_pos < state->n_out_args);

                    state->in_args[in_args_pos].v_pointer = &state->out_values[out_args_pos];
                    in_args_pos += 1;
                case GI_DIRECTION_OUT:
                    g_assert (out_args_pos < state->n_out_args);

                    /* caller allocates only applies to structures but GI has
                     * no way to denote that yet, so we only use caller allocates
                     * if we see  a structure
                     */
                    if (is_caller_allocates) {
                        GITypeTag type_tag;

                        is_caller_allocates = FALSE;
                        type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

                        if (type_tag  == GI_TYPE_TAG_INTERFACE) {
                            GIInfoType info_type;

                            info = g_type_info_get_interface (state->arg_type_infos[i]);
                            g_assert (info != NULL);
                            info_type = g_base_info_get_type (info);

                            if (info_type == GI_INFO_TYPE_STRUCT)
                                is_caller_allocates = TRUE;
                        }
                    }

                    if (is_caller_allocates) {
                        /* if caller allocates only use one level of indirection */
                        state->out_args[out_args_pos].v_pointer = NULL;
                        state->args[i] = &state->out_args[out_args_pos];
                        if (g_struct_info_is_foreign((GIStructInfo *) info) ) {
                            hb_errRT_BASE_SubstR( HBGI_ERR, 50009, __func__, "foreign structs not implemented yet", HB_ERR_ARGS_BASEPARAMS );
                            /*PyObject *foreign_struct =
                                pygi_struct_foreign_convert_from_g_argument(info, NULL);

                            pygi_struct_foreign_convert_to_g_argument(
                                foreign_struct,
                                info,
                                GI_TRANSFER_EVERYTHING,
                                state->args[i]);

                            Py_DECREF(foreign_struct);*/
                        } else if (g_type_is_a (g_registered_type_info_get_g_type (info), G_TYPE_BOXED)) {
                            state->args[i]->v_pointer = _hbgi_boxed_alloc (info, NULL);
                        } else {
                            gssize size = g_struct_info_get_size ( (GIStructInfo *) info);
                            state->args[i]->v_pointer = g_malloc0 (size);
                        }
                    } else {
                        state->out_args[out_args_pos].v_pointer = &state->out_values[out_args_pos];
                        state->out_values[out_args_pos].v_pointer = NULL;
                        state->args[i] = &state->out_values[out_args_pos];
                    }

                    out_args_pos += 1;
            }
        }

        g_assert (in_args_pos == state->n_in_args);
        g_assert (out_args_pos == state->n_out_args);
    }

    /* Convert the input arguments. */
    {
        gsize hb_args_pos;
        gsize backup_args_pos;

        hb_args_pos = 1;
        backup_args_pos = 0;

        if (state->is_constructor) {
            /* Skip the first argument. */
            //py_args_pos += 1;
        } else if (state->is_method) {
            /* Get the instance. */
            GIBaseInfo *container_info;
            GIInfoType container_info_type;
            PHB_ITEM hb_arg;
            gint check_val;

            container_info = g_base_info_get_container (function_info);
            container_info_type = g_base_info_get_type (container_info);

            hb_arg = hb_stackSelfItem();

            /* In python 2 python takes care of checking the type
             * of the self instance.  In python 3 it does not
             * so we have to check it here
             */
            /*check_val = _pygi_g_type_interface_check_object(container_info,
                                                            py_arg);
            if (check_val < 0) {
                return FALSE;
            } else if (!check_val) {
                _PyGI_ERROR_PREFIX ("instance: ");
                return FALSE;
            }*/

            switch (container_info_type) {
                case GI_INFO_TYPE_UNION:
                case GI_INFO_TYPE_STRUCT:
                {
                    GType type;

                    type = g_registered_type_info_get_g_type ( (GIRegisteredTypeInfo *) container_info);

                    if (g_type_is_a (type, G_TYPE_BOXED)) {
                        g_assert (state->n_in_args > 0);
                        state->in_args[0].v_pointer = hbg_boxed_get(hb_arg, void);
                    } else if (g_struct_info_is_foreign (container_info)) {
                        hb_errRT_BASE_SubstR( HBGI_ERR, 50009, __func__, "foreign structs not implemented yet", HB_ERR_ARGS_BASEPARAMS );
                        /*PyObject *result;
                        result = pygi_struct_foreign_convert_to_g_argument (
                                     py_arg, container_info,
                                     GI_TRANSFER_NOTHING,
                                     &state->in_args[0]);*/
                    } else if (g_type_is_a (type, G_TYPE_POINTER) || type == G_TYPE_NONE) {
                        g_assert (state->n_in_args > 0);
                        hb_errRT_BASE_SubstR( HBGI_ERR, 50011, __func__, "pointers/voids not implemented yet", HB_ERR_ARGS_BASEPARAMS );
                        //state->in_args[0].v_pointer = pyg_pointer_get (py_arg, void);
                    } else {
                        gchar *msg = g_strdup_printf("unable to convert an instance of '%s'", g_type_name(type));
                        hb_errRT_BASE_SubstR( HBGI_ERR, 50012, msg, g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                        return FALSE;
                    }

                    break;
                }
                case GI_INFO_TYPE_OBJECT:
                case GI_INFO_TYPE_INTERFACE:
                    g_assert (state->n_in_args > 0);
                    state->in_args[0].v_pointer = hbgobject_get (hb_arg);
                    if (!state->in_args[0].v_pointer) {
                        hb_errRT_BASE_SubstR( HBGI_ERR, 50099, "You must instantiate an object using a constructor before invoking a non-class method on it", g_base_info_get_namespace((GIBaseInfo *)function_info), HB_ERR_ARGS_BASEPARAMS );
                        return FALSE;
                    }
                    break;
                default:
                    /* Other types don't have methods. */
                    g_assert_not_reached();
            }
        }

        for (i = 0; i < state->n_args; i++) {
            GIDirection direction;

            if (i == state->callback_index) {
                if (state->closure)
                    state->args[i]->v_pointer = state->closure->closure;
                else
                    /* Some callbacks params accept NULL */
                    state->args[i]->v_pointer = NULL;
                hb_args_pos++;
                continue;
            } else if (i == state->user_data_index) {
                state->args[i]->v_pointer = state->closure;
                hb_args_pos++;
                continue;
            } else if (i == state->destroy_notify_index) {
                if (state->closure) {
                    /* No need to clean up if the callback is NULL */
                    HbGICClosure *destroy_notify = _hbgi_destroy_notify_create();
                    state->args[i]->v_pointer = destroy_notify->closure;
                }
                continue;
            }

            if (state->args_is_auxiliary[i]) {
                continue;
            }

            direction = g_arg_info_get_direction (state->arg_infos[i]);

            if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT) {
                PHB_ITEM hb_arg;
                GITypeTag arg_type_tag;
                GITransfer transfer;

                arg_type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

                if (arg_type_tag == GI_TYPE_TAG_ERROR) {
                    GError **error;

                    error = g_slice_new (GError *);
                    *error = NULL;

                    state->args[i]->v_pointer = error;
                    continue;
                }

                transfer = g_arg_info_get_ownership_transfer (state->arg_infos[i]);

                g_assert (hb_args_pos <= state->n_hb_args);
                hb_arg = hb_itemParam(hb_args_pos);

                *state->args[i] = _hbgi_argument_from_object (hb_arg, state->arg_type_infos[i], transfer);

                hb_itemRelease(hb_arg);

#if 0
                if (PyErr_Occurred()) {
                    /* TODO: release previous input arguments. */
                    return FALSE;
                }
#endif

                if (direction == GI_DIRECTION_INOUT) {
                    /* We need to keep a copy of the argument to be able to release it later. */
                    g_assert (backup_args_pos < state->n_backup_args);
                    state->backup_args[backup_args_pos] = *state->args[i];
                    backup_args_pos += 1;
                }

                if (arg_type_tag == GI_TYPE_TAG_ARRAY) {
                    GArray *array;
                    gssize length_arg_pos;

                    array = state->args[i]->v_pointer;

                    length_arg_pos = g_type_info_get_array_length (state->arg_type_infos[i]);
                    if (length_arg_pos >= 0) {
                        int len = 0;
                        /* Set the auxiliary argument holding the length. */
                        if (array)
                            len = array->len;

                        state->args[length_arg_pos]->v_size = len;
                    }

                    /* Get rid of the GArray. */
                    if ( (array != NULL) &&
                            (g_type_info_get_array_type (state->arg_type_infos[i]) == GI_ARRAY_TYPE_C)) {
                        state->args[i]->v_pointer = array->data;

                        /* HACK: We have unwrapped a C array so
                         *       set the state to reflect this.
                         *       If there is an error between now
                         *       and when we rewrap the array
                         *       we will leak C arrays due to
                         *       being in an inconsitant state.
                         *       e.g. for interfaces with more
                         *       than one C array argument, an
                         *       error may occure when not all
                         *       C arrays have been rewrapped.
                         *       This will be removed once the invoke
                         *       rewrite branch is merged.
                         */
                        state->c_arrays_are_wrapped = FALSE;
                        if (direction != GI_DIRECTION_INOUT || transfer != GI_TRANSFER_NOTHING) {
                            /* The array hasn't been referenced anywhere, so free it to avoid losing memory. */
                            g_array_free (array, FALSE);
                        }
                    }
                }

                hb_args_pos++;
            }
        }

        g_assert (hb_args_pos == state->n_hb_args + 1);
        g_assert (backup_args_pos == state->n_backup_args);
    }

    return TRUE;
}

static gboolean
_invoke_function (struct invocation_state *state,
                  GICallableInfo *callable_info)
{
    GError *error;
    gint retval;

    error = NULL;

    //pyg_begin_allow_threads;
    if (g_base_info_get_type (callable_info) == GI_INFO_TYPE_FUNCTION) {
        retval = g_function_info_invoke ( (GIFunctionInfo *) callable_info,
                                          state->in_args,
                                          state->n_in_args,
                                          state->out_args,
                                          state->n_out_args,
                                          &state->return_arg,
                                          &error);
    } else {
        retval = g_vfunc_info_invoke ( (GIVFuncInfo *) callable_info,
                                       state->implementor_gtype,
                                       state->in_args,
                                       state->n_in_args,
                                       state->out_args,
                                       state->n_out_args,
                                       &state->return_arg,
                                       &error);
    }
    //pyg_end_allow_threads;

    if (!retval) {
        gchar *msg = g_strdup_printf("GError check not implemented yet, but GError message is '%s'", error->message);
        hb_errRT_BASE_SubstR( HBGI_ERR, 50014, g_base_info_get_name((GIBaseInfo *)callable_info), msg, HB_ERR_ARGS_BASEPARAMS );
        g_free(msg);
        //pyglib_error_check(&error);

        /* TODO: release input arguments. */

        return FALSE;
    }

    if (state->error_arg_pos >= 0) {
        GError **error;

        error = state->args[state->error_arg_pos]->v_pointer;

        gchar *msg = g_strdup_printf("GError check not implemented yet, but GError message is '%s'", (*error)->message);
        hb_errRT_BASE_SubstR( HBGI_ERR, 50014, g_base_info_get_name((GIBaseInfo *)callable_info), msg, HB_ERR_ARGS_BASEPARAMS );
        g_free(msg);
        //if (pyglib_error_check(error)) {
            /* TODO: release input arguments. */

            return FALSE;
        //}
    }

    return TRUE;
}

static gboolean
_process_invocation_state (struct invocation_state *state,
                           GIFunctionInfo *function_info)
{
    gsize i;

    /* Convert the return value. */
    if (state->is_constructor) {
        const char *hb_type_name;
        HB_USHORT hb_type;
        GIBaseInfo *info;
        GIInfoType info_type;
        GITransfer transfer;

        if (state->return_arg.v_pointer == NULL) {
            hb_errRT_BASE_SubstR( HBGI_ERR, 50015, __func__, "constructor returned NULL", HB_ERR_ARGS_BASEPARAMS );
            return FALSE;
        }

        hb_type = hb_objGetClass(hb_stackSelfItem());
        hb_type_name = hb_objGetClsName(hb_stackSelfItem());

        info = g_type_info_get_interface (state->return_type_info);
        g_assert (info != NULL);

        info_type = g_base_info_get_type (info);

        transfer = g_callable_info_get_caller_owns ( (GICallableInfo *) function_info);

        switch (info_type) {
            case GI_INFO_TYPE_UNION:
            case GI_INFO_TYPE_STRUCT:
            {
                GType type;

                type = g_registered_type_info_get_g_type ( (GIRegisteredTypeInfo *) info);

                if (g_struct_info_is_foreign (info)) {
                    hb_errRT_BASE_SubstR( HBGI_ERR, 50009, __func__, "foreign structs not implemented yet", HB_ERR_ARGS_BASEPARAMS );
                    /*state->return_value =
                        pygi_struct_foreign_convert_from_g_argument (
                            info, state->return_arg.v_pointer);*/
                } else if (g_type_is_a (type, G_TYPE_BOXED)) {
                    g_warn_if_fail (transfer == GI_TRANSFER_EVERYTHING);
                    state->return_value = _hbgi_boxed_new (hb_type, state->return_arg.v_pointer, transfer == GI_TRANSFER_EVERYTHING);
                } else if (g_type_is_a (type, G_TYPE_POINTER) || type == G_TYPE_NONE) {
                    if (transfer != GI_TRANSFER_NOTHING)
                        g_warning ("Return argument in %s returns a struct "
                                   "with a transfer mode of \"full\" "
                                   "Transfer mode should be set to None for "
                                   "struct types as there is no way to free "
                                   "them safely.  Ignoring transfer mode "
                                   "to prevent a potential invalid free. "
                                   "This may cause a leak in your application.",
                                   g_base_info_get_name ( (GIBaseInfo *) function_info) );

                    hb_errRT_BASE_SubstR( HBGI_ERR, 50011, __func__, "pointers/voids not implemented yet", HB_ERR_ARGS_BASEPARAMS );
                    //state->return_value = _pygi_struct_new (py_type, state->return_arg.v_pointer, FALSE);
                } else {
                    gchar *msg = g_strdup_printf("cannot create '%s' instances", hb_type_name);
                    hb_errRT_BASE_SubstR( HBGI_ERR, 50016, g_base_info_get_name((GIBaseInfo *)function_info), msg, HB_ERR_ARGS_BASEPARAMS );
                    g_free(msg);
                    g_base_info_unref (info);
                    return FALSE;
                }

                break;
            }
            case GI_INFO_TYPE_OBJECT:
                if (state->return_arg.v_pointer == NULL) {
                    hb_errRT_BASE_SubstR( HBGI_ERR, 50015, __func__, "constructor returned NULL", HB_ERR_ARGS_BASEPARAMS );
                    break;
                }
                state->return_value = hbgobject_new (state->return_arg.v_pointer);
                if (transfer == GI_TRANSFER_EVERYTHING) {
                    /* The new wrapper increased the reference count, so decrease it. */
                    g_object_unref (state->return_arg.v_pointer);
                }
                if (state->is_constructor && G_IS_INITIALLY_UNOWNED (state->return_arg.v_pointer)) {
                    /* GInitiallyUnowned constructors always end up with one extra reference, so decrease it. */
                    g_object_unref (state->return_arg.v_pointer);
                }
                break;
            default:
                /* Other types don't have neither methods nor constructors. */
                g_assert_not_reached();
        }

        g_base_info_unref (info);

        if (state->return_value == NULL) {
            /* TODO: release arguments. */
            return FALSE;
        }
    } else {
        GITransfer transfer;

        if ( (state->return_type_tag == GI_TYPE_TAG_ARRAY) &&
                (g_type_info_get_array_type (state->return_type_info) == GI_ARRAY_TYPE_C)) {
            /* Create a #GArray. */
            state->return_arg.v_pointer = _hbgi_argument_to_array (&state->return_arg, state->args, state->return_type_info);
        }

        transfer = g_callable_info_get_caller_owns ( (GICallableInfo *) function_info);

        state->return_value = _hbgi_argument_to_object (&state->return_arg, state->return_type_info, transfer);
        if (state->return_value == NULL) {
            /* TODO: release argument. */
            return FALSE;
        }

        _hbgi_argument_release (&state->return_arg, state->return_type_info, transfer, GI_DIRECTION_OUT);

        if (state->return_type_tag == GI_TYPE_TAG_ARRAY
                && transfer == GI_TRANSFER_NOTHING) {
            /* We created a #GArray, so free it. */
            state->return_arg.v_pointer = g_array_free (state->return_arg.v_pointer, FALSE);
        }
    }

    /* Convert output arguments and release arguments. */
    {
        gsize return_values_pos;

        return_values_pos = 1;

        if (state->n_return_values > 1) {
            /* Return a tuple. */
            PHB_ITEM return_values;

            return_values = hb_itemArrayNew(state->n_return_values);
            if (return_values == NULL) {
                /* TODO: release arguments. */
                return FALSE;
            }

            if (state->return_type_tag == GI_TYPE_TAG_VOID) {
                /* The current return value is None. */
                hb_itemRelease(state->return_value);
            } else {
                /* Put the return value first. */
                g_assert (state->return_value != NULL);
                hb_arraySet(return_values, return_values_pos, state->return_value);
                return_values_pos += 1;
            }

            state->return_value = return_values;
        }

        for (i = 0; i < state->n_args; i++) {
            GIDirection direction;
            GITypeTag type_tag;
            GITransfer transfer;

            if (state->args_is_auxiliary[i]) {
                /* Auxiliary arguments are handled at the same time as their relatives. */
                continue;
            }

            direction = g_arg_info_get_direction (state->arg_infos[i]);
            transfer = g_arg_info_get_ownership_transfer (state->arg_infos[i]);

            type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

            if ( (type_tag == GI_TYPE_TAG_ARRAY) &&
                    (g_type_info_get_array_type (state->arg_type_infos[i]) == GI_ARRAY_TYPE_C) &&
                    (direction != GI_DIRECTION_IN || transfer == GI_TRANSFER_NOTHING)) {
                /* Create a #GArray. */
                state->args[i]->v_pointer = _hbgi_argument_to_array (state->args[i], state->args, state->arg_type_infos[i]);
            }

            if (direction == GI_DIRECTION_INOUT || direction == GI_DIRECTION_OUT) {
                /* Convert the argument. */
                PHB_ITEM obj;

                /* If we created it, deallocate when it goes out of scope
                 * otherwise it is unsafe to deallocate random structures
                 * we are given
                 */
                if (type_tag == GI_TYPE_TAG_INTERFACE) {
                    GIBaseInfo *info;
                    GIInfoType info_type;
                    GType type;

                    info = g_type_info_get_interface (state->arg_type_infos[i]);
                    g_assert (info != NULL);
                    info_type = g_base_info_get_type (info);
                    type = g_registered_type_info_get_g_type ( (GIRegisteredTypeInfo *) info);

                    if ( (info_type == GI_INFO_TYPE_STRUCT) &&
                             !g_struct_info_is_foreign((GIStructInfo *) info) &&
                             !g_type_is_a (type, G_TYPE_BOXED)) {
                        if (g_arg_info_is_caller_allocates (state->arg_infos[i])) {
                            transfer = GI_TRANSFER_EVERYTHING;
                        } else if (transfer == GI_TRANSFER_EVERYTHING) {
                            transfer = GI_TRANSFER_NOTHING;
                            g_warning ("Out argument %ld in %s returns a struct "
                                       "with a transfer mode of \"full\". "
                                       "Transfer mode should be set to \"none\" for "
                                       "struct type returns as there is no way to free "
                                       "them safely.  Ignoring transfer mode "
                                       "to prevent a potential invalid free. "
                                       "This may cause a leak in your application.",
                                       i, g_base_info_get_name ( (GIBaseInfo *) function_info) );
                        }
                    }
                }

                obj = _hbgi_argument_to_object (state->args[i], state->arg_type_infos[i], transfer);
                if (obj == NULL) {
                    /* TODO: release arguments. */
                    return FALSE;
                }

                g_assert (return_values_pos <= state->n_return_values);

                if (state->n_return_values > 1) {
                    hb_arraySet(state->return_value, return_values_pos, obj);
                } else {
                    /* The current return value is None. */
                    hb_itemRelease(state->return_value);
                    state->return_value = obj;
                }

                return_values_pos += 1;
            }

        }

        /* HACK: We rewrapped any C arrays above in a GArray so they are ok to
         *       free as GArrays.  We will always leak C arrays if there is
         *       an error before we reach this state as there is no easy way
         *       to know which arrays were wrapped if there are more than one.
         *       This will be removed with better array handling once merge
         *       the invoke rewrite branch.
         */
        state->c_arrays_are_wrapped = TRUE;
        g_assert (state->n_return_values <= 1 || return_values_pos == state->n_return_values + 1);
    }

    return TRUE;
}

static void
_free_invocation_state (struct invocation_state *state)
{
    gsize i;
    gsize backup_args_pos;

    if (state->return_type_info != NULL) {
        g_base_info_unref ( (GIBaseInfo *) state->return_type_info);
    }

    if (state->closure != NULL) {
        if (state->closure->scope == GI_SCOPE_TYPE_CALL)
            _hbgi_invoke_closure_free (state->closure);
    }

    /* release all arguments. */
    backup_args_pos = 0;
    for (i = 0; i < state->n_args; i++) {

        if (state->args_is_auxiliary[i]) {
            /* Auxiliary arguments are not released. */
            continue;
        }

        if (state->arg_infos[i] != NULL
            && state->arg_type_infos[i] != NULL) {
            GIDirection direction;
            GITypeTag type_tag;
            GITransfer transfer;

            direction = g_arg_info_get_direction (state->arg_infos[i]);
            transfer = g_arg_info_get_ownership_transfer (state->arg_infos[i]);

            /* Release the argument. */
            if (direction == GI_DIRECTION_INOUT) {
                if (state->args != NULL) {
                    _hbgi_argument_release (&state->backup_args[backup_args_pos],
                                            state->arg_type_infos[i],
                                            transfer, GI_DIRECTION_IN);
                }
                backup_args_pos += 1;
            }
            if (state->args != NULL && state->args[i] != NULL) {
                type_tag = g_type_info_get_tag (state->arg_type_infos[i]);

                if (type_tag == GI_TYPE_TAG_ARRAY &&
                        (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT) &&
                        (g_type_info_get_array_type (state->arg_type_infos[i]) == GI_ARRAY_TYPE_C) &&
                        !state->c_arrays_are_wrapped) {
                    /* HACK: Noop - we are in an inconsitant state due to
                     *       complex array handler so leak any C arrays
                     *       as we don't know if we can free them safely.
                     *       This will be removed when we merge the
                     *       invoke rewrite branch.
                     */
                } else {
                    _hbgi_argument_release (state->args[i], state->arg_type_infos[i],
                                            transfer, direction);
                }

                if (type_tag == GI_TYPE_TAG_ARRAY
                    && (direction != GI_DIRECTION_IN && transfer == GI_TRANSFER_NOTHING)) {
                    /* We created an *out* #GArray and it has not been released above, so free it. */
                    state->args[i]->v_pointer = g_array_free (state->args[i]->v_pointer, FALSE);
                }
            }

        }

        if (state->arg_type_infos[i] != NULL)
            g_base_info_unref ( (GIBaseInfo *) state->arg_type_infos[i]);
        if (state->arg_infos[i] != NULL)
            g_base_info_unref ( (GIBaseInfo *) state->arg_infos[i]);
    }
    g_assert (backup_args_pos == state->n_backup_args);

    g_slice_free1 (sizeof (gpointer) * state->n_args, state->arg_infos);
    g_slice_free1 (sizeof (gpointer) * state->n_args, state->arg_type_infos);
    g_slice_free1 (sizeof (gboolean) * state->n_args, state->args_is_auxiliary);

    if (state->args != NULL) {
        g_slice_free1 (sizeof (gpointer) * state->n_args, state->args);
    }

    if (state->in_args != NULL) {
        g_slice_free1 (sizeof (GIArgument) * state->n_in_args, state->in_args);
    }

    if (state->out_args != NULL) {
        g_slice_free1 (sizeof (GIArgument) * state->n_out_args, state->out_args);
    }

    if (state->out_values != NULL) {
        g_slice_free1 (sizeof (GIArgument) * state->n_out_args, state->out_values);
    }

    if (state->backup_args != NULL) {
        g_slice_free1 (sizeof (GIArgument) * state->n_backup_args, state->backup_args);
    }

    if (0/*PyErr_Occurred()*/) {
        hb_itemClear(hb_stackSelfItem());
    }
}


void
_wrap_g_callable_info_invoke(GIBaseInfo *info)
{
    struct invocation_state state = { 0, };

    if (!_initialize_invocation_state (&state, info)) {
        _free_invocation_state (&state);
        hb_ret();
        return;
    }

    if (!_prepare_invocation_state (&state, info)) {
        _free_invocation_state (&state);
        hb_ret();
        return;
    }

    if (!_invoke_function (&state, info)) {
        _free_invocation_state (&state);
        hb_ret();
        return;
    }

    if (!_process_invocation_state (&state, info)) {
        _free_invocation_state (&state);
        hb_ret();
        return;
    }

    _free_invocation_state (&state);

    hbgi_hb_itemReturnRelease(state.return_value);
}

