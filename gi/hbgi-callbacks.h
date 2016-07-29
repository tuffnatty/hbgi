/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_CALLBACKS
#define HBGI_CALLBACKS

#include <girepository.h>

#include "hbgi-closure.h"

G_BEGIN_DECLS

void _hbgi_callback_notify_info_free (gpointer user_data);

HbGICClosure *_hbgi_destroy_notify_create (void);

gboolean
_hbgi_scan_for_callbacks (GIFunctionInfo *function_info,
                          guint8        *callback_index,
                          guint8        *user_data_index,
                          guint8        *destroy_notify_index);

gboolean _hbgi_create_callback (GIFunctionInfo *self,
                                gboolean       is_method,
                                gboolean       is_constructor,
                                int            n_args,
                                guint8         callback_index,
                                guint8         user_data_index,
                                guint8         destroy_notify_index,
                                HbGICClosure **closure_out);

G_END_DECLS

#endif
