/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_CALLBACKS
#define HBGI_CALLBACKS

#include <girepository.h>

gboolean
_hbgi_scan_for_callbacks (GIFunctionInfo *function_info,
                          guint8        *callback_index,
                          guint8        *user_data_index,
                          guint8        *destroy_notify_index);

#endif
