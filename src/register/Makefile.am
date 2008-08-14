noinst_LIBRARIES = libfred.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(LIBIDN_CFLAGS) -I$(top_srcdir)/src \
 	      -I$(top_srcdir)/util -I$(top_srcdir)

libfred_a_SOURCES = register.cc zone.cc blacklist.cc domain.cc registrar.cc \
                    object_impl.cc contact.cc nsset.cc auth_info.cc invoice.cc \
                    documents.cc bank.cc info_buffer.cc poll.cc common_impl.cc \
                    notify.cc filter.cc public_request.cc mail.cc file.cc \
                    auth_info.h common_object.h mailer.h \
          					keyset.cc \
                    object_impl.h sql.h bank.h contact.h exceptions.h \
                    notify.h poll.h types.h blacklist.h documents.h \
                    info_buffer.h nsset.h register.h zone.h common_impl.h \
                    domain.h invoice.h object.h registrar.h filter.h \
                    public_request.h mail.h file.h \
					          keyset.h

### noinst_PROGRAMS = test-register
### test_register_SOURCES = test.cc
### test_register_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
### test_register_LDADD   = libfred.a \
						                ### $(top_builddir)/src/model/libmodel.a \
            				    		### $(top_builddir)/util/db/libdb.a \
                						### $(top_builddir)/util/log/liblog.a \
                						### $(top_builddir)/src/old_utils/liboldutils.a \
                						### $(BOOST_DATE_TIME_LIB) \
                						### $(BOOST_REGEX_LIB) \
                						### $(BOOST_THREAD_LIB)
