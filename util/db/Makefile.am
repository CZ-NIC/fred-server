noinst_LIBRARIES = libdb.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} $(AM_CFLAGS) \
              -I$(top_srcdir)/util -I$(top_srcdir)
libdb_a_SOURCES = connection.cc psql_connection.cc psql_result.cc psql_result_it.cc     \
                  query.cc result.cc transaction.cc db_value.cc data_types.cc filter.cc \
                  compound_filter.cc simple_filter.cc                                   \
                  compound_filter.h dbexceptions.h filter.h psql_connection.h query.h   \
                  simple_filter.h connection.h dbs.h filter_it.h psql_result.h result.h \
                  simple_filters.h sql_operators.h data_types.h db_value.h manager.h    \
                  psql_result_it.h result_it.h sql_helper_objects.h transaction.h       \
                  base_filters.h row.h sequence.h
