/*
 *  esconfd
 *
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

#include "esconf-backend.h"


static void esconf_backend_base_init(gpointer g_class);

static gboolean esconf_property_is_valid(const gchar *property,
                                         GError **error);
static gboolean esconf_channel_is_valid(const gchar *channel,
                                        GError **error);

/**
 * SECTION:esconf-backend
 * @title: Esconf Backend
 * @short_description: Interface for configuration store backends
 *
 *  EsconfBackend is an abstract interface that allows the Esconf Daemon
 *  to  use different backends for storing configuration data.
 *  These backends can be flat text or binary files, a database,
 *  or just about anything one could think of to store data.
 **/

/**
 * EsconfBackendInterface:
 * @parent: GObject interface parent.
 * @initialize: See esconf_backend_initialize().
 * @set: See esconf_backend_set().
 * @get: See esconf_backend_get().
 * @get_all: See esconf_backend_get_all().
 * @exists: See esconf_backend_exists().
 * @reset: See esconf_backend_reset().
 * @list_channels: See esconf_backend_list_channels().
 * @is_property_locked: See esconf_backend_is_property_locked().
 * @flush: See esconf_backend_flush().
 * @register_property_changed_func: See esconf_backend_register_property_changed_func().
 * @_xb_reserved0: Reserved for future expansion.
 * @_xb_reserved1: Reserved for future expansion.
 * @_xb_reserved2: Reserved for future expansion.
 * @_xb_reserved3: Reserved for future expansion.
 *
 * An interface for implementing pluggable configuration store backends
 * into the Esconf Daemon.
 *
 * See the #EsconfBackend function documentation for a description of what
 * each virtual function in #EsconfBackendInterface should do.
 **/


/**
 * EsconfBackend:
 *
 * An instance of a class implementing a #EsconfBackendInterface.
 **/


GType
esconf_backend_get_type(void)
{
    static GType backend_type = 0;
    
    if(!backend_type) {
        static const GTypeInfo backend_info = {
            sizeof(EsconfBackendInterface),
            esconf_backend_base_init,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            NULL,
        };
        
        backend_type = g_type_register_static(G_TYPE_INTERFACE,"EsconfBackend",
                                              &backend_info, 0);
        g_type_interface_add_prerequisite(backend_type, G_TYPE_OBJECT);
    }
    
    return backend_type;
}



static void
esconf_backend_base_init(gpointer g_class)
{
    static gboolean _inited = FALSE;
    
    if(!_inited) {
        _inited = TRUE;
    }
}



static gboolean
esconf_property_is_valid(const gchar *property,
                         GError **error)
{
    const gchar *p = property;

    if(!p || *p != '/') {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_INVALID_PROPERTY,
                        _("Property names must start with a '/' character"));
        }
        return FALSE;
    }

    p++;
    if(!*p) {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_INVALID_PROPERTY,
                        _("The root element ('/') is not a valid property name"));
        }
        return FALSE;
    }

    while(*p) {
        if(!(*p >= 'A' && *p <= 'Z') && !(*p >= 'a' && *p <= 'z')
           && !(*p >= '0' && *p <= '9')
           && *p != '_' && *p != '-' && *p != '/' && *p != '{' && *p != '}'
           && !(*p == '<' || *p == '>') && *p != '|' && *p != ','
           && *p != '[' && *p != ']' && *p != '.' && *p != ':')
        {
            if(error) {
                g_set_error(error, ESCONF_ERROR,
                            ESCONF_ERROR_INVALID_PROPERTY,
                            _("Property names can only include the ASCII characters A-Z, a-z, 0-9, '_', '-', ':', '.', ',', '[', ']', '{', '}', '<' and '>', as well as '/' as a separator"));
            }
            return FALSE;
        }

        if('/' == *p && '/' == *(p-1)) {
            if(error) {
                g_set_error(error, ESCONF_ERROR,
                            ESCONF_ERROR_INVALID_PROPERTY,
                            _("Property names cannot have two or more consecutive '/' characters"));
            }
            return FALSE;
        }

        p++;
    }

    if(*(p-1) == '/') {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_INVALID_PROPERTY,
                        _("Property names cannot end with a '/' character"));
        }
        return FALSE;
    }

    return TRUE;
}

static gboolean
esconf_channel_is_valid(const gchar *channel,
                        GError **error)
{
    const gchar *p = channel;

    if(!p || !*p) {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_INVALID_CHANNEL,
                        _("Channel name cannot be an empty string"));
        }
        return FALSE;
    }

    while(*p) {
        if(!(*p >= 'A' && *p <= 'Z') && !(*p >= 'a' && *p <= 'z')
           && !(*p >= '0' && *p <= '9')
           && *p != '_' && *p != '-' && *p != '{' && *p != '}'
           && *p != '|' && *p != ','
           && *p != '[' && *p != ']' && *p != '.' && *p != ':')
        {
            if(error) {
                g_set_error(error, ESCONF_ERROR,
                            ESCONF_ERROR_INVALID_CHANNEL,
                            _("Channel names can only include the ASCII characters A-Z, a-z, 0-9, '{', '}', '|', ']', '[', ':', ',', '.', '_', and '-'"));
            }
            return FALSE;
        }
        p++;
    }

    return TRUE;
}



/**
 * esconf_backend_initialize:
 * @backend: The #EsconfBackend.
 * @error: An error return.
 *
 * Does any pre-initialization that the backend needs to function.
 *
 * Return value: The backend should return %TRUE if initialization
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_initialize(EsconfBackend *backend,
                          GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->initialize
                                      && (!error || !*error), FALSE);
    
    return iface->initialize(backend, error);
}

/**
 * esconf_backend_set:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property: A property name.
 * @value: A value.
 * @error: An error return.
 *
 * Sets the variant @value for @property on @channel.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_set(EsconfBackend *backend,
                   const gchar *channel,
                   const gchar *property,
                   const GValue *value,
                   GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->set && channel && *channel
                                      && property && *property
                                      && value && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;
    if(!esconf_property_is_valid(property, error))
        return FALSE;
    
    return iface->set(backend, channel, property, value, error);
}

/**
 * esconf_backend_get:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property: A property name.
 * @value: A #GValue return.
 * @error: An error return.
 *
 * Gets the value of @property on @channel and stores it in @value.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_get(EsconfBackend *backend,
                   const gchar *channel,
                   const gchar *property,
                   GValue *value,
                   GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->get && channel && *channel
                                      && property && *property
                                      && value && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;
    if(!esconf_property_is_valid(property, error))
        return FALSE;
    
    return iface->get(backend, channel, property, value, error);
}

/**
 * esconf_backend_get_all:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property_base: The base of properties to return.
 * @properties: A #GHashTable.
 * @error: An error return.
 *
 * Gets multiple properties and values on @channel and stores them in
 * @properties, which is already initialized to hold #gchar* keys and
 * #GValue<!-- -->* values.  The @property_base parameter can be
 * used to limit the retrieval to a sub-tree of the property tree.
 *
 * A value of the empty string ("") or forward slash ("/") for
 * @property_base indicates the entire channel.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_get_all(EsconfBackend *backend,
                       const gchar *channel,
                       const gchar *property_base,
                       GHashTable *properties,
                       GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->get_all && channel
                                      && *channel && property_base
                                      && properties
                                      && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;
    if(*property_base && !(property_base[0] == '/' && !property_base[1])
       && !esconf_property_is_valid(property_base, error))
    {
        return FALSE;
    }

    return iface->get_all(backend, channel, property_base, properties, error);
}

/**
 * esconf_backend_exists:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property: A property name.
 * @exists: A boolean return.
 * @error: An error return.
 *
 * Checks to see if @property exists on @channel, and stores %TRUE or
 * %FALSE in @exists.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_exists(EsconfBackend *backend,
                      const gchar *channel,
                      const gchar *property,
                      gboolean *exists,
                      GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->exists && channel
                                      && *channel && property && *property
                                      && exists
                                      && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;
    if(!esconf_property_is_valid(property, error))
        return FALSE;
 
    return iface->exists(backend, channel, property, exists, error);
}

/**
 * esconf_backend_reset:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property: A property name.
 * @recursive: Whether or not the reset is recursive.
 * @error: An error return.
 *
 * Resets the property identified by @property from @channel.
 * If @recursive is %TRUE, all sub-properties of @property will be
 * reset as well.  If the empty string ("") or a forward slash ("/")
 * is specified for @property, the entire channel will be reset.
 *
 * If none of the properties specified are locked or have root-owned
 * system-wide defaults set, this effectively removes the properties
 * from the configuration store entirely.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_reset(EsconfBackend *backend,
                     const gchar *channel,
                     const gchar *property,
                     gboolean recursive,
                     GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->reset && channel
                                      && *channel && property && *property
                                      && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;

    if(!recursive && (!*property || (property[0] == '/' && !property[1]))) {
        if(error) {
            g_set_error(error, ESCONF_ERROR, ESCONF_ERROR_INVALID_PROPERTY,
                        _("The property name can only be empty or \"/\" if a recursive reset was specified"));
        }
        return FALSE;
    }
    if(*property && !(property[0] == '/' && !property[1])
       && !esconf_property_is_valid(property, error))
    {
        return FALSE;
    }

    return iface->reset(backend, channel, property, recursive, error);
}

/**
 * esconf_backend_list_channels:
 * @backend: The #EsconfBackend.
 * @channels: A pointer to a #GSList head.
 * @error: An error return.
 *
 * Instructs the backend to return a list of channels with
 * configuration data stored in the configuration store.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_list_channels(EsconfBackend *backend,
                             GSList **channels,
                             GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);

    esconf_backend_return_val_if_fail(iface && iface->list_channels
                                      && (!error || !*error), FALSE);

    return iface->list_channels(backend, channels, error);
}

/**
 * esconf_backend_is_property_locked:
 * @backend: The #EsconfBackend.
 * @channel: A channel name.
 * @property: A property name.
 * @locked: A boolean return.
 * @error: An error return.
 *
 * Queries whether or not the property on @channel is locked by
 * system policy.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_is_property_locked(EsconfBackend *backend,
                                  const gchar *channel,
                                  const gchar *property,
                                  gboolean *locked,
                                  GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);

    esconf_backend_return_val_if_fail(iface && iface->is_property_locked
                                      && (!error || !*error), FALSE);
    if(!esconf_channel_is_valid(channel, error))
        return FALSE;
    if(!esconf_property_is_valid(property, error))
        return FALSE;

    return iface->is_property_locked(backend, channel, property, locked, error);
}

/**
 * esconf_backend_flush
 * @backend: The #EsconfBackend.
 * @error: An error return.
 *
 * For backends that support persistent storage, ensures that all
 * configuration data stored in memory is saved to persistent storage.
 *
 * Return value: The backend should return %TRUE if the operation
 *               was successful, or %FALSE otherwise.  On %FALSE,
 *               @error should be set to a description of the failure.
 **/
gboolean
esconf_backend_flush(EsconfBackend *backend,
                     GError **error)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);
    
    esconf_backend_return_val_if_fail(iface && iface->flush
                                      && (!error || !*error), FALSE);
    
    return iface->flush(backend, error);
}

/**
 * esconf_backend_register_property_changed_func:
 * @backend: The #EsconfBackend.
 * @func: A function of type #EsconfPropertyChangedFunc.
 * @user_data: Arbitrary caller-supplied data.
 *
 * Registers a function to be called when a property changes.  The
 * backend implementation should keep a pointer to @func and @user_data
 * and call @func when a property in the configuration store changes.
 **/
void
esconf_backend_register_property_changed_func(EsconfBackend *backend,
                                              EsconfPropertyChangedFunc func,
                                              gpointer user_data)
{
    EsconfBackendInterface *iface = ESCONF_BACKEND_GET_INTERFACE(backend);

    g_return_if_fail(iface);
    if(!iface->register_property_changed_func)
        return;

    iface->register_property_changed_func(backend, func, user_data);
}
