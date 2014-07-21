/*
 * hbgi source code
 * Extensions for Harbour C Class API
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 *
 */

#ifndef HBGIHB_H
#define HBGIHB_H

#include <hbapi.h>

HB_BOOL
hbgi_hb_clsAddData(HB_USHORT uiClass, const char *szMessage, HB_USHORT uiType, HB_USHORT uiScope, HB_UINT uiOffset, PHB_ITEM pInit);

HB_BOOL
hbgi_hb_clsAddMsg(HB_USHORT uiClass, const char *szMessage, HB_USHORT uiType, HB_USHORT uiScope, PHB_FUNC pFuncPtr, PHB_ITEM pInit);

PHB_ITEM
hbgi_hb_clsGetInitData(HB_USHORT uiClass, HB_SIZE uiIndex);

PHB_ITEM
hbgi_hb_clsInst(HB_USHORT uiClass);

HB_USHORT
hbgi_hb_clsNew(const char *szClassName, int nDatas, PHB_ITEM pSuperArray);

void
hbgi_hb_itemReturnRelease(PHB_ITEM pItem);

PHB_ITEM
hbgi_hb_paramSlice(int start, int end);

#endif
