noinst_LIBRARIES = libtypes.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) ${BOOST_CPPFLAGS} $(AM_CFLAGS) \
              -I$(top_srcdir)/util -I$(top_srcdir)

libtypes_a_SOURCES = conversions.cc                     \
										 date.cc date_interval.cc           \
										 datetime.cc datetime_interval.cc   \
                     id.cc money.cc                     \
										 conversions.h                      \
                     date.h date_interval.h             \
                     datetime.h datetime_interval.h     \
                     id.h money.h                       \
										 datetime_common.h common.h         \
										 null.h                             \
										 data_types.h
