AM_CPPFLAGS = ${BOOST_CPPFLAGS} $(AM_CFLAGS) -I$(top_srcdir)/util 

noinst_LIBRARIES = libhp.a
libhp_a_SOURCES = hp.cc hp.h

noinst_PROGRAMS = test-hp
test_hp_SOURCES  = test-hp.cc
test_hp_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS) 
test_hp_LDADD  =  libhp.a
