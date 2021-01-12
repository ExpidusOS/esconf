
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"esconfd\" \
	-DLIBESCONF_COMPILATION \
	-DBINDIR=\"$(bindir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

esconfddir = $(HELPER_PATH_PREFIX)/xfce4/esconf
esconfd_PROGRAMS = esconfd

esconf_backend_sources =

if BUILD_ESCONF_BACKEND_PERCHANNEL_XML
esconf_backend_sources += \
	esconf-backend-perchannel-xml.c \
	esconf-backend-perchannel-xml.h
endif

esconfd_SOURCES = \
	main.c \
	esconf-backend-factory.c \
	esconf-backend-factory.h \
	esconf-backend.c \
	esconf-backend.h \
	esconf-daemon.c \
	esconf-daemon.h \
	esconf-locking-utils.c \
	esconf-locking-utils.h \
	$(esconf_backend_sources) \
	$(top_srcdir)/common/esconf-types.c

esconfd_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(LIBEXPIDUS1UTIL_CFLAGS) \
	$(PLATFORM_CFLAGS)

esconfd_LDFLAGS = \
	$(PLATFORM_LDFLAGS)

if ENABLE_PROFILING
esconfd_CFLAGS += -pg
endif

esconfd_LDADD = \
	$(top_builddir)/common/libesconf-common.la \
	$(top_builddir)/common/libesconf-gvaluefuncs.la \
	$(top_builddir)/common/libesconf-dbus.la \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(LIBEXPIDUS1UTIL_LIBS)

servicedir = $(datadir)/dbus-1/services
service_in_files = com.expidus.Esconf.service.in
service_DATA = $(service_in_files:.service.in=.service)

%.service: %.service.in
	$(AM_V_GEN) sed -e "s,\@HELPERDIR\@,$(HELPER_PATH_PREFIX),g" < $< > $@

CLEANFILES = \
	$(service_DATA)

EXTRA_DIST = \
	esconf-backend-perchannel-xml.c \
	esconf-backend-perchannel-xml.h \
	$(service_in_files)

# required for gtk-doc
dist-hook: all
