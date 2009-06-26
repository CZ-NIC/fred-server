noinst_LIBRARIES = liblog_impl.a 

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) \
-I. -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/util -I$(top_builddir)/src -I$(top_srcdir)/util/db/model

LDADD = $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB) \
	$(top_builddir)/util/db/libdb.a   \
	$(top_builddir)/util/types/libtypes.a   \
	$(top_builddir)/util/log/liblog.a 

#LDADD = liblog_impl.a 	\
#	$(prefix)/util/db/libdb.a   \
#	$(prefix)/util/types/libtypes.a   


liblog_impl_a_SOURCES = log_impl_wrap.h log_impl_wrap.cc log_impl.h log_impl.cc model_request_type.cc model_request.cc model_request_property.cc model_request_property_value.cc model_request_data.cc model_session.cc model_service.cc model_request_type.h model_request.h model_request_property.h model_request_property_value.h model_request_data.h model_session.h model_service.h 
liblog_impl_a_LDFLAGS = $(AM_LDFLAGS) 

sbin_PROGRAMS = test_logd

test_logd_SOURCES = test_logd.cc log_impl.h log_impl.cc model_request_type.cc model_request.cc model_request_property.cc model_request_property_value.cc model_request_data.cc model_session.cc model_service.cc model_request_type.h model_request.h model_request_property.h model_request_property_value.h model_request_data.h model_session.h model_service.h														

test_logd_CPPFLAGS = $(AM_CPPFLAGS)
# test_logd_LDFLAGS = $(AM_LDFLAGS) -lboost_unit_test_framework liblog_impl.a
test_logd_LDFLAGS = $(AM_LDFLAGS) -lboost_unit_test_framework 


