/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_TYPE_H
#define HBGI_TYPE_H

#include <hbapi.h>

#include <glib-object.h>

#include <girepository.h>

HB_USHORT
hbgi_type_import_by_g_type_real(GType g_type);

HB_USHORT
_hbgi_type_import_by_gi_info (GIBaseInfo *info);

HB_USHORT
_hbgi_type_get_from_g_type (GType g_type);

#endif
