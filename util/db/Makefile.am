noinst_LIBRARIES = libdb.a

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} $(AM_CFLAGS)        \
              -I$(top_srcdir)/util -I$(top_srcdir) 

libdb_a_SOURCES = query/query_old.cc             \
                  query/filter.cc                \
                  query/compound_filter.cc       \
                  query/simple_filter.cc         \
                  query/base_filters.h           \
                  query/query.h                  \
                  query/query_common.h           \
                  query/query_old.h              \
                  query/filter.h                 \
                  query/compound_filter.h        \
                  query/simple_filter.h          \
                  query/simple_filters.h         \
                  query/sql_helper_objects.h     \
                  query/filter_it.h              \
                  query/sql_operators.h          \
                  psql/psql_connection.h         \
                  psql/psql_result.h             \
				  model/field.h                  \
				  model/job_queue.h              \
				  model/mem_ptr.h                \
				  model/model_exception.h        \
				  model/model_field_attrs.h      \
				  model/model_field.h            \
				  model/model_field_list.h       \
				  model/model_field_macros.h     \
				  model/model.h                  \
                  connection_factory.h           \
                  transaction.h                  \
                  connection.h                   \
                  result.h                       \
                  row.h                          \
                  value.h                        \
                  statement.h                    \
                  sequence.h                     \
                  manager.h                      \
                  manager_tss.h                  \
                  db_exceptions.h                \
                  database.h                     \
                  manager_old_db.h

noinst_PROGRAMS = test-db
test_db_SOURCES  = test-db.cc
test_db_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_db_LDADD    = $(top_builddir)/util/types/libtypes.a libdb.a $(top_builddir)/util/log/liblog.a $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB)

