ACLOCAL_AMFLAGS = -I m4
include $(top_srcdir)/am_doxygen.am
SUBDIRS=util src
CLEANFILES=config/server.conf init-scripts/fred-server

fredconfdir = $(sysconfdir)/fred
rcconfdir = $(sysconfdir)/init.d

EXTRA_DIST = fred-server.spec
dist_fredconf_DATA = config/server.conf
dist_rcconf_SCRIPTS = init-scripts/fred-server

if DX_COND_html
doc_DATA = @DX_DOCDIR@/html/*
$(doc_DATA): doxygen-doc
MOSTLYCLEANFILES = ${DX_CLEANFILES}
endif
