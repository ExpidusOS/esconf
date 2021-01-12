
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLIBESCONF_COMPILATION \
	-DG_LOG_DOMAIN=\"esconf-common\" \
	$(PLATFORM_CPPFLAGS)

noinst_LTLIBRARIES = \
	libesconf-common.la \
	libesconf-gvaluefuncs.la \
	libesconf-dbus.la

libesconf_common_la_SOURCES = \
	esconf-errors.c \
	esconf-marshal.c \
	esconf-marshal.h

libesconf_common_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(PLATFORM_CFLAGS)

libesconf_common_la_LDFLAGS = \
	$(PLATFORM_LDFLAGS)

libesconf_common_la_LIBADD = \
	$(GLIB_LIBS)

libesconf_built_sources = \
	esconf-alias.h \
	esconf-aliasdef.c

libesconf_gvaluefuncs_la_SOURCES = \
	esconf-gvaluefuncs.c \
	esconf-gvaluefuncs.h

libesconf_gvaluefuncs_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(PLATFORM_CFLAGS)

libesconf_gvaluefuncs_la_LDFLAGS = \
	$(PLATFORM_LDFLAGS)

libesconf_gvaluefuncs_la_LIBADD = \
	$(GLIB_LIBS) \
	$(GIO_UNIX_LIBS)


libesconf_dbus_la_SOURCES = \
	esconf-gdbus-bindings.c \
	esconf-gdbus-bindings.h

libesconf_dbus_la_CFLAGS = $(GIO_UNIX_CFLAGS)

libesconf_dbus_la_LIBADD = $(GIO_UNIX_LIBS)
	

BUILT_SOURCES = \
	esconf-marshal.c \
	esconf-marshal.h \
	esconf-gdbus-bindings.c \
	esconf-gdbus-bindings.h

esconf-marshal.h: $(srcdir)/esconf-marshal.list Makefile
	$(AM_V_GEN) glib-genmarshal --prefix=_esconf_marshal $(srcdir)/esconf-marshal.list --header > esconf-marshal.h
esconf-marshal.c: $(srcdir)/esconf-marshal.list Makefile
	$(AM_V_GEN) echo '#include "esconf-marshal.h"' > esconf-marshal.c \
	&& glib-genmarshal --prefix=_esconf_marshal $(srcdir)/esconf-marshal.list --body >> esconf-marshal.c

esconf-gdbus-bindings.h:
esconf-gdbus-bindings.c: esconf-dbus.xml Makefile
	$(AM_V_GEN) $(GDBUS_CODEGEN) \
	--interface-prefix com.expidus.Esconf \
	--annotate "com.expidus.Esconf" "org.gtk.GDBus.C.Name" Exported \
	--c-namespace Esconf   \
	--generate-c-code esconf-gdbus-bindings \
	$<

CLEANFILES = \
	actual-abi \
	expected-abi

DISTCLEANFILES = \
	$(libesconf_built_sources) \
	$(BUILT_SOURCES)

BUILT_SOURCES += \
	$(libesconf_built_sources)

esconf-alias.h: make-exo-alias.pl $(top_srcdir)/esconf/esconf.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-exo-alias.pl <$(top_srcdir)/esconf/esconf.symbols >esconf-alias.h

esconf-aliasdef.c: make-exo-alias.pl $(top_srcdir)/esconf/esconf.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-exo-alias.pl -def <$(top_srcdir)/esconf/esconf.symbols >esconf-aliasdef.c


EXTRA_DIST = \
	make-exo-alias.pl \
	esconf-alias.h \
	esconf-aliasdef.c \
	esconf-common-private.h \
	esconf-dbus.xml \
	esconf-marshal.list \
	esconf-types.c

# required for make distcheck
dist-hook: all
