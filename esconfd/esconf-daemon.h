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

#ifndef __ESCONF_DAEMON_H__
#define __ESCONF_DAEMON_H__

#include <glib-object.h>
#include "esconf/esconf-errors.h"

#define ESCONF_TYPE_DAEMON             (esconf_daemon_get_type())
#define ESCONF_DAEMON(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ESCONF_TYPE_DAEMON, EsconfDaemon))
#define ESCONF_IS_DAEMON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESCONF_TYPE_DAEMON))
#define ESCONF_DAEMON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), ESCONF_TYPE_DAEMON, EsconfDaemonClass))
#define ESCONF_IS_DAEMON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), ESCONF_TYPE_DAEMON))
#define ESCONF_DAEMON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), ESCONF_TYPE_DAEMON, EsconfDaemonClass))

G_BEGIN_DECLS

typedef struct _EsconfDaemon         EsconfDaemon;

GType esconf_daemon_get_type(void) G_GNUC_CONST;

EsconfDaemon *esconf_daemon_new_unique(gchar * const *backend_ids,
                                       GError **error);

G_END_DECLS

#endif  /* __ESCONF_DAEMON_H__ */
