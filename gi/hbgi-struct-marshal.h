/*
 * hbgi source code
 * Core code
 *
 * Copyright 2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-struct-marshal.h from pygobject
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

#ifndef HBGI_STRUCT_MARSHAL_H
#define HBGI_STRUCT_MARSHAL_H

#include <hbapi.h>

#include <girepository.h>

G_BEGIN_DECLS

PHB_ITEM hbgi_arg_struct_to_hb_marshal       (GIArgument *arg,
                                              GIInterfaceInfo *interface_info,
                                              GType            g_type,
                                              HB_USHORT        hb_type,
                                              GITransfer       transfer,
                                              gboolean         is_allocated,
                                              gboolean         is_foreign);

G_END_DECLS

#endif /* HBGI_STRUCT_MARSHAL_H */
