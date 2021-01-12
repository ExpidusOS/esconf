/*
 *  esconf
 *
 *  Copyright (c) 2008 Brian Tarricone <bjt23@cornell.edu>
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

#ifndef __ESCONF_GVALUEFUNCS_H__
#define __ESCONF_GVALUEFUNCS_H__

#include <glib-object.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL GType _esconf_gtype_from_string(const gchar *type);
G_GNUC_INTERNAL const gchar *_esconf_string_from_gtype(GType gtype);

G_GNUC_INTERNAL gboolean _esconf_gvalue_from_string(GValue *value,
                                                    const gchar *str);

G_GNUC_INTERNAL gchar *_esconf_string_from_gvalue(GValue *value);

G_GNUC_INTERNAL gboolean _esconf_gvalue_is_equal(const GValue *value1,
                                                 const GValue *value2);

G_GNUC_INTERNAL void _esconf_gvalue_free(GValue *value);

G_GNUC_INTERNAL GPtrArray *esconf_dup_value_array (GPtrArray *arr, gboolean auto_destroy_value);

G_GNUC_INTERNAL GVariant *esconf_basic_gvalue_to_gvariant (const GValue *value);

G_GNUC_INTERNAL GVariant *esconf_gvalue_to_gvariant (const GValue *value);

G_GNUC_INTERNAL gboolean esconf_basic_gvariant_to_gvalue (GVariant *variant, GValue *value);

G_GNUC_INTERNAL GValue * esconf_gvariant_to_gvalue (GVariant *variant);

G_GNUC_INTERNAL GVariant *esconf_hash_to_gvariant (GHashTable *hash);

G_GNUC_INTERNAL GHashTable *esconf_gvariant_to_hash (GVariant *variant);

G_END_DECLS

#endif  /* __ESCONF_GVALUEFUNCS_H__ */
