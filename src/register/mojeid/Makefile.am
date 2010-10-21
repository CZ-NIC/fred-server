noinst_LIBRARIES = libmojeid.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
              -I$(top_builddir)/src

libmojeid_a_SOURCES = contact.h \
                      contact.cc \
                      nullable.h \
                      request.h \
                      mojeid_data_validation.h \
                      mojeid_data_validation.cc

