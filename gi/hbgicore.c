/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 */

#include <hbapi.h>
#include <hbapicls.h>
#include <hbapierr.h>
#include <hbapiitm.h>
#include <hbcomp.h>
#include <hbinit.h>
#include <hboo.ch>
#include <hbstack.h>
#include <hbvm.h>

#include "hbgi.h"
#include "hbgihb.h"
#include "hbgi-invoke.h"
#include "hbgi-signal-closure.h"
#include "hbgi-type.h"
#include "hbgobject.h"

#include <glib.h>
#include <girepository.h>


#define HBGI_IVAR_MODULE_NAMESPACE 1


static HB_USHORT s_uiModuleClass;
static GHashTable *s_hbgi_gtype_class_hash;
static PHB_DYNS s_pDyns__info__;
static PHB_DYNS s_pDyns__method_info__;
static GPtrArray *s_method_info_hashes = NULL;
static GSList *s_infos = NULL;
static GSList *s_strings = NULL;

/*
 * EXAMPLE:
 *   STATIC gtk := hbgi_import("Gtk", [version], [lazy=.F.])
 */

HB_FUNC( HBGI_IMPORT )
{
   const char *namespace = hb_parc(1);
   const char *version = hb_parc(2);
   gboolean lazy = hb_parl(3);
   GError *error = NULL;

   g_irepository_require(NULL, namespace, version, lazy ? G_IREPOSITORY_LOAD_FLAG_LAZY : 0, &error);
   if (error)
   {
      hb_errRT_BASE_SubstR( HBGI_ERR, 50001, error->message, namespace, HB_ERR_ARGS_BASEPARAMS );
   }
   else
   {
      PHB_ITEM pModuleObject = hbgi_hb_clsInst(s_uiModuleClass);

      hb_arraySetC(pModuleObject, HBGI_IVAR_MODULE_NAMESPACE, namespace);
      hbgi_hb_itemReturnRelease(pModuleObject);
   }
}


HB_FUNC_STATIC(hbgi_gtype_class_wrapper_invoke)
{
   PHB_ITEM pObject = hb_stackSelfItem();
   const char *szMethod = hb_itemGetSymbol(hb_stackBaseItem())->szName;
   GHashTable *method_info_hash = hb_itemGetPtr(hb_objGetVarPtr(pObject, s_pDyns__method_info__));
   GIFunctionInfo *meth = g_hash_table_lookup(method_info_hash, szMethod);

   if (!meth)
   {
      hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, NULL, szMethod, HB_ERR_ARGS_SELFPARAMS );
   }

   _wrap_g_callable_info_invoke((GICallableInfo *)meth);
}


static void
hbgi_gtype_class_wrapper_add_method(HB_USHORT uiClass, GIFunctionInfo *method, GHashTable *method_info_hash)
{
   const gchar *name = g_base_info_get_name((GIBaseInfo *)method);
   gchar *name_upper = g_ascii_strup(name, -1);
   s_strings = g_slist_prepend(s_strings, name_upper);
   hbgi_hb_clsAddMsg(uiClass, name, HB_OO_MSG_METHOD, 0, HB_FUNCNAME(hbgi_gtype_class_wrapper_invoke), NULL);
   g_hash_table_insert(method_info_hash, name_upper, method);
}

static void
hbgi_gtype_class_wrapper_add_methods(HB_USHORT uiClass, GIBaseInfo *info, GHashTable *method_info_hash)
{
   int i, n_methods;

   switch (g_base_info_get_type(info))
   {
      case GI_INFO_TYPE_ENUM:
      case GI_INFO_TYPE_FLAGS:
      {
         GIEnumInfo *einfo = (GIEnumInfo *)info;
         n_methods = g_enum_info_get_n_methods(einfo);
         for (i = 0; i < n_methods; i++)
         {
            GIFunctionInfo *meth = g_enum_info_get_method(einfo, i);
            s_infos = g_slist_prepend(s_infos, meth);
            hbgi_gtype_class_wrapper_add_method(uiClass, meth, method_info_hash);
         }
         break;
      }
      case GI_INFO_TYPE_OBJECT:
      {
         GIObjectInfo *oinfo = (GIObjectInfo *)info;
         n_methods = g_object_info_get_n_methods(oinfo);
         for (i = 0; i < n_methods; i++)
         {
            GIFunctionInfo *meth = g_object_info_get_method(oinfo, i);
            s_infos = g_slist_prepend(s_infos, meth);
            hbgi_gtype_class_wrapper_add_method(uiClass, meth, method_info_hash);
         }
         break;
      }
      case GI_INFO_TYPE_UNION:
      {
         GIUnionInfo *oinfo = (GIUnionInfo *)info;
         n_methods = g_union_info_get_n_methods(oinfo);
         for (i = 0; i < n_methods; i++)
         {
            GIFunctionInfo *meth = g_union_info_get_method(oinfo, i);
            s_infos = g_slist_prepend(s_infos, meth);
            hbgi_gtype_class_wrapper_add_method(uiClass, meth, method_info_hash);
         }
         break;
      }
      default:
      {
         gchar *msg = g_strdup_printf("dunno how to add methods for %s", g_base_info_get_name(info));
         hb_errRT_BASE_SubstR( HBGI_ERR, 50020, msg, g_base_info_get_namespace(info), HB_ERR_ARGS_BASEPARAMS );
         g_free(msg);
      }
   }
}


static HB_USHORT
get_parent_for_object(GIObjectInfo *info)
{
   GIObjectInfo *parent_info = g_object_info_get_parent(info);
   const gchar *namespace, *name;

   if (!parent_info)
   {
      return 0;
   }
   namespace = g_base_info_get_namespace((GIBaseInfo *)parent_info);
   name = g_base_info_get_name((GIBaseInfo *)parent_info);
   if (!strcmp(namespace, "GObject") && (!strcmp(name, "Object") || !strcmp(name, "InitiallyUnowned")))
   {
      g_base_info_unref(parent_info);
      return HbGObject_Type;
   }
   g_print("Importing namespace %s\n", namespace);
   hb_vmPushDynSym(hb_dynsymFind("HBGI_IMPORT"));
   hb_vmPushNil();
   hb_vmPushString(namespace, strlen(namespace));
   hb_vmProc(1);

   g_print("Importing class %s\n", name);
   hb_objSendMsg(hb_stackReturnItem(), name, 0);

   g_base_info_unref(parent_info);

   return hb_objGetClass(hb_stackReturnItem());
}

static void
_copy_hash_item(gpointer key, gpointer value, gpointer user_data)
{
   g_hash_table_insert(user_data, key, value);
}

static HB_USHORT
hbgi_gtype_class_wrapper_register(GType gtype, GIRegisteredTypeInfo *info)
{
   HB_USHORT uiClass, parent;
   PHB_ITEM pInit, bases = NULL;
   int n_datas = 0;
   GIEnumInfo *enum_info = (GIEnumInfo *)info;
   GIInfoType info_type = g_base_info_get_type(info);
   GHashTable *method_info_hash = NULL;

   switch (info_type)
   {
      case GI_INFO_TYPE_ENUM:
      case GI_INFO_TYPE_FLAGS:
         n_datas = g_enum_info_get_n_values(enum_info);
         bases = hb_itemArrayNew(0);
         break;
      case GI_INFO_TYPE_OBJECT:
         parent = get_parent_for_object((GIObjectInfo *)info);
         bases = hb_itemArrayNew(parent ? 1 : 0);
         method_info_hash = g_hash_table_new(g_str_hash, g_str_equal);
         if (parent)
         {
            if (parent < s_method_info_hashes->len && s_method_info_hashes->pdata[parent])
            {
               GHashTable *parent_hash = s_method_info_hashes->pdata[parent];
               g_hash_table_foreach(parent_hash, _copy_hash_item, method_info_hash);
            }
            hb_arraySetNI(bases, 1, parent);
         }
         break;
      case GI_INFO_TYPE_UNION:
         bases = hb_itemArrayNew(0);
         method_info_hash = g_hash_table_new(g_str_hash, g_str_equal);
         break;
      default:
         hb_errRT_BASE_SubstR( HBGI_ERR, 50019, "unknown GIInfoType", "hbgi", HB_ERR_ARGS_BASEPARAMS );
   }

   g_print("Registering Harbour class %s\n", g_type_name(gtype));
   uiClass = hbgi_hb_clsNew(g_type_name(gtype), HBGI_IVAR_COUNT + n_datas, bases);

   if (bases)
   {
      hb_itemRelease(bases);
   }
   g_ptr_array_set_size(s_method_info_hashes, uiClass + 1);
   s_method_info_hashes->pdata[uiClass] = method_info_hash;

   pInit = hb_itemPutNLL(NULL, gtype);
   hbgi_hb_clsAddData(uiClass, "__gtype__", HB_OO_MSG_ACCESS, 0, HBGI_IVAR_GTYPE, pInit);
   hb_itemPutPtr(pInit, info);
   s_infos = g_slist_prepend(s_infos, info);
   hbgi_hb_clsAddData(uiClass, "__info__", HB_OO_MSG_ACCESS, 0, HBGI_IVAR_INFO, pInit);
   hbgi_hb_clsAddData(uiClass, "__gobject__", HB_OO_MSG_ACCESS, 0, HBGI_IVAR_GOBJECT, NULL);
   hb_itemPutPtr(pInit, method_info_hash);
   hbgi_hb_clsAddData(uiClass, "__method_info__", HB_OO_MSG_ACCESS, 0, HBGI_IVAR_METHOD_INFO, pInit);
   //hbgi_hb_clsAddMsg(uiClass, "__getattr__", HB_OO_MSG_ONERROR, 0, hbgi_gtype_class_wrapper_getattr, NULL);
   if (info_type == GI_INFO_TYPE_ENUM || info_type == GI_INFO_TYPE_FLAGS)
   {
      int i;
      for (i = 0; i < n_datas; i++)
      {
         GIValueInfo *value_info = g_enum_info_get_value(enum_info, i);
         hb_itemPutNLL(pInit, g_value_info_get_value(value_info));
         hbgi_hb_clsAddData(uiClass, g_base_info_get_name((GIBaseInfo *)value_info), HB_OO_MSG_ACCESS, 0, HBGI_IVAR_COUNT + i + 1, pInit);
      }
   }
   hb_itemRelease(pInit);
   hbgi_gtype_class_wrapper_add_methods(uiClass, (GIBaseInfo *)info, method_info_hash);
   g_hash_table_insert(s_hbgi_gtype_class_hash, GUINT_TO_POINTER(gtype), GUINT_TO_POINTER(uiClass));
   return uiClass;
}


static HB_USHORT
hbgi_module_get_class_from_message(void)
{
   PHB_ITEM pObject = hb_stackSelfItem();
   PHB_SYMB pSymbol = hb_itemGetSymbol(hb_stackBaseItem());
   const char *namespace = hb_arrayGetCPtr(pObject, HBGI_IVAR_MODULE_NAMESPACE);
   GIBaseInfo *info;
   HB_USHORT pcount = hb_pcount();
   const char *szMethod = pSymbol->szName;
   HB_USHORT uiClass = 0;

   if (szMethod[0] == '_' && pcount >= 1)
   {
      szMethod++;
      //setattr = TRUE;
   }
   {
      int i, len = g_irepository_get_n_infos(NULL, namespace);
      for (i = 0; i < len; i++)
      {
         info = g_irepository_get_info(NULL, namespace, i);
         if (!g_ascii_strcasecmp(szMethod, g_base_info_get_name(info)))
         {
            break;
         }
         g_base_info_unref(info);
      }
      if (i == len)
      {
         info = NULL;
         hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, NULL, szMethod, HB_ERR_ARGS_SELFPARAMS );
      }
   }
   if (info && GI_IS_REGISTERED_TYPE_INFO(info))
   {
      GType type = g_registered_type_info_get_g_type((GIRegisteredTypeInfo *)info);

      if (type != G_TYPE_NONE)
      {
         uiClass = (HB_USHORT)GPOINTER_TO_UINT(g_hash_table_lookup(s_hbgi_gtype_class_hash, GUINT_TO_POINTER(type)));
      }
      if (!uiClass)
      {
         switch (g_base_info_get_type(info))
         {
            case GI_INFO_TYPE_ENUM:
            case GI_INFO_TYPE_FLAGS:
            case GI_INFO_TYPE_OBJECT:
            case GI_INFO_TYPE_UNION:
               uiClass = hbgi_gtype_class_wrapper_register(type, info);
               break;
            default:
            {
               gchar *msg = g_strdup_printf("dunno how to wrap %s", g_base_info_get_name(info));
               g_base_info_unref(info);
               hb_errRT_BASE_SubstR( HBGI_ERR, 50020, msg, g_base_info_get_namespace(info), HB_ERR_ARGS_BASEPARAMS );
               g_free(msg);
            }
         }
      }
   }
   return uiClass;
}


HB_FUNC_STATIC(hbgi_module_constructor_wrapper)
{
   HB_USHORT uiClass = hbgi_module_get_class_from_message();
   PHB_ITEM pInstance = hbgi_hb_clsInst(uiClass);
   hbgi_hb_itemReturnRelease(pInstance);
}


HB_FUNC_STATIC(hbgi_module_getattr)
{
   PHB_ITEM pObject = hb_stackSelfItem();
   PHB_SYMB pSymbol = hb_itemGetSymbol(hb_stackBaseItem());
   const char *szMethod = pSymbol->szName;
   gchar *szMethodLower;
   GIBaseInfo *info;
   HB_USHORT uiClass;

   g_print("Module method %s\n", szMethod);
   szMethodLower = g_ascii_strdown(szMethod, -1);
   info = g_irepository_find_by_name(NULL, hb_arrayGetCPtr(pObject, HBGI_IVAR_MODULE_NAMESPACE), szMethodLower);
   g_free(szMethodLower);
   if (info && g_base_info_get_type(info) == GI_INFO_TYPE_FUNCTION)
   {
      g_print("Invoking function %s\n", szMethod);
      _wrap_g_callable_info_invoke((GICallableInfo *)info);
   }
   else if ((uiClass = hbgi_module_get_class_from_message()) > 0)
   {
      HB_USHORT pcount = hb_pcount();
      int n;

      if (szMethod[0] == '_' && pcount >= 1)
      {
         szMethod++;
         //setattr = TRUE;
      }

      hbgi_hb_clsAddMsg(hb_objGetClass(pObject), szMethod, HB_OO_MSG_METHOD, 0, HB_FUNCNAME(hbgi_module_constructor_wrapper), NULL);
      hb_vmPushSymbol(pSymbol);
      hb_vmPush(pObject);
      for (n = 1; n <= pcount; n++)
      {
         hb_vmPush(hb_stackItemFromBase(n));
      }
      hb_vmSend(pcount);
   }
   if (info)
   {
      g_base_info_unref(info);
   }
}


static struct HbGI_API CAPI = {
     hbgi_type_import_by_g_type_real,
     hbgi_signal_closure_new_real,
};

static void
hbgi_exit(void *dummy)
{
   GSList *list;
   guint i;
   for (i = 0; i < s_method_info_hashes->len; i++)
   {
      GHashTable *hash = s_method_info_hashes->pdata[i];
      if (hash)
      {
         g_hash_table_unref(hash);
      }
   }
   g_ptr_array_free(s_method_info_hashes, TRUE);
   g_hash_table_unref(s_hbgi_gtype_class_hash);

   for (list = s_infos; list; list = g_slist_next(list))
   {
      g_base_info_unref(list->data);
   }
   g_slist_free(s_infos);

   for (list = s_strings; list; list = g_slist_next(list))
   {
      g_free(list->data);
   }
   g_slist_free(s_strings);
   g_print("\n\n******************HBGI_EXIT DONE ***********\n");
}


static void
hbgi_init(void *dummy)
{
   PHB_ITEM pMemVar = hb_itemPutC(NULL, "HBGI_API");
   PHB_ITEM pAPI = hb_itemPutPtr(NULL, &CAPI);
   HB_SYMBOL_UNUSED(dummy);

   s_uiModuleClass = hb_clsCreate(1, "HBGIMODULE");
   hbgi_hb_clsAddMsg(s_uiModuleClass, "__getattr__", HB_OO_MSG_ONERROR, 0, HB_FUNCNAME(hbgi_module_getattr), NULL);

   s_hbgi_gtype_class_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
   hb_memvarCreateFromItem(pMemVar, HB_VSCOMP_PUBLIC, pAPI);
   hb_itemRelease(pAPI);
   hb_itemRelease(pMemVar);
   s_pDyns__info__ = hb_dynsymGet("__info__");
   s_pDyns__method_info__ = hb_dynsymGet("__method_info__");
   s_method_info_hashes = g_ptr_array_new();

   hb_vmAtQuit( hbgi_exit, NULL );
}


HB_CALL_ON_STARTUP_BEGIN( _hbgi_init_ )
   hb_vmAtInit( hbgi_init, NULL );
HB_CALL_ON_STARTUP_END( _hbgi_init_ )

#if defined( HB_PRAGMA_STARTUP )
   #pragma startup _hbgi_init_
#elif defined( HB_DATASEG_STARTUP )
   #define HB_DATASEG_BODY  HB_DATASEG_FUNC( _hbgi_init_ )
   #include "hbiniseg.h"
#endif
