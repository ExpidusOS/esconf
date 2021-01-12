/*
 *  esconf
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __ESCONF_TESTS_COMMON_H__
#define __ESCONF_TESTS_COMMON_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib.h>
#include <gio/gio.h>
#include <esconf/esconf.h>

#define TEST_CHANNEL_NAME  "test-channel"
#define WAIT_TIMEOUT       15

#define TEST_OPERATION(x) G_STMT_START{ \
    if(!(x)) { \
        g_critical("Test failed: " # x); \
        esconf_tests_end(); \
        return 1; \
    } \
}G_STMT_END

/* don't use static to avoid compiler warnings in tests that don't use
 * all of them */
const gchar *test_string_property = "/test/stringtest/string";
const gchar *test_string = "teststring";
const gchar *test_strlist_property = "/test/stringtest/strlist";
const gchar *test_strlist[] = { "teststring1", "teststring2", NULL };
const gchar *test_int_property = "/test/inttest/int";
const gint test_int = 42;
const gchar *test_uint64_property = "/test/uint64test/uint64";
const guint64 test_uint64 = 42000000000LL;
const gchar *test_double_property = "/test/doubletest/double";
const gdouble test_double = 42.4242;
const gchar *test_bool_property = "/test/booltest/bool";
const gboolean test_bool = TRUE;
const gchar *test_array_property = "/test/arrayprop";

static void esconf_tests_end();

static gboolean
esconf_tests_start(void)
{
    GDBusConnection *conn;
    GDBusMessage *msg, *ret;
    GTimeVal start, now;
    GError *error = NULL;

    /* wait until esconfd finishes starting */
    conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if(!conn) {
        g_critical("Failed to connect to D-Bus: %s", error->message);
        g_error_free (error);
        esconf_tests_end();
        return FALSE;
    }
    msg = g_dbus_message_new_method_call("com.expidus.EsconfTest",
                                         "/com/expidus/Esconf",
                                         "org.freedesktop.DBus.Peer",
                                         "Ping");
    g_get_current_time(&start);
    while(!(ret = g_dbus_connection_send_message_with_reply_sync(conn,
                                                                 msg, 
                                                                 G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                                                 -1, NULL, NULL, NULL)))
    {
        g_get_current_time(&now);
        if(now.tv_sec - start.tv_sec > WAIT_TIMEOUT) {
            g_critical("esconfd failed to start after %d seconds", WAIT_TIMEOUT);
            g_object_unref (msg);
            esconf_tests_end();
            return FALSE;
        }
    }
    if (g_dbus_message_get_message_type(ret) != G_DBUS_MESSAGE_TYPE_METHOD_RETURN)
    {
        g_critical("esconfd is not running and can not be autostarted");
        g_object_unref (msg);
        g_object_unref (ret);
        esconf_tests_end();
        return FALSE;
    }
    g_object_unref (msg);
    g_object_unref (ret);

    if(!esconf_init(&error)) {
        g_critical("Failed to init libesconf: %s", error->message);
        g_error_free(error);
        esconf_tests_end();
        return FALSE;
    }
    
    return TRUE;
}

static void
esconf_tests_end(void)
{
    esconf_shutdown();
}

#endif  /* __ESCONF_TESTS_COMMON_H__ */
