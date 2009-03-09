noinst_LIBRARIES = libtypes.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} $(AM_CFLAGS) \
              -I$(top_srcdir)/util -I$(top_srcdir)

libtypes_a_SOURCES = date.cc                       \
                     date.h                        \
                     date_interval.cc              \
                     date_interval.h               \
                     datetime.cc                   \
                     datetime.h                    \
                     datetime_interval.cc          \
                     datetime_interval.h           \
                     datetime_common.h             \
                     common.h                      \
                     money.cc                      \
                     money.h                       \
                     id.cc                         \
                     id.h                          \
                     null.h                        \
                     data_types.h                  \
                     convert_common.h              \
                     convert_str_base.h            \
                     convert_str_pod.h             \
                     convert_str_boost_datetime.h  \
                     convert_sql_base.h            \
                     convert_sql_pod.h             \
                     convert_sql_boost_datetime.h  \
                     convert_sql_db_types.h        \
                     stringify.h                   \
										 sqlize.h

