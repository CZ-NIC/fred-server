AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(XML2_LIBS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) $(XML2_CFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/util \
              -I$(top_srcdir)/src/corba/log -I$(top_builddir)/src -I$(top_srcdir) -I$(top_srcdir)/util/db/model

sbin_PROGRAMS = migrate_log

migrate_log_SOURCES = migrate.cc log_if.cc m_epp_parser.cc migrate.h m_epp_parser.h
migrate_log_CPPFLAGS = $(AM_CPPFLAGS)
migrate_log_LDFLAGS  = $(AM_LDFLAGS)
migrate_log_LDADD    = $(top_builddir)/src/register/libfred.a	    \
		       $(top_builddir)/src/model/libmodel.a 	    \
		       $(top_builddir)/src/corba/log/liblog_impl.a  \
                       $(top_builddir)/util/types/libtypes.a        \
                       $(top_builddir)/util/db/libdb.a              \
                       $(top_builddir)/util/log/liblog.a            \
                       $(top_builddir)/src/old_utils/liboldutils.a  \
                       $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB) 


