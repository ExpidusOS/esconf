giomodule_LTLIBRARIES= libesconfgsettingsbackend.la

giomoduledir = $(GIO_MODULE_DIR)

libesconfgsettingsbackend_la_SOURCES =		\
	esconf-giomodule.c			\
	esconf-gsettings-backend.c		\
	esconf-gsettings-backend.h

libesconfgsettingsbackend_la_CFLAGS =		\
	-I$(top_srcdir) 			\
	-I$(top_builddir) 			\
	$(GIO_CFLAGS)				\
	$(GMODULE_CFLAGS)

libesconfgsettingsbackend_la_LDFLAGS =		\
	-export-dynamic				\
	-avoid-version				\
	-module 				\
	-export-symbols-regex "^[^_].*"

libesconfgsettingsbackend_la_LIBADD =		\
	$(top_builddir)/esconf/libesconf-0.la 	\
	$(top_builddir)/common/libesconf-gvaluefuncs.la\
	$(GIO_LDFLAGS)				\
	$(GMODULE_LDFLAGS)

