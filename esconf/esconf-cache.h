/*
 *  esconf
 *
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

#ifndef __ESCONF_CACHE_H__
#define __ESCONF_CACHE_H__

#include <glib-object.h>

#define ESCONF_TYPE_CACHE             (esconf_cache_get_type())
#define ESCONF_CACHE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ESCONF_TYPE_CACHE, EsconfCache))
#define ESCONF_IS_CACHE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESCONF_TYPE_CACHE))
#define ESCONF_CACHE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), ESCONF_TYPE_CACHE, EsconfCacheClass))
#define ESCONF_IS_CACHE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), ESCONF_TYPE_CACHE))
#define ESCONF_CACHE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), ESCONF_TYPE_CACHE, EsconfCacheClass))

G_BEGIN_DECLS

typedef struct _EsconfCache         EsconfCache;

G_GNUC_INTERNAL
GType esconf_cache_get_type(void) G_GNUC_CONST;

G_GNUC_INTERNAL
EsconfCache *esconf_cache_new(const gchar *channel_name) G_GNUC_MALLOC;

G_GNUC_INTERNAL
gboolean esconf_cache_prefetch(EsconfCache *cache,
                               const gchar *property_base,
                               GError **error);

G_GNUC_INTERNAL
gboolean esconf_cache_lookup(EsconfCache *cache,
                             const gchar *property,
                             GValue *value,
                             GError **error);

G_GNUC_INTERNAL
gboolean esconf_cache_set(EsconfCache *cache,
                          const gchar *property,
                          const GValue *value,
                          GError **error);

G_GNUC_INTERNAL
gboolean esconf_cache_reset(EsconfCache *cache,
                            const gchar *property_base,
                            gboolean recursive,
                            GError **error);
#if 0
G_GNUC_INTERNAL
void esconf_cache_set_max_entries(EsconfCache *cache,
                                  gint max_entries);
G_GNUC_INTERNAL
gint esconf_cache_get_max_entries(EsconfCache *cache);

G_GNUC_INTERNAL
void esconf_cache_set_max_age(EsconfCache *cache,
                              gint max_age);
G_GNUC_INTERNAL
gint esconf_cache_get_max_age(EsconfCache *cache);
#endif
G_END_DECLS

#endif  /* __ESCONF_CACHE_H__ */
