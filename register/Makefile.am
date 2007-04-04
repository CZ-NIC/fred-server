noinst_LIBRARIES = libccreg.a

bin_PROGRAMS = fred-client

libccreg_a_SOURCES = register.cc zone.cc blacklist.cc domain.cc registrar.cc \
		     object_impl.cc contact.cc nsset.cc auth_info.cc invoice.cc \
		     documents.cc bank.cc

libccreg_a_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CFLAGS) $(AM_CPPFLAGS)

COMMON_LIBS = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS)

fred-client_SOURCES   = client.cc
fred-client_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
fred-client_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
fred-client_LDADD     = libccreg.a $(COMMON_LIBS)
