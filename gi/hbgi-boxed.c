/*
 * hbgi source code
 * Core code
 *
 * Copyright 2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-boxed.c from pygobject
 * library:
 *
 * Copyright (C) 2009 Simon van der Linden <svdlinden@src.gnome.org>
 *
 *   pygi-boxed.c: wrapper to handle registered structures.
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <hbapi.h>
#include <hbapiitm.h>

#include "hbgihb.h"

#include <girepository.h>

#include "hbgobject.h"
#include "hbgi.h"
#include "hbgi-boxed.h"

PHB_ITEM
_hbgi_boxed_new (HB_USHORT hb_type,
                 gpointer      boxed,
                 gboolean      copy_boxed,
                 gsize         allocated_slice)
{
    PHB_ITEM self;
    GType gtype;

    if (!boxed) {
        return NULL;
    }

    /*if (!hb_clsIsParent(hb_type, "HbgiBoxed")) {
        hb_errRT_BASE_SubstR(EG_DATATYPE, 50080, __func__, "must be a subtype of HbgiBoxed", HB_ERR_ARGS_BASEPARAMS);
        return NULL;
    }*/

    self = hb_itemNew(hbgi_hb_clsInst(hb_type));
    gtype = (GType)hb_itemGetNLL(hb_arrayGetItemPtr(self, HBGI_IVAR_GTYPE));

    /* Boxed objects with slice allocation means they come from caller allocated
     * out arguments. In this case copy_boxed does not make sense because we
     * already own the slice allocated memory and we should be receiving full
     * ownership transfer. */
    if (copy_boxed) {
        g_assert (allocated_slice == 0);
        boxed = g_boxed_copy (gtype, boxed);
    }

    /* We always free on dealloc because we always own the memory due to:
     *   1) copy_boxed == TRUE
     *   2) allocated_slice > 0
     *   3) otherwise the mode is assumed "transfer everything".
     */
    hb_arraySetL(self, HBGI_IVAR_FREE_ON_DEALLOC, TRUE);
    //hb_arraySetNLL(self, HBGI_IVAR_GTYPE, gtype); - already done in hbgi_hb_clsInst?
    hbg_boxed_set_ptr (self, boxed);

    /*if (allocated_slice > 0) {
        self->size = allocated_slice;
        self->slice_allocated = TRUE;
    } else {
        self->size = 0;
        self->slice_allocated = FALSE;
    }*/

    return self;
}
