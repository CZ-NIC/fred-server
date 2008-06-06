noinst_LIBRARIES = liboldutils.a

liboldutils_a_SOURCES = conf.cc dbsql.cc log.cc pqsql.cc timeclock.cc util.cc conf.h dbsql.h log.h pqsql.h timeclock.h util.h
AM_CPPFLAGS = $(AM_CFLAGS) $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} -I$(top_srcdir)/src 
