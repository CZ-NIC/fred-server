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



liblog_impl_a_SOURCES = log_impl.h log_impl.cc log_impl_wrap.h log_impl_wrap.cc manage_part_table.h manage_part_table.cc model_log_action_type.cc model_log_entry.cc model_log_property_name.cc model_log_property_value.cc model_log_raw_content.cc model_log_session.cc

sbin_PROGRAMS = test_logd test_manage_part

test_logd_SOURCES = test_logd.cc log_impl.cc log_impl.h manage_part_table.h manage_part_table.cc model_log_action_type.cc model_log_entry.cc model_log_property_name.cc model_log_property_value.cc model_log_raw_content.cc model_log_session.cc

test_logd_CPPFLAGS = $(AM_CPPFLAGS)
test_logd_LDFLAGS = $(AM_LDFLAGS) -lboost_unit_test_framework

test_manage_part_SOURCES = log_impl.h manage_part_table.h manage_part_table.cc test_manage_part.cc 
test_manage_part_CPPFLAGS = $(AM_CPPFLAGS)
 


