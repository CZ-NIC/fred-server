noinst_LIBRARIES = librequests.a

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

librequests_a_SOURCES =  model_request_property_name.h request.h \
	model_request.cc model_request_property_value.cc request_impl.h \
	model_request_data.cc model_request_property_value.h session.cc \
	model_request_data.h model_session.cc session.h model_request.h \
	model_session.h model_request_property_name.cc request.cc \
	request_cache.h request_cache.cc
