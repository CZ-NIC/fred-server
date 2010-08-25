noinst_LIBRARIES = libmessages.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/src/corba/admin \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
              -I$(top_builddir)/src

libmessages_a_SOURCES =  messages_impl.cc messages_impl.h


