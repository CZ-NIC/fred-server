
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


noinst_PROGRAMS = test_logd test-model test-registrar-certification-group \
					test-file-manager-client
					
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



test_registrar_certification_group_SOURCES = \
                     test-registrar-certification-group.cc \
                     test-registrar-certification-group.h \
                     $(top_srcdir)/src/corba/admin/common.cc \
                     $(top_srcdir)/src/corba/admin/common.h

#nodist_test_registrar_certification_group_SOURCES = \
#                     $(top_builddir)/src/corba/ccRegSK.cc

test_registrar_certification_group_LDFLAGS = \
                     $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(XML2_LIBS)

test_registrar_certification_group_LDADD = \
                     $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/conf/libconf.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(top_builddir)/src/corba/libcorba.a

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
                     

EXTRA_DIST = test-file.pdf
