/*
 * hbgi source code
 * Core code
 *
 * Copyright 2016 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_CLOSURE_H
#define HBGI_CLOSURE_H

#include <hbapi.h>
#include <girffi.h>
#include <ffi.h>

G_BEGIN_DECLS


/* Private */

typedef struct _HbGICClosure
{
    GICallableInfo *info;
    PHB_ITEM function;

    ffi_closure *closure;
    ffi_cif cif;

    GIScopeType scope;

    PHB_ITEM user_data;
} HbGICClosure;

void _hbgi_invoke_closure_free (gpointer user_data);

HbGICClosure* _hbgi_make_native_closure (GICallableInfo* info,
                                         GIScopeType scope,
                                         PHB_ITEM function,
                                         gpointer user_data);

G_END_DECLS

#endif /* HBGI_CLOSURE_H */
