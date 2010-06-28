
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/src/corba/admin \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
			  -I$(top_builddir)/src

noinst_PROGRAMS = test_logd test-model test-db\
					test-file-manager-client test-registrar-group \
					test-registrar-certification
					
#tests
#TESTS = $(noinst_PROGRAMS)

test_logd_SOURCES =  test_logd.cc \
                     $(top_srcdir)/src/register/request.h \
                     $(top_srcdir)/src/register/model_request.h \
                     $(top_srcdir)/src/register/model_request_property.h \
                     $(top_srcdir)/src/register/model_request_property_value.h \
                     $(top_srcdir)/src/register/model_request_data.h \
                     $(top_srcdir)/src/register/model_session.h

test_logd_LDFLAGS =  $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS)

test_logd_LDADD =    $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_THREAD_LIB)

test_model_SOURCES = test-model.cc \
                     test-model.h

test_model_LDFLAGS = $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB)

test_model_LDADD =   $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

test_db_SOURCES = test-db.cc \
                     test-db.h

test_db_LDFLAGS = $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB)

test_db_LDADD =   $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

test_file_manager_client_SOURCES = \
					test-file-manager-client.cc \
					test-file-manager-client.h
					
test_file_manager_client_LDFLAGS = \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB)

test_file_manager_client_LDADD = \
                     $(top_builddir)/src/corba/libcorba.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a

test_registrar_group_SOURCES = \
                     test-registrar-group.cc \
                     test-registrar-group.h \
                     $(top_srcdir)/src/corba/admin/common.cc \
                     $(top_srcdir)/src/corba/admin/common.h

test_registrar_group_LDFLAGS = \
                     $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

test_registrar_group_LDADD = \
                     $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(top_builddir)/src/corba/libcorba.a

test_registrar_certification_SOURCES = \
                     test-registrar-certification.cc \
                     test-registrar-certification.h \
                     $(top_srcdir)/src/corba/admin/common.cc \
                     $(top_srcdir)/src/corba/admin/common.h

test_registrar_certification_LDFLAGS = \
                     $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

test_registrar_certification_LDADD = \
                     $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(top_builddir)/src/corba/libcorba.a

install-tests: $(noinst_PROGRAMS)
	$(INSTALL_PROGRAM) $(noinst_PROGRAMS) $(bindir)


EXTRA_DIST = test-file.pdf
