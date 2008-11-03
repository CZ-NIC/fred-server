noinst_LIBRARIES = libmodel.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) \
              -I$(top_srcdir)/util -I$(top_srcdir) -I$(top_srcdir)/src

libmodel_a_SOURCES = object_registry_filter.cc object_filter.cc domain_filter.cc \
                     contact_filter.cc nsset_filter.cc registrar_filter.cc \
                     epp_action_filter.cc epp_session_filter.cc filter_filter.cc \
                     object_state_filter.cc invoice_filter.cc public_request_filter.cc \
                     mail_filter.cc file_filter.cc keyset_filter.cc \
          					 union_filter.cc \
                     contact_filter.h epp_action_filter.h nsset_filter.h \
                     object_registry_filter.h registrar_filter.h domain_filter.h \
                     epp_session_filter.h model_filters.h object_filter.h \
                     object_state_filter.h filter_filter.h invoice_filter.h \
                     public_request_filter.h mail_filter.h file_filter.h \
                     keyset_filter.h union_filter.h

noinst_PROGRAMS = test-model
test_model_SOURCES  = test-model.cc
test_model_LDFLAGS  = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_model_LDADD    = libmodel.a \
                      $(top_builddir)/util/types/libtypes.a \
                      $(top_builddir)/util/db/libdb.a \
                      $(top_builddir)/util/libutil.a \
                      $(top_builddir)/util/log/liblog.a \
                      $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB) $(BOOST_SERIALIZATION_LIB)
		      
