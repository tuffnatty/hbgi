/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_ARGUMENT_H
#define HBGI_ARGUMENT_H

#include <hbapi.h>

#include <girepository.h>

G_BEGIN_DECLS


/* Private */
gint _hbgi_g_type_interface_check_object (GIBaseInfo *info,
                                          PHB_ITEM object);
gint
_hbgi_g_type_info_check_object (GITypeInfo *type_info,
                                PHB_ITEM object,
                                gboolean   allow_none);
#if 0
gint _pygi_g_registered_type_info_check_object (GIRegisteredTypeInfo *info,
                                                gboolean              is_instance,
                                                PyObject             *object);
#endif


GArray* _hbgi_argument_to_array (GIArgument  *arg,
                                 GIArgument  *args[],
                                 GITypeInfo *type_info);

GIArgument _hbgi_argument_from_object(PHB_ITEM object,
                                      GITypeInfo *type_info,
                                      GITransfer  transfer);

PHB_ITEM _hbgi_argument_to_object(GIArgument  *arg,
                                  GITypeInfo *type_info,
                                  GITransfer  transfer);

GIArgument
_hbgi_argument_from_g_value(const GValue *value,
                            GITypeInfo *type_info);

void _hbgi_argument_release (GIArgument   *arg,
                             GITypeInfo  *type_info,
                             GITransfer   transfer,
                             GIDirection  direction);

#if 0
void _pygi_argument_init (void);
#endif

G_END_DECLS

#endif
