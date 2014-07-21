/*
 * hbgi source code
 * Extensions for Harbour C Class API
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 *
 */

#include <hbapi.h>
#include <hbapicls.h>
#include <hbapiitm.h>
#include <hbinit.h>
#include <hbstack.h>
#include <hbvm.h>

#include "hbgihb.h"

static void
hbgihb_init(void *dummy);

HB_FUNC_EXTERN(__CLSADDMSG);
HB_FUNC_EXTERN(__CLSNEW);
static PHB_DYNS s_pDyns__CLSADDMSG;
static PHB_DYNS s_pDyns__CLSNEW;

static void
hbgi_hb_clsAddMsgNative(HB_USHORT uiClass, const char *szMessage, HB_USHORT uiType, HB_USHORT uiScope, PHB_ITEM pFuncOrOffset, PHB_ITEM pInit)
{
   PHB_ITEM pInitCopy;
   HB_BOOL allocated = !pInit;
   pInitCopy = allocated ? hb_itemNew(NULL) : pInit;
   if (!s_pDyns__CLSADDMSG) {
      hbgihb_init(NULL);
   }
   hb_vmPushDynSym(s_pDyns__CLSADDMSG);
   hb_vmPushNil();
   hb_vmPushNumInt(uiClass);
   hb_vmPushString(szMessage, strlen(szMessage));
   hb_vmPush(pFuncOrOffset);
   hb_vmPushNumInt(uiType);
   hb_vmPush(pInitCopy);
   hb_vmPushNumInt(uiScope);
   hb_vmProc(6);
   if (allocated)
   {
      hb_itemRelease(pInitCopy);
   }
}


HB_BOOL
hbgi_hb_clsAddData(HB_USHORT uiClass, const char *szMessage, HB_USHORT uiType, HB_USHORT uiScope, HB_UINT uiOffset, PHB_ITEM pInit)
{
   PHB_ITEM pOffset = hb_itemPutNI(NULL, uiOffset);

   hbgi_hb_clsAddMsgNative(uiClass, szMessage, uiType, uiScope, pOffset, pInit);
   hb_itemRelease(pOffset);
   return HB_TRUE;
}

HB_BOOL
hbgi_hb_clsAddMsg(HB_USHORT uiClass, const char *szMessage, HB_USHORT uiType, HB_USHORT uiScope, PHB_FUNC pFuncPtr, PHB_ITEM pInit)
{
   PHB_SYMB pExecSym = hb_symbolNew("");
   PHB_ITEM pFuncItem;

   pExecSym->value.pFunPtr = pFuncPtr;
   pFuncItem = hb_itemPutSymbol(NULL, pExecSym);
   hbgi_hb_clsAddMsgNative(uiClass, szMessage, uiType, uiScope, pFuncItem, pInit);
   hb_itemRelease(pFuncItem);
   return HB_TRUE;
}


PHB_ITEM
hbgi_hb_clsGetInitData(HB_USHORT uiClass, HB_SIZE uiIndex)
{
   PHB_ITEM instance = hbgi_hb_clsInst(uiClass);
   return hb_arrayGetItemPtr(instance, uiIndex);
}


PHB_ITEM
hbgi_hb_clsInst(HB_USHORT uiClass)
{
   hb_clsAssociate(uiClass);
   return hb_stackReturnItem();
}


HB_USHORT
hbgi_hb_clsNew(const char *szClassName, int nDatas, PHB_ITEM pSuperArray)
{
   if (!s_pDyns__CLSNEW) {
      hbgihb_init(NULL);
   }
   hb_vmPushDynSym(s_pDyns__CLSNEW);
   hb_vmPushNil();
   hb_vmPushString(szClassName, strlen(szClassName));
   hb_vmPushNumInt(nDatas);
   if (!pSuperArray)
   {
      hb_vmPushNil();
   }
   else
   {
      hb_vmPush(pSuperArray);
   }
   hb_vmProc(3);
   return hb_itemGetNI(hb_stackReturnItem());
}

void
hbgi_hb_itemReturnRelease(PHB_ITEM pItem)
{
   if (pItem != hb_stackReturnItem())
   {
      hb_itemReturnRelease(pItem);
   }
}


PHB_ITEM
hbgi_hb_paramSlice(int start, int end)
{
   PHB_ITEM args = hb_itemArrayNew(end - start);
   int i;

   for (i = start; i < end; i++)
   {
      hb_arraySet(args, i - start + 1, hb_itemParam(i));
   }
   return args;
}


static void
hbgihb_init(void *dummy)
{
   HB_SYMBOL_UNUSED(dummy);

   if (!s_pDyns__CLSNEW) {
      PHB_SYMB pExecSym = hb_symbolNew("__CLSADDMSG");
      pExecSym->value.pFunPtr = HB_FUNCNAME(__CLSADDMSG);
      s_pDyns__CLSADDMSG = hb_dynsymNew(pExecSym);
      pExecSym = hb_symbolNew("__CLSNEW");
      pExecSym->value.pFunPtr = HB_FUNCNAME(__CLSNEW);
      s_pDyns__CLSNEW = hb_dynsymNew(pExecSym);
   }
}

HB_CALL_ON_STARTUP_BEGIN( _hbgihb_init_ )
   hb_vmAtInit( hbgihb_init, NULL );
HB_CALL_ON_STARTUP_END( _hbgihb_init_ )

#if defined( HB_PRAGMA_STARTUP )
   #pragma startup _hbgihb_init_
#elif defined( HB_DATASEG_STARTUP )
   #define HB_DATASEG_BODY  HB_DATASEG_FUNC( _hbgihb_init_ )
   #include "hbiniseg.h"
#endif
