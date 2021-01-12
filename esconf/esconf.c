/*
 *  esconf
 *
 *  Copyright (c) 2007 Brian Tarricone <bjt23@cornell.edu>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; version 2
 *  of the License ONLY.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "esconf.h"
#include "common/esconf-marshal.h"
#include "esconf-private.h"
#include "common/esconf-alias.h"

static guint esconf_refcnt = 0;

static GDBusConnection *gdbus = NULL;
static GDBusProxy *gproxy = NULL;
static GHashTable *named_structs = NULL;

#define ESCONF_DBUS_NAME "com.expidus.Esconf"
#define ESCONF_DBUS_NAME_TEST "com.expidus.EsconfTest"


/* private api */

GDBusConnection *
_esconf_get_gdbus_connection(void)
{
    if(!esconf_refcnt) {
        g_critical("esconf_init() must be called before attempting to use libesconf!");
        return NULL;
    }

    return gdbus;
}


GDBusProxy *
_esconf_get_gdbus_proxy(void)
{
    if(!esconf_refcnt) {
        g_critical("esconf_init() must be called before attempting to use libesconf!");
        return NULL;
    }

    return gproxy;
}

EsconfNamedStruct *
_esconf_named_struct_lookup(const gchar *struct_name)
{
    return named_structs ? g_hash_table_lookup(named_structs, struct_name) : NULL;
}

static void
_esconf_named_struct_free(EsconfNamedStruct *ns)
{
    g_free(ns->member_types);
    g_slice_free(EsconfNamedStruct, ns);
}

/* public api */

/**
 * SECTION:esconf
 * @title: Esconf Library Core
 * @short_description: Init routines and core functionality for libesconf
 *
 * Before libesconf can be used, it must be initialized by calling
 * esconf_init().  To free resources used by the library, call
 * esconf_shutdown().  These calls are "recursive": multiple calls to
 * esconf_init() are allowed, but each call must be matched by a
 * separate call to esconf_shutdown() to really free the library's
 * resources.
 **/

/**
 * esconf_init:
 * @error: An error return.
 *
 * Initializes the Esconf library.  Can be called multiple times with no
 * adverse effects.
 *
 * Returns: %TRUE if the library was initialized succesfully, %FALSE on
 *          error.  If there is an error @error will be set.
 **/
gboolean
esconf_init(GError **error)
{
    const gchar *is_test_mode;

    if(esconf_refcnt) {
        ++esconf_refcnt;
        return TRUE;
    }

    gdbus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);

    if (!gdbus)
        return FALSE;

    is_test_mode = g_getenv ("ESCONF_RUN_IN_TEST_MODE");
    gproxy = g_dbus_proxy_new_sync(gdbus,
                                   G_DBUS_PROXY_FLAGS_NONE,
                                   NULL,
                                   is_test_mode == NULL ? ESCONF_DBUS_NAME : ESCONF_DBUS_NAME_TEST,
                                   "/com/expidus/Esconf",
                                   "com.expidus.Esconf",
                                   NULL,
                                   NULL);

    ++esconf_refcnt;
    return TRUE;
}

/**
 * esconf_shutdown:
 *
 * Shuts down and frees any resources consumed by the Esconf library.
 * If esconf_init() is called multiple times, esconf_shutdown() must be
 * called an equal number of times to shut down the library.
 **/
void
esconf_shutdown(void)
{
    if(esconf_refcnt <= 0) {
        return;
    }

    if(esconf_refcnt > 1) {
        --esconf_refcnt;
        return;
    }

    /* Flush pending dbus calls */
    g_dbus_connection_flush_sync (gdbus, NULL, NULL);

    _esconf_channel_shutdown();
    _esconf_g_bindings_shutdown();

    if(named_structs) {
        g_hash_table_destroy(named_structs);
        named_structs = NULL;
    }

    --esconf_refcnt;
}

/**
 * esconf_named_struct_register:
 * @struct_name: The unique name of the struct to register.
 * @n_members: The number of data members in the struct.
 * @member_types: An array of the #GType<!-- -->s of the struct members.
 *
 * Registers a named struct for use with esconf_channel_get_named_struct()
 * and esconf_channel_set_named_struct().
 **/
void
esconf_named_struct_register(const gchar *struct_name,
                             guint n_members,
                             const GType *member_types)
{
    EsconfNamedStruct *ns;

    g_return_if_fail(struct_name && *struct_name && n_members && member_types);

    /* lazy initialize the hash table */
    if(named_structs == NULL)
        named_structs = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              (GDestroyNotify)g_free,
                                              (GDestroyNotify)_esconf_named_struct_free);

    if(G_UNLIKELY(g_hash_table_lookup(named_structs, struct_name)))
        g_critical("The struct '%s' is already registered", struct_name);
    else {
        ns = g_slice_new(EsconfNamedStruct);
        ns->n_members = n_members;
        ns->member_types = g_new(GType, n_members);
        memcpy(ns->member_types, member_types, sizeof(GType) * n_members);

        g_hash_table_insert(named_structs, g_strdup(struct_name), ns);
    }
}

#if 0
/**
 * esconf_array_new:
 * @n_preallocs: Number of entries to preallocate space for.
 *
 * Convenience function to greate a new #GArray to hold
 * #GValue<!-- -->s.  Normal #GArray functions may be used on
 * the returned array.  For convenience, see also esconf_array_free().
 *
 * Returns: A new #GArray.
 **/
GArray *
esconf_array_new(gint n_preallocs)
{
    return g_array_sized_new(FALSE, TRUE, sizeof(GValue), n_preallocs);
}
#endif

/**
 * esconf_array_free:
 * @arr: (element-type GValue): A #GPtrArray of #GValue<!-- -->s.
 *
 * Properly frees a #GPtrArray structure containing a list of
 * #GValue<!-- -->s.  This will also cause the contents of the
 * values to be freed as well.
 **/
void
esconf_array_free(GPtrArray *arr)
{
    guint i;

    if(!arr)
        return;

    for(i = 0; i < arr->len; ++i) {
        GValue *val = g_ptr_array_index(arr, i);
        g_value_unset(val);
        g_free(val);
    }

    g_ptr_array_free(arr, TRUE);
}



#define __ESCONF_C__
#include "common/esconf-aliasdef.c"
