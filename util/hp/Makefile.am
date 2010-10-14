AM_CPPFLAGS = ${BOOST_CPPFLAGS} $(AM_CFLAGS) -I$(top_srcdir)/util 

noinst_LIBRARIES = libhp.a
libhp_a_SOURCES = hp.cc hp.h hpmail.cc hpmail.h hpmailbatchstate.cc hpmailbatchstate.h

noinst_PROGRAMS = client-hp crc32
client_hp_SOURCES  = client-hp.cc handle_hpmail_args.h handle_hpmailbatchstate_args.h \
                     handle_clienthp_args.h
client_hp_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB)
client_hp_LDADD  =  libhp.a

crc32_SOURCES  = crc32.cc
crc32_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB)


bin_PROGRAMS = urlencode
urlencode_SOURCES  = urlencode.cc
urlencode_LDFLAGS  = $(BOOST_LDFLAGS) $(AM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB)
urlencode_LDADD  =  libhp.a
