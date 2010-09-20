
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
              -I$(top_builddir)/src

sbin_PROGRAMS = fred-mojeid


fred_mojeid_SOURCES = ../MojeIDSK.cc \
                      ../MojeIDDynSK.cc \
                      ../nameservice.cc \
					  mojeid_impl.h \
                      mojeid_impl.cc \
                      fred-mojeid.cc


fred_mojeid_LDFLAGS =  $(POSTGRESQL_LDFLAGS) \
                       $(BOOST_LDFLAGS) \
                       $(AM_LDFLAGS) \
                       $(BOOST_PROGRAM_OPTIONS_LIB) \
                       $(OMNIORB4_LIBS) \
                       $(BOOST_DATE_TIME_LIB) \
                       $(BOOST_REGEX_LIB) \
                       $(BOOST_THREAD_LIB)


fred_mojeid_LDADD =    $(top_builddir)/src/register/libfred.a \
                       $(top_builddir)/src/model/libmodel.a \
                       $(top_builddir)/util/db/libdb.a \
                       $(top_builddir)/util/types/libtypes.a \
                       $(top_builddir)/src/old_utils/liboldutils.a \
                       $(top_builddir)/util/log/liblog.a





