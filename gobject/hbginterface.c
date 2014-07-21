/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pyginterface.c from pygobject
 * library:
 *
 * pygtk- Python bindings for the GTK toolkit.
 * Copyright (C) 1998-2003  James Henstridge
 *               2004-2008  Johan Dahlin
 *   pyginterface.c: wrapper for the gobject library.
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

#include <hbapicls.h>
#include <hbapiitm.h>

#include <glib.h>

#include "hbginterface.h"
#include "hbgobject.h"
#include "hbgtype.h"

#define HBGINTERFACE_IVAR_GTYPE 1
#define HBGINTERFACE_IVAR_COUNT 1

GQuark hbginterface_type_key;
GQuark hbginterface_info_key;

HB_USHORT HbGInterface_Type = 0;

#if 0
static int
pyg_interface_init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    if (!PyArg_ParseTuple(args, ":GInterface.__init__"))
	return -1;

    g_snprintf(buf, sizeof(buf), "%s can not be constructed",
	       Py_TYPE(self)->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

static void
pyg_interface_free(PyObject *op)
{
    PyObject_FREE(op);
}

/**
 * pyg_register_interface:
 * @dict: a module dictionary.
 * @class_name: the class name for the wrapper class.
 * @gtype: the GType of the interface.
 * @type: the wrapper class for the interface.
 *
 * Registers a Python class as the wrapper for a GInterface.  As a
 * convenience it will also place a reference to the wrapper class in
 * the provided module dictionary.
 */
void
pyg_register_interface(PyObject *dict, const gchar *class_name,
                       GType gtype, PyTypeObject *type)
{
    PyObject *o;

    Py_TYPE(type) = &PyType_Type;
    type->tp_base = &PyGInterface_Type;

    if (PyType_Ready(type) < 0) {
        g_warning("could not ready `%s'", type->tp_name);
        return;
    }

    if (gtype) {
        o = pyg_type_wrapper_new(gtype);
        PyDict_SetItemString(type->tp_dict, "__gtype__", o);
        Py_DECREF(o);
    }

    g_type_set_qdata(gtype, pyginterface_type_key, type);
    
    PyDict_SetItemString(dict, (char *)class_name, (PyObject *)type);
    
}

void
pyg_register_interface_info(GType gtype, const GInterfaceInfo *info)
{
    g_type_set_qdata(gtype, pyginterface_info_key, (gpointer) info);
}

const GInterfaceInfo *
pyg_lookup_interface_info(GType gtype)
{
    return g_type_get_qdata(gtype, pyginterface_info_key);
}
#endif

void
hbgobject_interface_register_types(void)
{
  //PHB_ITEM pInit = hb_itemPutC(NULL, hbg_object_descr_doc_get());

  hbginterface_type_key = g_quark_from_static_string("HbGInterface::type");
  hbginterface_info_key = g_quark_from_static_string("HbGInterface::info");

  HbGInterface_Type = hb_clsCreate(HBGINTERFACE_IVAR_COUNT, "HbGInterface");
  /*hbgi_hb_clsAddData(HbGInterface_Type, "__doc__", HB_OO_MSG_ACCESS, 0, HBGINTERFACE_IVAR_DOC, pInit);
  hb_itemRelease(pInit);*/

  HBGOBJECT_REGISTER_GTYPE(HbGInterface_Type, "GInterface", G_TYPE_INTERFACE)
}
