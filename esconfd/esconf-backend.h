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

#ifndef __ESCONF_BACKEND_H__
#define __ESCONF_BACKEND_H__

#include <glib-object.h>

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <esconf/esconf-errors.h>
#include "esconf-daemon.h"

#define ESCONF_TYPE_BACKEND                (esconf_backend_get_type())
#define ESCONF_BACKEND(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), ESCONF_TYPE_BACKEND, EsconfBackend))
#define ESCONF_IS_BACKEND(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESCONF_TYPE_BACKEND))
#define ESCONF_BACKEND_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), ESCONF_TYPE_BACKEND, EsconfBackendInterface))

#define esconf_backend_return_val_if_fail(cond, val)  G_STMT_START{ \
    if(!(cond)) { \
        if(error) { \
            g_set_error(error, ESCONF_ERROR, \
                        ESCONF_ERROR_INTERNAL_ERROR, \
                        _("An internal error occurred; this is probably a bug")); \
        } \
        g_return_val_if_fail((cond), (val)); \
        return (val);  /* ensure return even if G_DISABLE_CHECKS */ \
    } \
}G_STMT_END

G_BEGIN_DECLS

typedef struct _EsconfBackend           EsconfBackend;
typedef struct _EsconfBackendInterface  EsconfBackendInterface;

typedef void (*EsconfPropertyChangedFunc)(EsconfBackend *backend,
                                          const gchar *channel,
                                          const gchar *property,
                                          gpointer user_data);

struct _EsconfBackendInterface
{
    GTypeInterface parent;
    
    gboolean (*initialize)(EsconfBackend *backend,
                           GError **error);
    
    gboolean (*set)(EsconfBackend *backend,
                    const gchar *channel,
                    const gchar *property,
                    const GValue *value,
                    GError **error);
    
    gboolean (*get)(EsconfBackend *backend,
                    const gchar *channel,
                    const gchar *property,
                    GValue *value,
                    GError **error);
    
    gboolean (*get_all)(EsconfBackend *backend,
                        const gchar *channel,
                        const gchar *property_base,
                        GHashTable *properties,
                        GError **error);
    
    gboolean (*exists)(EsconfBackend *backend,
                       const gchar *channel,
                       const gchar *property,
                       gboolean *exists,
                       GError **error);
    
    gboolean (*reset)(EsconfBackend *backend,
                      const gchar *channel,
                      const gchar *property,
                      gboolean recursive,
                      GError **error);

    gboolean (*list_channels)(EsconfBackend *backend,
                              GSList **channels,
                              GError **error);

    gboolean (*is_property_locked)(EsconfBackend *backend,
                                   const gchar *channel,
                                   const gchar *property,
                                   gboolean *locked,
                                   GError **error);
    
    gboolean (*flush)(EsconfBackend *backend,
                      GError **error);

    void (*register_property_changed_func)(EsconfBackend *backend,
                                           EsconfPropertyChangedFunc func,
                                           gpointer user_data);
    
    /*< reserved for future expansion >*/
    void (*_xb_reserved0)();
    void (*_xb_reserved1)();
    void (*_xb_reserved2)();
    void (*_xb_reserved3)();
};

GType esconf_backend_get_type(void) G_GNUC_CONST;

gboolean esconf_backend_initialize(EsconfBackend *backend,
                                   GError **error);

gboolean esconf_backend_set(EsconfBackend *backend,
                            const gchar *channel,
                            const gchar *property,
                            const GValue *value,
                            GError **error);

gboolean esconf_backend_get(EsconfBackend *backend,
                            const gchar *channel,
                            const gchar *property,
                            GValue *value,
                            GError **error);

gboolean esconf_backend_get_all(EsconfBackend *backend,
                                const gchar *channel,
                                const gchar *property_base,
                                GHashTable *properties,
                                GError **error);

gboolean esconf_backend_exists(EsconfBackend *backend,
                               const gchar *channel,
                               const gchar *property,
                               gboolean *exists,
                               GError **error);

gboolean esconf_backend_reset(EsconfBackend *backend,
                              const gchar *channel,
                              const gchar *property,
                              gboolean recursive,
                              GError **error);

gboolean esconf_backend_list_channels(EsconfBackend *backend,
                                      GSList **channels,
                                      GError **error);

gboolean esconf_backend_is_property_locked(EsconfBackend *backend,
                                           const gchar *channel,
                                           const gchar *property,
                                           gboolean *locked,
                                           GError **error);

gboolean esconf_backend_flush(EsconfBackend *backend,
                              GError **error);

void esconf_backend_register_property_changed_func(EsconfBackend *backend,
                                                   EsconfPropertyChangedFunc func,
                                                   gpointer user_data);

G_END_DECLS

#endif  /* __ESCONF_BACKEND_H__ */
