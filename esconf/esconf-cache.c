/*
 *  esconf
 *
 *  Copyright (c) 2016 Ali Abdallah <ali@xfce.org>
 *  Copyright (c) 2009 Brian Tarricone <brian@tarricone.org>
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

#include "esconf-cache.h"
#include "esconf-channel.h"
#include "esconf-errors.h"
#include "common/esconf-gdbus-bindings.h"
#include "common/esconf-gvaluefuncs.h"
#include "esconf-private.h"
#include "common/esconf-marshal.h"
#include "common/esconf-common-private.h"
#include "esconf-types.h"

#if 0
#include "esconf.h"
#include "esconf-alias.h"
#endif

#if 0
#define DEFAULT_MAX_ENTRIES  -1  /* no limit */
#define DEFAULT_MAX_AGE      (60*60)  /* 1 hour */
#endif

#define ALIGN_VAL(val, align)  ( ((val) + ((align) -1)) & ~((align) - 1) )



#define esconf_cache_mutex_lock(cache)   g_mutex_lock (&(cache)->cache_lock)
#define esconf_cache_mutex_unlock(cache) g_mutex_unlock (&(cache)->cache_lock)



/**************** EsconfCacheItem ****************/


typedef struct
{
#if 0
    GTimeVal last_used;
#endif
    GValue *value;
} EsconfCacheItem;

static EsconfCacheItem *
esconf_cache_item_new(const GValue *value,
                      gboolean steal)
{
    EsconfCacheItem *item;

    g_return_val_if_fail(value, NULL);

    item = g_slice_new0(EsconfCacheItem);
#if 0
    g_get_current_time(&item->last_used);
#endif

    if(G_LIKELY(steal)) {
        item->value = (GValue *) value;
    } else {

        item->value = g_new0(GValue, 1);
        g_value_init(item->value, G_VALUE_TYPE(value));
        /* We need to dup the array */
        if (G_VALUE_TYPE(value) == G_TYPE_PTR_ARRAY) {
            GPtrArray *arr = esconf_dup_value_array(g_value_get_boxed(value), TRUE);
            g_value_take_boxed(item->value, arr);
        } else {
            g_value_copy(value, item->value);
        }
    }

    return item;
}

static gboolean
esconf_cache_item_update(EsconfCacheItem *item,
                         const GValue *value)
{
    if(value && _esconf_gvalue_is_equal(item->value, value))
        return FALSE;

#if 0
    g_get_current_time(&item->last_used);
#endif

    if(value) {
        g_value_unset(item->value);
        g_value_init(item->value, G_VALUE_TYPE(value));

        /* We need to dup the array */
        if (G_VALUE_TYPE(value) == G_TYPE_PTR_ARRAY) {
            GPtrArray *arr = esconf_dup_value_array(g_value_get_boxed(value), TRUE);
            g_value_take_boxed(item->value, arr);
        } else {
            g_value_copy(value, item->value);
        }
        return TRUE;
    }

    return FALSE;
}

static void
esconf_cache_item_free(EsconfCacheItem *item)
{
    g_return_if_fail(item);

    g_value_unset(item->value);
    g_free(item->value);
    g_slice_free(EsconfCacheItem, item);
}


/******************* EsconfCacheOldItem *******************/

/**
 * EsconfCacheOldItem:
 * @property:
 * @item: a #EsconfCacheItem object
 * @cancellable:
 * @pending_calls_count:
 * @variant: Used in esconf_cache_old_item_end_call to end an already
 *           started call
 * @cache: Pointer to the cache object
 */
typedef struct
{
    gchar *property;
    EsconfCacheItem *item;

    GCancellable *cancellable;

    gint pending_calls_count;

    GVariant *variant;

    EsconfCache *cache;

} EsconfCacheOldItem;


static EsconfCacheOldItem *
esconf_cache_old_item_new(EsconfCache *cache, const gchar *property)
{
    EsconfCacheOldItem *old_item;

    g_return_val_if_fail(property, NULL);

    old_item = g_slice_new0(EsconfCacheOldItem);
    old_item->property = g_strdup(property);
    old_item->cancellable = g_cancellable_new ();
    old_item->cache = cache;
    old_item->variant = NULL;
    old_item->pending_calls_count = 0;

    return old_item;
}

static void
esconf_cache_old_item_free(EsconfCacheOldItem *old_item)
{
    g_return_if_fail(old_item);

    /* debug check to make sure the call is properly handled before
     * freeing the item. it should either been cancelled or we wait for
     * it to finish */
    g_return_if_fail(g_cancellable_is_cancelled(old_item->cancellable) == TRUE);

    g_object_unref (old_item->cancellable);
    g_free(old_item->property);

    if (old_item->variant)
        g_variant_unref (old_item->variant);

    if(old_item->item)
        esconf_cache_item_free(old_item->item);

    g_slice_free(EsconfCacheOldItem, old_item);
}

static gboolean
esconf_cache_old_item_end_call(gpointer key,
                               gpointer value,
                               gpointer user_data)
{
    GError *error = NULL;
    EsconfCacheOldItem *old_item = value;
    GDBusProxy *gproxy = _esconf_get_gdbus_proxy();
    const gchar *channel_name = user_data;
    GVariant *variant;

    g_return_val_if_fail(g_cancellable_is_cancelled(old_item->cancellable) == FALSE, TRUE);

    variant = g_variant_new_variant (old_item->variant);

    g_cancellable_cancel(old_item->cancellable);

    esconf_exported_call_set_property_sync ((EsconfExported *)gproxy,
                                            channel_name,
                                            old_item->property,
                                            variant,
                                            NULL,
                                            &error);

    if (error) {
        g_warning("Failed to set property \"%s::%s\": %s",
                  channel_name, old_item->property, error->message);
        g_error_free(error);
    }

    return TRUE;
}


/************************* EsconfCache ********************/


/**
 * EsconfCache:
 *
 * An opaque structure that holds state about a cache.
 **/
struct _EsconfCache
{
    GObject parent;

    gchar *channel_name;

#if 0
    gint max_entries;
    gint max_age;
#endif

    GTree *properties;

    GHashTable *pending_calls;
    GHashTable *old_properties;

    gint g_signal_id;

    GMutex cache_lock;
};

typedef struct _EsconfCacheClass
{
    GObjectClass parent;

    void (*property_changed)(EsconfCache *cache,
                             const gchar *channel_name,
                             const gchar *property,
                             const GValue *value);
} EsconfCacheClass;

enum
{
    SIG_PROPERTY_CHANGED = 0,
    N_SIGS,
};

enum
{
    PROP0 = 0,
    PROP_CHANNEL_NAME,
#if 0
    PROP_MAX_ENTRIES,
    PROP_MAX_AGE,
#endif
};

static void esconf_cache_set_g_property(GObject *object,
                                        guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec);
static void esconf_cache_get_g_property(GObject *object,
                                        guint property_id,
                                        GValue *value,
                                        GParamSpec *pspec);
static void esconf_cache_finalize(GObject *obj);

static void esconf_cache_proxy_signal_received_cb(GDBusProxy *proxy,
                                                  gchar      *sender_name,
                                                  gchar      *signal_name,
                                                  GVariant   *parameters,
                                                  gpointer    user_data);


static guint signals[N_SIGS] = { 0, };


G_DEFINE_TYPE(EsconfCache, esconf_cache, G_TYPE_OBJECT)


static void
esconf_cache_class_init(EsconfCacheClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;

    object_class->set_property = esconf_cache_set_g_property;
    object_class->get_property = esconf_cache_get_g_property;
    object_class->finalize = esconf_cache_finalize;

    signals[SIG_PROPERTY_CHANGED] = g_signal_new(I_("property-changed"),
                                                 ESCONF_TYPE_CACHE,
                                                 G_SIGNAL_RUN_LAST
                                                 | G_SIGNAL_DETAILED,
                                                 G_STRUCT_OFFSET(EsconfCacheClass,
                                                                 property_changed),
                                                 NULL,
                                                 NULL,
                                                 _esconf_marshal_VOID__STRING_STRING_BOXED,
                                                 G_TYPE_NONE,
                                                 3, G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_VALUE);

    g_object_class_install_property(object_class, PROP_CHANNEL_NAME,
                                    g_param_spec_string("channel-name",
                                                        "Channel Name",
                                                        "The name of the channel managed by the cache",
                                                        NULL,
                                                        G_PARAM_READWRITE
                                                        | G_PARAM_CONSTRUCT_ONLY
                                                        | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_NICK
                                                        | G_PARAM_STATIC_BLURB));
#if 0
    g_object_class_install_property(object_class, PROP_MAX_ENTRIES,
                                    g_param_spec_int("max-entries",
                                                     "Maximum entries",
                                                     "Maximum number of cache entries to hold at once",
                                                     -1, G_MAXINT,
                                                     DEFAULT_MAX_ENTRIES,
                                                     G_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT
                                                     | G_PARAM_STATIC_NAME
                                                     | G_PARAM_STATIC_NICK
                                                     | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class, PROP_MAX_AGE,
                                    g_param_spec_int("max-age",
                                                     "Maximum age",
                                                     "Maximum age (in seconds) before an entry gets evicted from the cache",
                                                     0, G_MAXINT,
                                                     DEFAULT_MAX_AGE,
                                                     G_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT
                                                     | G_PARAM_STATIC_NAME
                                                     | G_PARAM_STATIC_NICK
                                                     | G_PARAM_STATIC_BLURB));
#endif
}

static void
esconf_cache_init(EsconfCache *cache)
{
    GDBusProxy *gproxy = _esconf_get_gdbus_proxy();

    cache->g_signal_id = g_signal_connect(gproxy, "g-signal",
                                          G_CALLBACK(esconf_cache_proxy_signal_received_cb), cache);

    cache->properties = g_tree_new_full((GCompareDataFunc) (void (*)(void)) strcmp, NULL,
                                        (GDestroyNotify)g_free,
                                        (GDestroyNotify)esconf_cache_item_free);

    cache->pending_calls = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                                 NULL, NULL);
    cache->old_properties = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                  NULL, NULL);

    g_mutex_init (&cache->cache_lock);
}

static void
esconf_cache_set_g_property(GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
    EsconfCache *cache = ESCONF_CACHE(object);

    switch(property_id) {
        case PROP_CHANNEL_NAME:
            g_free(cache->channel_name);
            cache->channel_name = g_value_dup_string(value);
            break;
#if 0
        case PROP_MAX_ENTRIES:
            esconf_cache_set_max_entries(cache, g_value_get_int(value));
            break;

        case PROP_MAX_AGE:
            esconf_cache_set_max_age(cache, g_value_get_int(value));
            break;
#endif
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
esconf_cache_get_g_property(GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
    EsconfCache *cache = ESCONF_CACHE(object);

    switch(property_id) {
        case PROP_CHANNEL_NAME:
            g_value_set_string(value, cache->channel_name);
            break;
#if 0
        case PROP_MAX_ENTRIES:
            g_value_set_int(value, cache->max_entries);
            break;

        case PROP_MAX_AGE:
            g_value_set_int(value, cache->max_age);
            break;
#endif
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
esconf_cache_finalize(GObject *obj)
{
    EsconfCache *cache = ESCONF_CACHE(obj);
    GDBusProxy *proxy;

    proxy = _esconf_get_gdbus_proxy();

    g_signal_handler_disconnect(proxy,cache->g_signal_id);

    /* Finish pending calls with synchronous requests (without emitting
     * signals, therefore we cancel the cancellable on old_item).
     * Beware: even that we cancel cancellable objects for unfinished
     * asynchronous calls, their handlers are guaranted to be run in the
     * thread-default main context after we finish (i.e. after EsconfCache
     * will be freed). Due to that, we must not free - outside of handler
     * itself - the EsconfCacheOldItems provided as user_data to those
     * handlers. Otherwise the handler will have no realiable way of
     * knowing that call has been cancelled and will operate on freed data. */
    g_hash_table_foreach_remove(cache->pending_calls,
                                esconf_cache_old_item_end_call,
                                cache->channel_name);
    g_hash_table_unref(cache->pending_calls);

    g_free(cache->channel_name);

    g_tree_destroy(cache->properties);
    g_hash_table_destroy(cache->old_properties);

    G_OBJECT_CLASS(esconf_cache_parent_class)->finalize(obj);
}


static void
esconf_cache_handle_property_changed (EsconfCache *cache, GVariant *parameters)
{

    EsconfCacheItem *item;
    const gchar *channel_name, *property;
    GVariant *prop_variant;
    GValue *prop_value;
    gboolean changed = TRUE;
    if (g_variant_is_of_type(parameters, G_VARIANT_TYPE ("(ssv)"))) {
        g_variant_get(parameters, "(&s&sv)", &channel_name, &property, &prop_variant);

        if(strcmp(channel_name, cache->channel_name)) {
            return;
        }
        prop_value = esconf_gvariant_to_gvalue (prop_variant);

        /* if a call was cancelled, we still receive a property-changed from
         * that value, in that case, abort the emission of the signal. we can
         * detect this because the new reply is not processed yet and thus
         * there is still an old_prop in the hash table */
        if(g_hash_table_lookup(cache->old_properties, property))
            return;

        item = g_tree_lookup(cache->properties, property);
        if(item) {
            changed = esconf_cache_item_update(item, prop_value);
        }
        else {
            item = esconf_cache_item_new(prop_value, TRUE);
            g_tree_insert(cache->properties, g_strdup(property), item);
        }

        if(changed) {
            g_signal_emit(G_OBJECT(cache), signals[SIG_PROPERTY_CHANGED], 0,
                          cache->channel_name, property, prop_value);
        }
        g_variant_unref(prop_variant);
    }
    else {
        g_warning("property changed handler expects (ssv) type, but %s received",
                  g_variant_get_type_string(parameters));
    }

}


static void
esconf_cache_handle_property_removed (EsconfCache *cache, GVariant *parameters)
{

    const gchar *channel_name, *property;
    GValue value = G_VALUE_INIT;
    if (g_variant_is_of_type(parameters, G_VARIANT_TYPE ("(ss)"))) {
        g_variant_get(parameters, "(&s&s)", &channel_name, &property);

        if(strcmp(channel_name, cache->channel_name))
            return;

        g_tree_remove(cache->properties, property);

        g_signal_emit(G_OBJECT(cache), signals[SIG_PROPERTY_CHANGED], 0,
                      cache->channel_name, property, &value);

    }
    else {
        g_warning("property removed handler expects (ss) type, but %s received",
                  g_variant_get_type_string(parameters));
    }

}


static void
esconf_cache_proxy_signal_received_cb(GDBusProxy *proxy,
                                      gchar      *sender_name,
                                      gchar      *signal_name,
                                      GVariant   *parameters,
                                      gpointer    user_data)
{
    EsconfCache *cache=(EsconfCache*)user_data;

    g_return_if_fail(ESCONF_IS_CACHE(cache));

    if (g_strcmp0(signal_name, "PropertyChanged") == 0)
        esconf_cache_handle_property_changed (cache, parameters);
    else if (g_strcmp0(signal_name, "PropertyRemoved") == 0)
        esconf_cache_handle_property_removed(cache, parameters);
    else
        g_warning ("Unhandled signal name :%s\n", signal_name);
}


static void
esconf_cache_set_property_reply_handler(GDBusProxy *proxy,
                                        GAsyncResult *res,
                                        gpointer user_data)
{
    EsconfCache *cache;
    EsconfCacheOldItem *old_item = (EsconfCacheOldItem*) user_data;
    EsconfCacheItem *item;
    GError *error = NULL;
    gboolean result;

    old_item->pending_calls_count--;
    if(old_item->pending_calls_count > 0)
        return;

    /* cancellable is cancelled in esconf_cache_old_item_end_call to inform that
     * XconfCache finalization started. That means the last value of
     * property has been set synchronously, invalidating the need to run this
     * handler for any previously started, unfinished asynchronous calls. */
    if (g_cancellable_is_cancelled(old_item->cancellable) == TRUE)
    {
        esconf_cache_old_item_free(old_item);
        return;
    }

    cache = old_item->cache;
    esconf_cache_mutex_lock(cache);
/*
    old_item = g_hash_table_lookup(cache->pending_calls, call);
    if(G_UNLIKELY(!old_item)) {
#ifndef NDEBUG
        g_debug("Couldn't find old cache item based on pending call (libesconf bug?)");
#endif
        goto out;
    }
*/
    g_hash_table_remove(cache->old_properties, old_item->property);
    g_hash_table_remove(cache->pending_calls, old_item->cancellable);
    item = g_tree_lookup(cache->properties, old_item->property);
    if(G_UNLIKELY(!item)) {
#ifndef NDEBUG
        g_debug("Couldn't find current cache item based on pending call (libesconf bug?)");
#endif
        goto out;
    }

    result = esconf_exported_call_set_property_finish ((EsconfExported*)proxy, res, &error);
    if (!result) {
        GValue empty_val = { 0, };
        g_warning("Failed to set property \"%s::%s\": %s",
                  cache->channel_name, old_item->property, error->message);
        g_error_free(error);
        if(old_item->item)
            esconf_cache_item_update(item, old_item->item->value);
        else {
            g_tree_remove(cache->properties, old_item->property);
            item = NULL;
        }

        /* we need to drop the lock when running the signal handlers */
        esconf_cache_mutex_unlock(cache);
        g_signal_emit(G_OBJECT(cache), signals[SIG_PROPERTY_CHANGED],
                      g_quark_from_string(old_item->property),
                      cache->channel_name, old_item->property,
                      item ? item->value : &empty_val);
        esconf_cache_mutex_lock(cache);
    }

    /* we handled the call */
    g_cancellable_cancel(old_item->cancellable);
    esconf_cache_old_item_free(old_item);
out:
    esconf_cache_mutex_unlock(cache);
}



#if 0
static void
esconf_cache_reset_property_reply_handler(DBusGProxy *proxy,
                                          DBusGProxyCall *call,
                                          gpointer user_data)
{
    EsconfCache *cache = user_data;
    EsconfCacheOldItem *old_item;
    GError *error = NULL;

    esconf_cache_mutex_lock(cache);

    old_item = g_hash_table_lookup(cache->pending_calls, call);
    if(G_UNLIKELY(!old_item)) {
        g_critical("Couldn't find old cache item based on pending call (libesconf bug?)");
        goto out;
    }

    if(!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_INVALID)) {
        g_warning("Failed to reset property \"%s::%s\": %s",
                  cache->channel_name, old_item->property, error->message);
        g_error_free(error);
    }

out:
    if(old_item)
        g_hash_table_remove(cache->pending_calls, old_item->call);

    esconf_cache_mutex_unlock(cache);
}
#endif



EsconfCache *
esconf_cache_new(const gchar *channel_name)
{
    return g_object_new(ESCONF_TYPE_CACHE,
                        "channel-name", channel_name,
                        NULL);
}

gboolean
esconf_cache_prefetch(EsconfCache *cache,
                      const gchar *property_base,
                      GError **error)
{
    GVariant *props_variant, *value;
    GVariantIter *iter;
    gchar *key;
    gboolean ret = FALSE;
    GDBusProxy *proxy = _esconf_get_gdbus_proxy ();
    GError *tmp_error = NULL;

    g_return_val_if_fail(g_tree_nnodes(cache->properties) == 0, FALSE);

    esconf_cache_mutex_lock(cache);

    if(esconf_exported_call_get_all_properties_sync((EsconfExported *)proxy, cache->channel_name,
                                                  property_base ? property_base : "/",
                                                  &props_variant, NULL, &tmp_error))
    {
        g_variant_get (props_variant, "a{sv}", &iter);

        while (g_variant_iter_next (iter, "{sv}", &key, &value))
        {
            EsconfCacheItem *item;

            GValue *gvalue = esconf_gvariant_to_gvalue (value);
            item = esconf_cache_item_new(gvalue, TRUE);
            g_tree_insert(cache->properties, key, item);
        }
        /* TODO: honor max entries */
        ret = TRUE;
        g_variant_iter_free (iter);
        g_variant_unref(props_variant);
    } else
        g_propagate_error(error, tmp_error);

    esconf_cache_mutex_unlock(cache);

    return ret;
}

static gboolean
esconf_cache_lookup_locked(EsconfCache *cache,
                           const gchar *property,
                           GValue *value,
                           GError **error)
{
    EsconfCacheItem *item = NULL;
    item = g_tree_lookup(cache->properties, property);

    if(!item) {
        GVariant *variant;
        GDBusProxy *proxy = _esconf_get_gdbus_proxy();
        GError *tmp_error = NULL;
        /* blocking, ugh */
        if(esconf_exported_call_get_property_sync ((EsconfExported *)proxy, cache->channel_name,
                                                 property, &variant, NULL, &tmp_error))
        {
            GValue *tmpval;
            tmpval = esconf_gvariant_to_gvalue(variant);
            item = esconf_cache_item_new(tmpval, TRUE);
            g_tree_insert(cache->properties, g_strdup(property), item);
            g_variant_unref (variant);
            /* TODO: check tree for evictions */
        } else
            g_propagate_error(error, tmp_error);
    }

    if(item) {
        if(value) {
            if(!G_VALUE_TYPE(value))
                g_value_init(value, G_VALUE_TYPE(item->value));

            if (G_VALUE_TYPE(item->value) == G_TYPE_PTR_ARRAY) {
                if (G_VALUE_TYPE(value) != G_TYPE_PTR_ARRAY) {
                    g_warning("Given value is not of type G_TYPE_PTR_ARRAY");
                    item = NULL;
                }
                else {
                    GPtrArray *arr;
                    arr = esconf_dup_value_array (g_value_get_boxed(item->value), FALSE);
                    g_value_take_boxed(value, arr);
                }
            }
            else {
                if(G_VALUE_TYPE(value) == G_VALUE_TYPE(item->value))
                    g_value_copy(item->value, value);
                else {
                    if(!g_value_transform(item->value, value))
                        item = NULL;
                }
            }
        }
#if 0
        if(item)
            esconf_cache_item_update(item, NULL);
#endif
    }

    return !!item;
}

gboolean
esconf_cache_lookup(EsconfCache *cache,
                    const gchar *property,
                    GValue *value,
                    GError **error)
{
    gboolean ret;

    g_return_val_if_fail(ESCONF_IS_CACHE(cache) && property
                         && (!error || !*error), FALSE);

    esconf_cache_mutex_lock(cache);
    ret = esconf_cache_lookup_locked(cache, property, value, error);
    esconf_cache_mutex_unlock(cache);

    return ret;
}

gboolean
esconf_cache_set(EsconfCache *cache,
                 const gchar *property,
                 const GValue *value,
                 GError **error)
{
    GVariant *variant = NULL, *val = NULL;
    GDBusProxy *proxy = _esconf_get_gdbus_proxy();
    EsconfCacheItem *item = NULL;
    EsconfCacheOldItem *old_item = NULL;
    esconf_cache_mutex_lock(cache);

    item = g_tree_lookup(cache->properties, property);
    if(!item) {
        /* this is really quite the opposite of what we want here,
         * but i can't think of a better way yet. */
        GValue tmp_val = { 0, };
        GError *tmp_error = NULL;
        if(!esconf_cache_lookup_locked(cache, property, &tmp_val, &tmp_error)) {
            gchar *dbus_error_name = NULL;

            if(G_LIKELY(g_dbus_error_is_remote_error (tmp_error)))
                dbus_error_name = g_dbus_error_get_remote_error (tmp_error);

            if(g_strcmp0(dbus_error_name, "com.expidus.Esconf.Error.PropertyNotFound") != 0
               && g_strcmp0(dbus_error_name, "com.expidus.Esconf.Error.ChannelNotFound") != 0)
            {
                /* this is bad... */
                g_propagate_error(error, tmp_error);
                esconf_cache_mutex_unlock(cache);
                g_free (dbus_error_name);
                return FALSE;
            }
            /* prop just doesn't exist; continue */
            g_error_free(tmp_error);
            g_free (dbus_error_name);
        } else {
            g_value_unset(&tmp_val);
            item = g_tree_lookup(cache->properties, property);
        }
    }

    if(item) {
        /* if the value isn't changing, there's no reason to continue */
        if(_esconf_gvalue_is_equal(item->value, value)) {
            esconf_cache_mutex_unlock(cache);
            return TRUE;
        }
    }
    old_item = g_hash_table_lookup(cache->old_properties, property);
    if(old_item) {
        /* if we have an old item, it means that a previous set
         * call hasn't returned yet.  let's cancel that call and
         * throw away the current not-yet-committed value of
         * the property.
         * we also remove the old_item from the pending_calls table
         * so there is no pending item left. */
        if(!g_cancellable_is_cancelled (old_item->cancellable)) {
            g_cancellable_cancel(old_item->cancellable);
            g_hash_table_remove(cache->pending_calls, old_item->cancellable);
            g_object_unref (old_item->cancellable);
            old_item->cancellable = g_cancellable_new();
        }

        if(old_item->variant){
            g_variant_unref(old_item->variant);
            old_item->variant = NULL;
        }
    } else {
        old_item = esconf_cache_old_item_new(cache, property);
        if(item)
            old_item->item = esconf_cache_item_new(item->value, FALSE);
        g_hash_table_insert(cache->old_properties, old_item->property, old_item);
    }

    val = esconf_gvalue_to_gvariant (value);
    if (val) {
        variant = g_variant_new_variant (val);

        esconf_exported_call_set_property ((EsconfExported *)proxy,
                                           cache->channel_name,
                                           property,
                                           variant,
                                           old_item->cancellable,
                                           (GAsyncReadyCallback) esconf_cache_set_property_reply_handler,
                                           old_item);

        old_item->variant = val;
        old_item->pending_calls_count++;

        g_hash_table_insert(cache->pending_calls, old_item->cancellable, old_item);

        if(item)
            esconf_cache_item_update(item, value);
        else {
            item = esconf_cache_item_new(value, FALSE);
            g_tree_insert(cache->properties, g_strdup(property), item);
        }

        esconf_cache_mutex_unlock(cache);
        g_signal_emit(G_OBJECT(cache), signals[SIG_PROPERTY_CHANGED], 0,
                      cache->channel_name, property, value);

        return TRUE;
    }
    esconf_cache_mutex_unlock(cache);
    return FALSE;
}

typedef struct
{
    gchar *property_base;
    gsize property_base_len;
    GSList *matches;
} EsconfCacheRecurseData;

static gboolean
esconf_cache_collect_properties_recursive(gpointer key,
                                          gpointer value,
                                          gpointer user_data)
{
    gchar *property_name = key;
    EsconfCacheRecurseData *rdata = user_data;

    if(!g_ascii_strncasecmp(rdata->property_base, property_name, rdata->property_base_len))
        rdata->matches = g_slist_prepend(rdata->matches, property_name);

    return FALSE;
}

gboolean
esconf_cache_reset(EsconfCache *cache,
                   const gchar *property_base,
                   gboolean recursive,
                   GError **error)
{
    gboolean ret = FALSE;
    GDBusProxy *proxy = _esconf_get_gdbus_proxy();
#if 0
    EsconfCacheOldItem *old_item = NULL;
#endif

    esconf_cache_mutex_lock(cache);

#if 0
    /* it's not really feasible here to look up all the old/new values
     * here, so we're just gonna rely on the normal signals from the
     * daemon to notify us of changes */

    /* can't use the generated dbus-glib binding here cuz we won't
     * get the pending call pointer in the callback */
    old_item = esconf_cache_old_item_new(property_base);
    old_item->call = dbus_g_proxy_begin_call(proxy, "ResetProperty",
                                             esconf_cache_reset_property_reply_handler,
                                             cache, NULL,
                                             G_TYPE_STRING, cache->channel_name,
                                             G_TYPE_STRING, property_base,
                                             G_TYPE_BOOLEAN, recursive,
                                             G_TYPE_INVALID);
    if(old_item->call) {
        g_hash_table_insert(cache->pending_calls, old_item->call, old_item);
        ret = TRUE;
    } else {
        if(error) {
            g_set_error(error, DBUS_GERROR, DBUS_GERROR_FAILED,
                        _("Failed to make ResetProperty DBus call"));
        }
    }
#else
    /* unfortunately, doing the above asynchronously makes
     * esconf_channel_has_property() break, because we have no idea at
     * this point if a reset is going to remove the property or reset
     * it to a default.  so, we have to do this sync.  sad. */

    ret = esconf_exported_call_reset_property_sync ((EsconfExported*)proxy, cache->channel_name,
                                                  property_base, recursive, NULL, error);

    if(ret) {
        /* here we just evict the entry from the cache if we have one.
         * unfortunately i think it's the best we can do here.  this is
         * pretty slow because we have to traverse the entire tree if
         * recursive==TRUE. */

        g_tree_remove(cache->properties, property_base);

        if(recursive) {
            EsconfCacheRecurseData rdata;
            GSList *l;

            rdata.property_base = g_strdup_printf("%s/", property_base);
            rdata.property_base_len = strlen(rdata.property_base);
            rdata.matches = NULL;

            g_tree_foreach(cache->properties,
                           esconf_cache_collect_properties_recursive,
                           &rdata);

            for(l = rdata.matches; l; l = l->next)
                g_tree_remove(cache->properties, l->data);

            g_free(rdata.property_base);
            g_slist_free(rdata.matches);
        }
    }
#endif

    esconf_cache_mutex_unlock(cache);

    return ret;
}

#if 0
void
esconf_cache_set_max_entries(EsconfCache *cache,
                             gint max_entries)
{
    esconf_cache_mutex_lock(cache);
    cache->max_entries = max_entries;
    /* TODO: check tree for eviction */
    esconf_cache_mutex_unlock(cache);
}

gint
esconf_cache_get_max_entries(EsconfCache *cache)
{
    return cache->max_entries;
}

void
esconf_cache_set_max_age(EsconfCache *cache,
                         gint max_age)
{
    esconf_cache_mutex_lock(cache);
    cache->max_age = max_age;
    /* TODO: check tree for eviction */
    esconf_cache_mutex_unlock(cache);
}

gint
esconf_cache_get_max_age(EsconfCache *cache)
{
    return cache->max_age;
}
#endif
