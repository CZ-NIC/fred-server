noinst_LIBRARIES = liboldutils.a

AM_CPPFLAGS = $(AM_CFLAGS) $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS}     \
							-I$(top_srcdir)/src -I$(top_srcdir)/util -I$(top_srcdir) 

liboldutils_a_SOURCES = dbsql.cc log.cc pqsql.cc timeclock.cc \
												util.cc dbsql.h log.h pqsql.h timeclock.h util.h

