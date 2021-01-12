/*
 * Copyright (C) 2018 - Ali Abdallah <ali@expidus.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __ESCONF_GSETTINGS_BACKEND_H__
#define __ESCONF_GSETTINGS_BACKEND_H__

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#define ESCONF_GSETTINGS_BACKEND_TYPE (esconf_gsettings_backend_get_type ())
#define ESCONF_GSETTINGS_BACKEND(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ESCONF_GSETTINGS_BACKEND_TYPE, EsconfGsettingsBackend))
#define ESCONF_GSETTINGS_BACKEND_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ESCONF_GSETTINGS_BACKEND_TYPE, EsconfGsettingsBackendClass))
#define IS_ESCONF_GSETTINGS_BACKEND(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ESCONF_GSETTINGS_BACKEND_TYPE))
#define IS_ESCONF_GSETTINGS_BACKEND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ESCONF_GSETTINGS_BACKEND_TYPE))
#define ESCONF_GSETTINGS_BACKEND_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ESCONF_GSETTINGS_BACKEND_TYPE, EsconfGsettingsBackendClass))

G_BEGIN_DECLS

typedef struct _EsconfGsettingsBackend EsconfGsettingsBackend;

typedef struct
{
  GSettingsBackendClass parent_class;

} EsconfGsettingsBackendClass;

GType esconf_gsettings_backend_get_type (void) G_GNUC_CONST;

EsconfGsettingsBackend *esconf_gsettings_backend_new (void);

G_END_DECLS

#endif /* __ESCONF_GSETTINGS_BACKEND_H__ */

