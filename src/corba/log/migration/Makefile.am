	
AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(XML2_LIBS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) $(XML2_CFLAGS) -I$(top_srcdir)/util \
              -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)	
	
	
	
LDADD = $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB) \
	$(top_builddir)/util/log/liblog.a \
	$(top_builddir)/util/db/libdb.a   \
	$(top_builddir)/util/types/libtypes.a   
	 	
	
sbin_PROGRAMS = migrate_log

migrate_log_SOURCES = migrate.cc log_if.cc m_epp_parser.cc migrate.h m_epp_parser.h types.h
migrate_log_CPPFLAGS = $(AM_CPPFLAGS)
migrate_log_LDFLAGS  = $(AM_LDFLAGS)