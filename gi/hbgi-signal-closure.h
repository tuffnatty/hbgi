/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_SIGNAL_CLOSURE_H
#define HBGI_SIGNAL_CLOSURE_H

#include <girepository.h>

#include "hbgobject.h"

typedef struct _HbGISignalClosure
{
   HbGClosure hbg_closure;
   GISignalInfo *signal_info;
} HbGISignalClosure;

GClosure *
hbgi_signal_closure_new_real (PHB_ITEM instance,
                              const gchar *sig_name,
                              PHB_ITEM callback,
                              PHB_ITEM extra_args,
                              PHB_ITEM swap_data);

#endif
