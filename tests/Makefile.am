
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/fredlib \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/src/corba/admin \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
			  -I$(top_builddir)/src

noinst_PROGRAMS = test-server 
#tests
#TESTS = $(noinst_PROGRAMS)


test_server_SOURCES = test-server.cc \
                     test-server.h \
                     test-conv.cc \
                     test-conv.h \
					 test-file-manager-client.cc \
					 test-file-manager-client.h \
					 test-db.cc \
                     test-db.h \
                     test-model.cc \
                     test-model.h \
                     test-registrar-group.cc \
                     test-registrar-group.h \
                     $(top_srcdir)/src/corba/admin/filter_impl.cc \
                     $(top_srcdir)/src/corba/admin/filter_impl.h \
                     test-registrar-certification.cc \
                     test-registrar-certification.h \
                     test_logd.cc test-msgd.cc test-msgd.h

test_server_LDFLAGS = $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS)

test_server_LDADD =  $(top_builddir)/src/corba/admin/libadmin_impl.a \
					 $(top_builddir)/src/corba/libcorba.a \
					 $(top_builddir)/src/fredlib/requests/librequests.a \
					 $(top_builddir)/src/fredlib/libfred.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(top_builddir)/src/corba/libcorbamessages.a \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

install-tests: $(noinst_PROGRAMS)
	$(INSTALL_PROGRAM) $(noinst_PROGRAMS) $(bindir)


EXTRA_DIST = test-file.pdf
