/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_BOXED_H
#define HBGI_BOXED_H

PHB_ITEM _hbgi_boxed_new (HB_USHORT hb_type,
                            gpointer      boxed,
                            gboolean      free_on_dealloc);

void * _hbgi_boxed_alloc (GIBaseInfo *info,
                          gsize *size);

#endif /* HBGI_BOXED_H */
