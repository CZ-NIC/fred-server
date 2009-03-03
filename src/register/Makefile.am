noinst_LIBRARIES = libfred.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(LIBIDN_CFLAGS) \
			  $(XML2_CFLAGS) -I$(top_srcdir)/src \
			  -I$(top_srcdir)/util -I$(top_srcdir) 

libfred_a_SOURCES = register.cc          \
                    register.h           \
                    common_object.h      \
                    common_impl.cc       \
                    common_impl.h        \
                    object.h             \
                    object_impl.cc       \
                    object_impl.h        \
                    domain.cc            \
                    domain.h             \
                    contact.cc           \
                    contact.h            \
                    nsset.cc             \
                    nsset.h              \
                    keyset.cc            \
                    keyset.h             \
                    registrar.cc         \
                    registrar.h          \
                    public_request.cc    \
                    public_request.h     \
                    invoice.cc           \
                    invoice.h            \
                    zone.cc              \
                    zone.h               \
                    blacklist.cc         \
                    blacklist.h          \
                    documents.cc         \
                    documents.h          \
                    info_buffer.cc       \
                    info_buffer.h        \
                    bank.cc              \
                    bank.h               \
                    mail.cc              \
                    mail.h               \
                    notify.h             \
                    notify.cc            \
                    poll.cc              \
                    poll.h               \
                    file.cc              \
                    file.h               \
                    filter.cc            \
                    filter.h             \
                    mailer.h             \
                    sql.h                \
                    exceptions.h         \
                    types.h              \
                    db_settings.h 


# noinst_PROGRAMS = test-register
# test_register_SOURCES = test.cc
# test_register_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
# test_register_LDADD   = libfred.a \
#                         $(top_builddir)/src/model/libmodel.a \
#                         $(top_builddir)/util/libutil.a \
#                         $(top_builddir)/util/db/libdb.a \
#                         $(top_builddir)/util/log/liblog.a \
#                         $(top_builddir)/util/conf/libconf.a \
#                         $(top_builddir)/util/types/libtypes.a \
#                         $(top_builddir)/src/old_utils/liboldutils.a \
#                         $(BOOST_DATE_TIME_LIB) \
#                         $(BOOST_REGEX_LIB) \
#                         $(BOOST_THREAD_LIB) \
#                         $(XML2_LIBS)

