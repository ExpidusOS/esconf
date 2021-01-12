## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.8

# The name of the module.
DOC_MODULE=esconf

# The top-level SGML file.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS=--deprecated-guards="ESCONF_DISABLE_DEPRECATED"

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR=../..

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS=--module=$(DOC_MODULE)

# Used for dependencies
HFILE_GLOB = \
	$(top_srcdir)/esconf/*.h \
	$(top_srcdir)/esconfd/*.h
CFILE_GLOB = \
	$(top_srcdir)/esconf/*.c \
	$(top_srcdir)/esconfd/*.c \
	$(top_srcdir)/common/esconf-errors.c

# Header files to ignore when scanning
IGNORE_HFILES= \
	config.h \
	esconf-backend-factory.h \
	esconf-backend-perchannel-xml.h \
	esconf-daemon.h \
	esconf-gdbus-bindings.h \
	esconf-gdbus-bindings.c \
	esconf-marshal.h \
	esconf-private.h \
	esconf-common-private.h

# Extra files to add when scanning (relative to $srcdir)
EXTRA_HFILES=

# Images to copy into HTML directory
HTML_IMAGES = 

# Extra SGML files that are included by DOC_MAIN_SGML_FILE
content_files = \
	version.xml

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(GLIB_CFLAGS) \
	$(LIBEXPIDUS1UTIL_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(GTK_DOC_EXTRA_CFLAGS)

GTKDOC_LIBS = \
	$(top_builddir)/esconf/libesconf-$(LIBESCONF_VERSION_API).la \
	$(top_builddir)/esconfd/esconfd-esconf-backend.o \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS)

include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST += \
	version.xml.in

# manual pages
man_MANS =

if MAINTAINER_MODE
%.1: %.xml
	xsltproc -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<
endif

# required for gtk-doc
dist-hook: all

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
