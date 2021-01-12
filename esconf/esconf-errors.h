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

#ifndef __ESCONF_ERRORS_H__
#define __ESCONF_ERRORS_H__

#if !defined(LIBESCONF_COMPILATION) && !defined(ESCONF_IN_ESCONF_H)
#error "Do not include esconf-errors.h, as this file may change or disappear in the future.  Include <esconf/esconf.h> instead."
#endif

#include <glib-object.h>

#define ESCONF_TYPE_ERROR  (esconf_error_get_type())
#define ESCONF_ERROR       (esconf_get_error_quark())

G_BEGIN_DECLS

/**
 * EsconfError:
 * @ESCONF_ERROR_UNKNOWN: An unknown error occurred
 * @ESCONF_ERROR_CHANNEL_NOT_FOUND: The specified channel does not exist
 * @ESCONF_ERROR_PROPERTY_NOT_FOUND: The specified property does not exist on the channel
 * @ESCONF_ERROR_READ_FAILURE: There was a failure reading from the configuration store
 * @ESCONF_ERROR_WRITE_FAILURE: There was a failure writing to the configuration store
 * @ESCONF_ERROR_PERMISSION_DENIED: The user is not allowed to read or write to the channel or property
 * @ESCONF_ERROR_INTERNAL_ERROR: An internal error (likely a bug in esconf) occurred
 * @ESCONF_ERROR_NO_BACKEND: No backends were found, or those found could not be loaded
 * @ESCONF_ERROR_INVALID_PROPERTY: The property name specified was invalid
 * @ESCONF_ERROR_INVALID_CHANNEL: The channel name specified was invalid
 *
 * An enumeration listing the different kinds of errors under the ESCONF_ERROR domain.
 *
 **/
typedef enum
{
    ESCONF_ERROR_UNKNOWN = 0,
    ESCONF_ERROR_CHANNEL_NOT_FOUND,
    ESCONF_ERROR_PROPERTY_NOT_FOUND,
    ESCONF_ERROR_READ_FAILURE,
    ESCONF_ERROR_WRITE_FAILURE,
    ESCONF_ERROR_PERMISSION_DENIED,
    ESCONF_ERROR_INTERNAL_ERROR,
    ESCONF_ERROR_NO_BACKEND,
    ESCONF_ERROR_INVALID_PROPERTY,
    ESCONF_ERROR_INVALID_CHANNEL,
} EsconfError;

GType esconf_error_get_type(void) G_GNUC_CONST;
GQuark esconf_get_error_quark(void);

G_END_DECLS

#endif  /* __ESCONF_ERRORS_H__ */
