/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_INFO_H
#define HBGI_INFO_H

#include <girepository.h>

G_BEGIN_DECLS


/* Private */
gchar *
_hbgi_g_base_info_get_fullname (GIBaseInfo *info);

gsize
_hbgi_g_type_info_size (GITypeInfo *type_info);

G_END_DECLS

#endif
