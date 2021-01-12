/*
 *  esconfd
 *  
 *  Copyright (c) 2016 Ali Abdallah <ali@xfce.org>
 *  Copyright (c) 2007 Brian Tarricone <bjt23@cornell.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License ONLY.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libexpidus1util/libexpidus1util.h>

#include "esconf-daemon.h"
#include "esconf-backend-factory.h"
#include "esconf-backend.h"
#include "common/esconf-marshal.h"
#include "common/esconf-gvaluefuncs.h"
#include "esconf/esconf-errors.h"
#include "common/esconf-common-private.h"
#include "common/esconf-gdbus-bindings.h"

struct _EsconfDaemon
{
    EsconfExportedSkeleton parent;
    guint filter_id;
    
    GDBusConnection *conn;

    GList *backends;
};

typedef struct _EsconfDaemonClass
{
    EsconfExportedSkeletonClass parent;
} EsconfDaemonClass;

static void esconf_daemon_finalize(GObject *obj);

G_DEFINE_TYPE(EsconfDaemon, esconf_daemon, ESCONF_TYPE_EXPORTED_SKELETON)
  
static void
esconf_daemon_class_init(EsconfDaemonClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;

    object_class->finalize = esconf_daemon_finalize;
}

static void
esconf_daemon_init(EsconfDaemon *instance)
{
    instance->filter_id = 0;
}

static void
esconf_daemon_finalize(GObject *obj)
{
    EsconfDaemon *esconfd = ESCONF_DAEMON(obj);
    GList *l;
    for(l = esconfd->backends; l; l = l->next) {
        esconf_backend_register_property_changed_func(l->data, NULL, NULL);
        esconf_backend_flush(l->data, NULL);
        g_object_unref(l->data);
    }
    g_list_free(esconfd->backends);

    if(esconfd->filter_id) {
        g_signal_handler_disconnect (esconfd->conn, esconfd->filter_id);
    }

    G_OBJECT_CLASS(esconf_daemon_parent_class)->finalize(obj);
}

typedef struct
{
    EsconfDaemon *esconfd;
    EsconfBackend *backend;
    gchar *channel;
    gchar *property;
} EsconfPropChangedData;

static gboolean
esconf_daemon_emit_property_changed_idled(gpointer data)
{
    EsconfPropChangedData *pdata = data;
    GValue value = { 0, };
    esconf_backend_get(pdata->backend, pdata->channel, pdata->property,
                       &value, NULL);
    if(G_VALUE_TYPE(&value)) {
        GVariant *val, *variant;
        val = esconf_gvalue_to_gvariant (&value);
        if (val) {
            variant = g_variant_new_variant (val);
            esconf_exported_emit_property_changed ((EsconfExported*)pdata->esconfd,
                                                   pdata->channel, pdata->property, variant);
            g_variant_unref (val);
        }
        g_value_unset(&value);
    } else {
        esconf_exported_emit_property_removed ((EsconfExported*)pdata->esconfd,
                                               pdata->channel, pdata->property);
    }
    g_object_unref(G_OBJECT(pdata->backend));
    g_free(pdata->channel);
    g_free(pdata->property);
    g_object_unref(G_OBJECT(pdata->esconfd));
    g_slice_free(EsconfPropChangedData, pdata);

    return FALSE;
}

static void
esconf_daemon_backend_property_changed(EsconfBackend *backend,
                                       const gchar *channel,
                                       const gchar *property,
                                       gpointer user_data)
{
    EsconfPropChangedData *pdata = g_slice_new0(EsconfPropChangedData);
    pdata->esconfd = g_object_ref(ESCONF_DAEMON(user_data));
    pdata->backend = g_object_ref(ESCONF_BACKEND(backend));
    pdata->channel = g_strdup(channel);
    pdata->property = g_strdup(property);
    g_idle_add(esconf_daemon_emit_property_changed_idled, pdata);
}

static gboolean
esconf_set_property(EsconfExported *skeleton,
                    GDBusMethodInvocation *invocation,
                    const gchar *channel,
                    const gchar *property,
                    GVariant *variant,
                    EsconfDaemon *esconfd)
{
    GList *l;
    GError *error = NULL;
    GValue *value;

    /* if there's more than one backend, we need to make sure the
     * property isn't locked on ANY of them */
    if(G_UNLIKELY(esconfd->backends->next)) {
        for(l = esconfd->backends; l; l = l->next) {
            gboolean locked = FALSE;

            if(!esconf_backend_is_property_locked(l->data, channel, property,
                                                  &locked, &error))
                break;

            if(locked) {
                g_set_error(&error, ESCONF_ERROR,
                            ESCONF_ERROR_PERMISSION_DENIED,
                            _("Permission denied while modifying property \"%s\" on channel \"%s\""),
                            property, channel);
                break;
            }
        }

        /* there is always an error set if something failed or the
         * property is locked */
        if(error) {
            g_dbus_method_invocation_return_gerror(invocation, error);
            g_error_free(error);
            return FALSE;
        }
    }
    
    value = esconf_gvariant_to_gvalue (variant);
    /* only write to first backend */
    if(esconf_backend_set(esconfd->backends->data, channel, property,
                          value, &error))
    {
        esconf_exported_complete_set_property(skeleton, invocation);
    } else {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    }

    g_value_unset (value);
    g_free (value);
    return TRUE;
}


static gboolean
esconf_get_property(EsconfExported *skeleton,
                    GDBusMethodInvocation *invocation,
                    const gchar *channel,
                    const gchar *property,
                    EsconfDaemon *esconfd)
{
    GList *l;
    GValue value = { 0, };
    GError *error = NULL;

    /* check each backend until we find a value */
    for(l = esconfd->backends; l; l = l->next) {
        if(esconf_backend_get(l->data, channel, property, &value, &error)) {
            GVariant *variant, *val;
            val = esconf_gvalue_to_gvariant (&value);
            if (val){
                variant = g_variant_new_variant (val);
                esconf_exported_complete_get_property(skeleton, invocation, variant);
                g_variant_unref (val);
            }
            else {
                g_set_error (&error, ESCONF_ERROR, 
                             ESCONF_ERROR_INTERNAL_ERROR, _("GType transformation failed \"%s\""),
                             G_VALUE_TYPE_NAME(&value));
                g_dbus_method_invocation_return_gerror(invocation, error);
                g_error_free(error);
            }
            g_value_unset(&value);
            return TRUE;
        } else if(l->next)
            g_clear_error(&error);
    }
    g_dbus_method_invocation_return_gerror(invocation, error);
    g_error_free(error);
    return TRUE;
}

static gboolean
esconf_get_all_properties(EsconfExported *skeleton,
                          GDBusMethodInvocation *invocation,
                          const gchar *channel,
                          const gchar *property_base,
                          EsconfDaemon *esconfd)
{
    GList *l;
    GHashTable *properties;
    GError *error = NULL;
    gboolean succeed = FALSE;
    properties = g_hash_table_new_full(g_str_hash, g_str_equal,
                                        (GDestroyNotify)g_free,
                                        (GDestroyNotify)_esconf_gvalue_free);
    /* get all properties from all backends.  if they all fail, return FALSE */
    for(l = esconfd->backends; l; l = l->next) {
        if(esconf_backend_get_all(l->data, channel, property_base,
                                  properties, &error))
            succeed = TRUE;
        else if(l->next) {
            g_clear_error(&error);
        }
    }
    if(succeed) {
        GVariant *variant;
        variant = esconf_hash_to_gvariant (properties);
        esconf_exported_complete_get_all_properties (skeleton, invocation, variant);
    }
    else
        g_dbus_method_invocation_return_gerror(invocation, error);

    if(error)
        g_error_free(error);
    g_hash_table_destroy(properties);
    return TRUE;
}

static gboolean
esconf_property_exists(EsconfExported *skeleton,
                       GDBusMethodInvocation *invocation,
                       const gchar *channel,
                       const gchar *property,
                       EsconfDaemon *esconfd)
{
    gboolean exists = FALSE;
    gboolean succeed = FALSE;
    GList *l;
    GError *error = NULL;
    /* if at least one backend returns TRUE (regardles if |*exists| gets set
     * to TRUE or FALSE), we'll return TRUE from this function */
    for(l = esconfd->backends; !exists && l; l = l->next) {
        if(esconf_backend_exists(l->data, channel, property, &exists, &error))
            succeed = TRUE;
        else if(l->next)
            g_clear_error(&error);
    }

    if(succeed)
        esconf_exported_complete_property_exists (skeleton, invocation, exists);
    else {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    }
    return TRUE;
}

static gboolean
esconf_reset_property(EsconfExported *skeleton,
                      GDBusMethodInvocation *invocation,
                      const gchar *channel,
                      const gchar *property,
                      gboolean recursive,
                      EsconfDaemon *esconfd)
{
    gboolean succeed = FALSE;
    GList *l;
    GError *error = NULL;
    /* while technically all backends but the first should be opened read-only,
     * we need to reset in all backends so the property doesn't reappear
     * later */

    for(l = esconfd->backends; l; l = l->next) {
        if(esconf_backend_reset(l->data, channel, property, recursive, &error))
            succeed = TRUE;
        else if(l->next)
            g_clear_error(&error);
    }

    if(succeed)
        esconf_exported_complete_reset_property(skeleton, invocation);
    else
        g_dbus_method_invocation_return_gerror(invocation, error);

    if(error)
        g_error_free(error);

    return TRUE;
}

static gboolean
esconf_list_channels(EsconfExported *skeleton,
                    GDBusMethodInvocation *invocation,
                    EsconfDaemon *esconfd)
{
    GSList *lchannels = NULL, *chans_tmp, *lc;
    GList *l;
    guint i;
    gchar **channels;
    GError *error = NULL;
    /* FIXME: with multiple backends, this can cause duplicates */
    for(l = esconfd->backends; l; l = l->next) {
        chans_tmp = NULL;
        if(esconf_backend_list_channels(l->data, &chans_tmp, &error))
            lchannels = g_slist_concat(lchannels, chans_tmp);
        else if(l->next)
            g_clear_error(&error);
    }

    if(error && !lchannels) {
        /* no channels and an error, something went wrong */
        g_dbus_method_invocation_return_gerror(invocation, error);
    } else {
        channels = g_new (gchar *, g_slist_length(lchannels) + 1);
        for(lc = lchannels, i = 0; lc; lc = lc->next, ++i)
            channels[i] = lc->data;
        channels[i] = NULL;
        
        esconf_exported_complete_list_channels (skeleton, invocation, (const gchar *const*)channels);

        g_strfreev(channels);
        g_slist_free(lchannels);
    }

    if(error)
        g_error_free(error);

    return TRUE;
}

static gboolean esconf_is_property_locked(EsconfExported *skeleton,
                                          GDBusMethodInvocation *invocation,
                                          const gchar *channel,
                                          const gchar *property,
                                          EsconfDaemon *esconfd)
{
    GList *l;
    gboolean locked = FALSE;
    GError *error = NULL;
    gboolean succeed = FALSE;
    for(l = esconfd->backends; !locked && l; l = l->next) {
        if(esconf_backend_is_property_locked(l->data, channel, property,
                                             &locked, &error))
            succeed = TRUE;
        else if(l->next)
            g_clear_error(&error);
    }

    if(succeed)
        esconf_exported_complete_is_property_locked(skeleton, invocation, locked);
    else
        g_dbus_method_invocation_return_gerror(invocation, error);

    if(error)
        g_error_free(error);

    return TRUE;
}

static void
esconf_daemon_handle_dbus_disconnect(GDBusConnection *conn,
                                     gboolean remote,
                                     GError *error,
                                     gpointer data)
{
    EsconfDaemon *esconfd = (EsconfDaemon*)data;
    GList *l;
    
    DBG("got dbus disconnect; flushing all channels");
    
    for(l = esconfd->backends; l; l = l->next) {
        GError *lerror = NULL;
        if(!esconf_backend_flush(ESCONF_BACKEND(l->data), &lerror)) {
            g_critical("Failed to flush backend on disconnect: %s",
                       lerror->message);
            g_error_free(lerror);
        }
    }
    
}



static gboolean
esconf_daemon_start(EsconfDaemon *esconfd,
                    GError **error)
{
    int ret;

    esconfd->conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);
    if (G_UNLIKELY(!esconfd->conn))
    {
        return FALSE;
    }
    
    ret = 
    g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON(esconfd),
                                      esconfd->conn,
                                      "/com/expidus/Esconf",
                                      error);
    
    if (ret == FALSE)
        return FALSE;
    
    esconfd->filter_id = g_signal_connect (esconfd->conn, "closed",
                                           G_CALLBACK(esconf_daemon_handle_dbus_disconnect),
                                           esconfd);

    return TRUE;
}

static gboolean
esconf_daemon_load_config(EsconfDaemon *esconfd,
                          gchar * const *backend_ids,
                          GError **error)
{
    gint i;

    for(i = 0; backend_ids[i]; ++i) {
        GError *error1 = NULL;
        EsconfBackend *backend = esconf_backend_factory_get_backend(backend_ids[i],
                                                                    &error1);
        if(!backend) {
            g_warning("Unable to start backend \"%s\": %s", backend_ids[i],
                      error1->message);
            g_clear_error(&error1);
        } else {
            esconfd->backends = g_list_prepend(esconfd->backends, backend);
            esconf_backend_register_property_changed_func(backend,
                                                          esconf_daemon_backend_property_changed,
                                                          esconfd);
        }
    }

    if(!esconfd->backends) {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_NO_BACKEND,
                        _("No backends could be started"));
        }
        return FALSE;
    }

    esconfd->backends = g_list_reverse(esconfd->backends);

    return TRUE;
}


EsconfDaemon *
esconf_daemon_new_unique(gchar * const *backend_ids,
                         GError **error)
{
    EsconfDaemon *esconfd;

    g_return_val_if_fail(backend_ids && backend_ids[0], NULL);

    esconfd = g_object_new(ESCONF_TYPE_DAEMON, NULL);

    if(!esconf_daemon_start(esconfd, error)
       || !esconf_daemon_load_config(esconfd, backend_ids, error))
    {
        g_object_unref(G_OBJECT(esconfd));
        return NULL;
    }

    g_signal_connect (esconfd, "handle-get-all-properties",
                      G_CALLBACK(esconf_get_all_properties), esconfd);
    
    g_signal_connect (esconfd, "handle-get-property",
                      G_CALLBACK(esconf_get_property), esconfd);

    g_signal_connect (esconfd, "handle-is-property-locked",
                      G_CALLBACK(esconf_is_property_locked), esconfd);
    
    g_signal_connect (esconfd, "handle-list-channels",
                      G_CALLBACK(esconf_list_channels), esconfd);

    g_signal_connect (esconfd, "handle-property-exists",
                      G_CALLBACK(esconf_property_exists), esconfd);
    
    g_signal_connect (esconfd, "handle-reset-property",
                      G_CALLBACK(esconf_reset_property), esconfd);
    
    g_signal_connect (esconfd, "handle-set-property",
                      G_CALLBACK(esconf_set_property), esconfd);
    
    return esconfd;
}
