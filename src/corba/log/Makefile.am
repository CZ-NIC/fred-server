noinst_LIBRARIES = liblog_impl.a 

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) -I. -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/util -I$(top_srcdir)/src/register -I$(top_srcdir)/util/db/model -I$(top_builddir)/src

LDADD = $(prefix)/src/register/libfred.a

liblog_impl_a_SOURCES = log_impl_wrap.h log_impl_wrap.cc 

