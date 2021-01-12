
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLIBESCONF_COMPILATION \
	-DG_LOG_DOMAIN=\"esconf\" \
	$(PLATFORM_CPPFLAGS)

lib_LTLIBRARIES = libesconf-0.la

libesconfincludedir = $(includedir)/expidus1/esconf-$(LIBESCONF_VERSION_API)/esconf
libesconfinclude_HEADERS = \
	esconf-binding.h \
	esconf-channel.h \
	esconf-errors.h \
	esconf-types.h \
	esconf.h

libesconf_0_la_SOURCES = \
	$(libesconfinclude_HEADERS) \
	esconf-binding.c \
	esconf-cache.c \
	esconf-cache.h \
	esconf-channel.c \
	esconf-private.h \
	esconf.c \
	$(top_srcdir)/common/esconf-types.c

libesconf_0_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
        $(GIO_UNIX_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(PLATFORM_CFLAGS)

libesconf_0_la_LDFLAGS = \
	-export-dynamic \
	-version-info $(LIBESCONF_VERINFO) \
	-export-symbols-regex "^[^_].*" \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

libesconf_0_la_LIBADD = \
	$(top_builddir)/common/libesconf-common.la \
	$(top_builddir)/common/libesconf-gvaluefuncs.la \
	$(top_builddir)/common/libesconf-dbus.la \
	$(GIO_LIBS) \
        $(GIO_UNIX_LIBS) \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libesconf-0.pc


EXTRA_DIST = \
	abicheck.sh \
	esconf.symbols \
	libesconf-0.deps

# required for gtk-doc
dist-hook: all

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ENV = CC="$(CC)"
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(srcdir) \
	--warn-all \
	-DLIBESCONF_COMPILATION \
	--symbol-prefix=esconf_g_ \
	--symbol-prefix=esconf_get_ \
	--symbol-prefix=esconf_ \
	--c-include=esconf/esconf.h
INTROSPECTION_COMPILER_ARGS = \
	--includedir=$(srcdir) \
	--includedir=.

if HAVE_INTROSPECTION
introspection_sources = $(filter-out %private.h %cache.h, $(libesconf_0_la_SOURCES))

Esconf-0.gir: libesconf-0.la Makefile
Esconf_0_gir_INCLUDES = GObject-2.0 GLib-2.0 Gio-2.0
Esconf_0_gir_CFLAGS = $(AM_CPPFLAGS)
Esconf_0_gir_LIBS = libesconf-0.la
Esconf_0_gir_FILES = $(introspection_sources)
Esconf_0_gir_EXPORT_PACKAGES = libesconf-0
INTROSPECTION_GIRS += Esconf-0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

DISTCLEANFILES = $(gir_DATA) $(typelib_DATA)
endif

if ENABLE_VAPIGEN
-include $(VAPIGEN_MAKEFILE)

libesconf-0.vapi: Esconf-0.gir libesconf-0.deps

VAPIGEN_VAPIS = libesconf-0.vapi
VAPIGEN_GIRDIRS = $(prefix)/share/gir-1.0

libesconf_0_vapi_DEPS = gobject-2.0 glib-2.0 gio-2.0
libesconf_0_vapi_FILES = Esconf-0.gir

vapidir = $(datadir)/vala/vapi
vapi_DATA = $(VAPIGEN_VAPIS) $(VAPIGEN_VAPIS:.vapi=.deps)
endif

if HAVE_GNUC_VISIBILITY
TESTS = abicheck.sh
endif
