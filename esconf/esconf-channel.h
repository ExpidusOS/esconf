/*
 *  esconf
 *
 *  Copyright (c) 2007-2008 Brian Tarricone <bjt23@cornell.edu>
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

#ifndef __ESCONF_CHANNEL_H__
#define __ESCONF_CHANNEL_H__

#if !defined(LIBESCONF_COMPILATION) && !defined(ESCONF_IN_ESCONF_H)
#error "Do not include esconf-channel.h, as this file may change or disappear in the future.  Include <esconf/esconf.h> instead."
#endif

#include <glib-object.h>

#define ESCONF_TYPE_CHANNEL             (esconf_channel_get_type())
#define ESCONF_CHANNEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ESCONF_TYPE_CHANNEL, EsconfChannel))
#define ESCONF_IS_CHANNEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESCONF_TYPE_CHANNEL))
#define ESCONF_CHANNEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), ESCONF_TYPE_CHANNEL, EsconfChannelClass))
#define ESCONF_IS_CHANNEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), ESCONF_TYPE_CHANNEL))
#define ESCONF_CHANNEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), ESCONF_TYPE_CHANNEL, EsconfChannelClass))

G_BEGIN_DECLS

typedef struct _EsconfChannel         EsconfChannel;

GType esconf_channel_get_type(void) G_GNUC_CONST;

EsconfChannel *esconf_channel_get(const gchar *channel_name);

EsconfChannel *esconf_channel_new(const gchar *channel_name) G_GNUC_WARN_UNUSED_RESULT;

EsconfChannel *esconf_channel_new_with_property_base(const gchar *channel_name,
                                                     const gchar *property_base) G_GNUC_WARN_UNUSED_RESULT;

gboolean esconf_channel_has_property(EsconfChannel *channel,
                                     const gchar *property);

gboolean esconf_channel_is_property_locked(EsconfChannel *channel,
                                           const gchar *property);

void esconf_channel_reset_property(EsconfChannel *channel,
                                   const gchar *property_base,
                                   gboolean recursive);

GHashTable *esconf_channel_get_properties(EsconfChannel *channel,
                                          const gchar *property_base) G_GNUC_WARN_UNUSED_RESULT;

/* basic types */

gchar *esconf_channel_get_string(EsconfChannel *channel,
                                 const gchar *property,
                                 const gchar *default_value) G_GNUC_WARN_UNUSED_RESULT;
gboolean esconf_channel_set_string(EsconfChannel *channel,
                                   const gchar *property,
                                   const gchar *value);

gint32 esconf_channel_get_int(EsconfChannel *channel,
                              const gchar *property,
                              gint32 default_value);
gboolean esconf_channel_set_int(EsconfChannel *channel,
                                const gchar *property,
                                gint32 value);

guint32 esconf_channel_get_uint(EsconfChannel *channel,
                                const gchar *property,
                                guint32 default_value);
gboolean esconf_channel_set_uint(EsconfChannel *channel,
                                 const gchar *property,
                                 guint32 value);

guint64 esconf_channel_get_uint64(EsconfChannel *channel,
                                  const gchar *property,
                                  guint64 default_value);
gboolean esconf_channel_set_uint64(EsconfChannel *channel,
                                   const gchar *property,
                                   guint64 value);

gdouble esconf_channel_get_double(EsconfChannel *channel,
                                  const gchar *property,
                                  gdouble default_value);
gboolean esconf_channel_set_double(EsconfChannel *channel,
                                   const gchar *property,
                                   gdouble value);

gboolean esconf_channel_get_bool(EsconfChannel *channel,
                                 const gchar *property,
                                 gboolean default_value);
gboolean esconf_channel_set_bool(EsconfChannel *channel,
                                 const gchar *property,
                                 gboolean value);

/* this is just convenience API for the array stuff, where
 * all the values are G_TYPE_STRING */
gchar **esconf_channel_get_string_list(EsconfChannel *channel,
                                       const gchar *property) G_GNUC_WARN_UNUSED_RESULT;
gboolean esconf_channel_set_string_list(EsconfChannel *channel,
                                        const gchar *property,
                                        const gchar * const *values);

/* really generic API - can set some value types that aren't
 * supported by the basic type API, e.g., char, signed short,
 * unsigned int, etc.  no, you can't set arbitrary GTypes. */
gboolean esconf_channel_get_property(EsconfChannel *channel,
                                     const gchar *property,
                                     GValue *value);
gboolean esconf_channel_set_property(EsconfChannel *channel,
                                     const gchar *property,
                                     const GValue *value);

/* array types - arrays can be made up of values of arbitrary
 * (and mixed) types, even some not supported by the basic
 * type API */

gboolean esconf_channel_get_array(EsconfChannel *channel,
                                  const gchar *property,
                                  GType first_value_type,
                                  ...);
gboolean esconf_channel_get_array_valist(EsconfChannel *channel,
                                         const gchar *property,
                                         GType first_value_type,
                                         va_list var_args);
GPtrArray *esconf_channel_get_arrayv(EsconfChannel *channel,
                                     const gchar *property) G_GNUC_WARN_UNUSED_RESULT;

gboolean esconf_channel_set_array(EsconfChannel *channel,
                                  const gchar *property,
                                  GType first_value_type,
                                  ...);
gboolean esconf_channel_set_array_valist(EsconfChannel *channel,
                                         const gchar *property,
                                         GType first_value_type,
                                         va_list var_args);
gboolean esconf_channel_set_arrayv(EsconfChannel *channel,
                                   const gchar *property,
                                   GPtrArray *values);

/* struct types */

gboolean esconf_channel_get_named_struct(EsconfChannel *channel,
                                         const gchar *property,
                                         const gchar *struct_name,
                                         gpointer value_struct);
gboolean esconf_channel_set_named_struct(EsconfChannel *channel,
                                         const gchar *property,
                                         const gchar *struct_name,
                                         gpointer value_struct);

gboolean esconf_channel_get_struct(EsconfChannel *channel,
                                   const gchar *property,
                                   gpointer value_struct,
                                   GType first_member_type,
                                   ...);
gboolean esconf_channel_get_struct_valist(EsconfChannel *channel,
                                          const gchar *property,
                                          gpointer value_struct,
                                          GType first_member_type,
                                          va_list var_args);
gboolean esconf_channel_get_structv(EsconfChannel *channel,
                                    const gchar *property,
                                    gpointer value_struct,
                                    guint n_members,
                                    GType *member_types);

gboolean esconf_channel_set_struct(EsconfChannel *channel,
                                   const gchar *property,
                                   const gpointer value_struct,
                                   GType first_member_type,
                                   ...);
gboolean esconf_channel_set_struct_valist(EsconfChannel *channel,
                                          const gchar *property,
                                          const gpointer value_struct,
                                          GType first_member_type,
                                          va_list var_args);
gboolean esconf_channel_set_structv(EsconfChannel *channel,
                                    const gchar *property,
                                    const gpointer value_struct,
                                    guint n_members,
                                    GType *member_types);

#if 0  /* future (maybe) */

//gboolean esconf_channel_begin_transaction(EsconfChannel *channel);
//gboolean esconf_channel_commit_transaction(EsconfChannel *channel);
//void esconf_channel_cancel_transaction(EsconfChannel *channel);

#endif

G_END_DECLS

#endif  /* __ESCONF_CHANNEL_H__ */
