/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygi-type.c from pygobject
 * library:
 *
 * Copyright (C) 2009 Simon van der Linden <svdlinden@src.gnome.org>
 *
 *   pygi-type.c: helpers to lookup Python wrappers from GType and GIBaseInfo.
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
#include <hbapicls.h>
#include <hbapiitm.h>
#include <hbstack.h>
#include <hbvm.h>

#include <girepository.h>

#include "hbgtype.h"

#include "hbgi-type.h"


static PHB_DYNS s_dynsHBGI_IMPORT = NULL;

HB_USHORT
_hbgi_type_import_by_name (const char *namespace_,
                           const char *name)
{
    if (!s_dynsHBGI_IMPORT) {
       s_dynsHBGI_IMPORT = hb_dynsymFind("HBGI_IMPORT");
    }
    if (!s_dynsHBGI_IMPORT) {
       return 1;
    }
    hb_vmPushDynSym(s_dynsHBGI_IMPORT);
    hb_vmPushNil();
    hb_vmPushString(namespace_, strlen(namespace_));
    hb_vmProc(1);

    return hb_objGetClass(hb_objSendMsg(hb_stackReturnItem(), name, 0));
}

HB_USHORT
hbgi_type_import_by_g_type_real(GType g_type)
{
    GIRepository *repository;
    GIBaseInfo *info;
    HB_USHORT type;

    repository = g_irepository_get_default();

    info = g_irepository_find_by_gtype (repository, g_type);
    if (info == NULL) {
        return 0;
    }

    type = _hbgi_type_import_by_gi_info(info);
    g_base_info_unref (info);

    return type;
}

HB_USHORT
_hbgi_type_import_by_gi_info(GIBaseInfo *info)
{
    return _hbgi_type_import_by_name (g_base_info_get_namespace (info),
                                      g_base_info_get_name (info));
}

HB_USHORT
_hbgi_type_get_from_g_type (GType g_type)
{
    PHB_ITEM hb_g_type;
    HB_USHORT type;

    hb_g_type = hbg_type_wrapper_new (g_type);
    if (hb_g_type == NULL) {
        return 0;
    }

    /*type = hb_itemGetNL(hb_objSendMsg(hb_g_type, "pytype", 0));
    if (type == 0)*/ {
        type = hbgi_type_import_by_g_type_real (g_type);
    }

    hb_itemRelease(hb_g_type);

    return type;
}
