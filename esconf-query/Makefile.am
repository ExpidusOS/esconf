
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"esconf-query\" \
	-DDATADIR=\"$(datadir)\" \
	-DSRCDIR=\"$(top_srcdir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = esconf-query

esconf_query_SOURCES = \
	main.c

esconf_query_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(LIBEXPIDUS1UTIL_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(PLATFORM_CFLAGS)

esconf_query_LDFLAGS = \
	$(PLATFORM_LDFLAGS)

esconf_query_LDADD = \
	$(top_builddir)/common/libesconf-gvaluefuncs.la \
	$(top_builddir)/esconf/libesconf-0.la \
	$(GLIB_LIBS) \
	$(LIBEXPIDUS1UTIL_LIBS) \
	$(GIO_UNIX_LIBS)
