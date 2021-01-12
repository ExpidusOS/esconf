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

#include <gio/gio.h>

#include "esconf/esconf-errors.h"
#include "esconf-alias.h"

/**
 * SECTION:esconf-errors
 * @title: Error Reporting
 * @short_description: Esconf library and daemon error descriptions
 *
 * Both the Esconf daemon and library provide error information via the use of #GError
 **/


static const GDBusErrorEntry esconf_daemon_dbus_error_entries[] = 
{
    { ESCONF_ERROR_UNKNOWN, "com.expidus.Esconf.Error.Unknown" },
    { ESCONF_ERROR_CHANNEL_NOT_FOUND, "com.expidus.Esconf.Error.ChannelNotFound" },
    { ESCONF_ERROR_PROPERTY_NOT_FOUND, "com.expidus.Esconf.Error.PropertyNotFound" },
    { ESCONF_ERROR_READ_FAILURE, "com.expidus.Esconf.Error.ReadFailure" },
    { ESCONF_ERROR_WRITE_FAILURE, "com.expidus.Esconf.Error.WriteFailure" },
    { ESCONF_ERROR_PERMISSION_DENIED, "com.expidus.Esconf.Error.PermissionDenied" },
    { ESCONF_ERROR_INTERNAL_ERROR, "com.expidus.Esconf.Error.InternalError" },
    { ESCONF_ERROR_NO_BACKEND, "com.expidus.Esconf.Error.NoBackend" },
    { ESCONF_ERROR_INVALID_PROPERTY, "com.expidus.Esconf.Error.InvalidProperty" },
    { ESCONF_ERROR_INVALID_CHANNEL, "com.expidus.Esconf.Error.InvalidChannel" },
};

/**
 * ESCONF_ERROR:
 *
 * The #GError error domain for Esconf.
 **/



/**
 * ESCONF_TYPE_ERROR:
 *
 * An enum GType for Esconf errors.
 **/


GQuark
esconf_get_error_quark(void)
{
    static volatile gsize quark_volatile = 0;
    
    g_dbus_error_register_error_domain ("esconf_daemon_error",
                                        &quark_volatile,
                                        esconf_daemon_dbus_error_entries,
                                        G_N_ELEMENTS (esconf_daemon_dbus_error_entries));
    
    return quark_volatile;
}

/* unfortunately glib-mkenums can't generate types that are compatible with
 * dbus error names -- the 'nick' value is used, which can have dashes in it,
 * which dbus doesn't like. */


GType
esconf_error_get_type(void)
{
    static GType type = 0;
    
    if(!type) {
        static const GEnumValue values[] = {
            { ESCONF_ERROR_UNKNOWN, "ESCONF_ERROR_UNKNOWN", "Unknown" },
            { ESCONF_ERROR_CHANNEL_NOT_FOUND, "ESCONF_ERROR_CHANNEL_NOT_FOUND", "ChannelNotFound" },
            { ESCONF_ERROR_PROPERTY_NOT_FOUND, "ESCONF_ERROR_PROPERTY_NOT_FOUND", "PropertyNotFound" },
            { ESCONF_ERROR_READ_FAILURE, "ESCONF_ERROR_READ_FAILURE", "ReadFailure" },
            { ESCONF_ERROR_WRITE_FAILURE, "ESCONF_ERROR_WRITE_FAILURE", "WriteFailure" },
            { ESCONF_ERROR_PERMISSION_DENIED, "ESCONF_ERROR_PERMISSION_DENIED", "PermissionDenied" },
            { ESCONF_ERROR_INTERNAL_ERROR, "ESCONF_ERROR_INTERNAL_ERROR", "InternalError" },
            { ESCONF_ERROR_NO_BACKEND, "ESCONF_ERROR_NO_BACKEND", "NoBackend" },
            { ESCONF_ERROR_INVALID_PROPERTY, "ESCONF_ERROR_INVALID_PROPERTY", "InvalidProperty" },
            { ESCONF_ERROR_INVALID_CHANNEL, "ESCONF_ERROR_INVALID_CHANNEL", "InvalidChannel" },
            { 0, NULL, NULL }
        };
        
        type = g_enum_register_static("EsconfError", values);
    }
    
    return type;
}



#define __ESCONF_ERRORS_C__
#include "esconf-aliasdef.c"
