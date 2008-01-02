SUBDIRS=register
CLEANFILES=ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh
AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS)
LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) \
        $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS) libfred.a register/libccreg.a

IDLFILE = $(IDLDIR)/ccReg.idl

fredconfdir = $(sysconfdir)/fred

noinst_LIBRARIES = libfred.a
sbin_PROGRAMS = fred-rifd fred-adifd fred-pifd fred-banking fred-admin
sbin_SCRIPTS = banking.sh
EXTRA_DIST = banking.sh
dist_fredconf_DATA = server.conf.example

BUILT_SOURCES = ccRegSK.cc ccRegDynSK.cc ccReg.hh
libfred_a_SOURCES = conf.cc dbsql.cc pqsql.cc util.cc log.cc nameservice.cc \
  mailer_manager.cc admin.cc tech_check.cc action.h gpc.h nameservice.h \
  response.h admin.h countrycode.h log.h notifier.h tech_check.h ccReg_epp.h \
  csv.h mailer_manager.h pqsql.h timeclock.h conf.h dbsql.h messages.h \
  reason.h util.h
nodist_libfred_a_SOURCES = ccRegSK.cc ccRegDynSK.cc

fred_rifd_SOURCES   = ccReg_epp.cc countrycode.cc messages.cc notifier.cc \
                      ccReg_server.cc
fred_rifd_CPPFLAGS  = $(AM_CPPFLAGS) -DRIFD

fred_adifd_SOURCES   = ccReg_server.cc
fred_adifd_CPPFLAGS  = $(AM_CPPFLAGS) -DADIF

fred_pifd_SOURCES   = ccReg_server.cc
fred_pifd_CPPFLAGS  = $(AM_CPPFLAGS) -DPIFD

fred_banking_SOURCES = gpc.cc banking.cc log.cc csv.cc

fred_admin_SOURCES   = register/client.cc

ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)
