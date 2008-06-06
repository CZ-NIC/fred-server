AM_CPPFLAGS = ${BOOST_CPPFLAGS} $(AM_CFLAGS) -I$(top_srcdir)/util

noinst_LIBRARIES = liblog.a
liblog_a_SOURCES = logger.cc log.cc log_types.cc log.h log_types.h logger.h 
liblog_a_ADD 	 = $(BOOST_THREAD_LIB)

noinst_PROGRAMS = test-log
test_log_SOURCES  = test-log.cc
test_log_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_log_LDADD    = liblog.a $(BOOST_DATE_TIME_LIB) $(BOOST_THREAD_LIB)
 
