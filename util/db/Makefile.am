noinst_LIBRARIES = libdb.a

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} $(AM_CFLAGS)     \
              -I$(top_srcdir)/util -I$(top_srcdir) 

libdb_a_SOURCES = query.cc                                            \
                  filter.cc compound_filter.cc simple_filter.cc       \
                  base_filters.h                                      \
                  connection.h result.h row.h value.h                 \
                  psql_connection.h psql_result.h                     \
                  query.h sequence.h manager.h                        \
                  filter.h compound_filter.h simple_filter.h          \
                  simple_filters.h sql_helper_objects.h filter_it.h   \
                  sql_operators.h db_exceptions.h database.h

noinst_PROGRAMS = test-db
test_db_SOURCES  = test-db.cc
test_db_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_db_LDADD    =  $(top_builddir)/util/types/libtypes.a libdb.a $(top_builddir)/util/log/liblog.a $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB)

