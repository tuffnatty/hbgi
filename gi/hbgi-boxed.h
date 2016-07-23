/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 */

#ifndef HBGI_BOXED_H
#define HBGI_BOXED_H

PHB_ITEM
_hbgi_boxed_new (HB_USHORT hb_type,
                 gpointer      boxed,
                 gboolean      copy_boxed,
                 gsize         allocated_slice);

#endif /* HBGI_BOXED_H */
