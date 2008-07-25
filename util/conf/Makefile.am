AM_CPPFLAGS = ${BOOST_CPPFLAGS} $(AM_CFLAGS) -I$(top_srcdir)/util 

noinst_LIBRARIES = libconf.a
libconf_a_SOURCES = manager.cc conf.cc manager.h conf.h
libconf_a_ADD = $(BOOST_PROGRAM_OPTIONS_LIB)

noinst_PROGRAMS = test-conf
test_conf_SOURCES  = test-conf.cc
test_conf_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_conf_LDADD    = libconf.a $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_THREAD_LIB) 
