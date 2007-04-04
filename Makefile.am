SUBDIRS=register

noinst_LIBRARIES = libfred.a

bin_PROGRAMS = fred_rifd fred_adifd fred_pifd banking expiration

bin_SCRIPTS = banking.sh

libfred_a_SOURCES = ccRegSK.cc ccRegDynSK.cc conf.cc dbsql.cc pqsql.cc util.cc log.cc nameservice.cc mailer_manager.cc admin.cc tech_check.cc
libfred_a_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)

COMMON_LIBS = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS)

IDLDIR = idl
IDLFILE = $(IDLDIR)/ccReg.idl

fred_rifd_SOURCES   = ccReg_epp.cc countrycode.cc messages.cc notifier.cc

fred_rifd_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
fred_rifd_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
fred_rifd_LDADD     = ccReg_rifd.o libfred.a register/libccreg.a $(COMMON_LIBS)


fred_adifd_SOURCES   = 
fred_adifd_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
fred_adifd_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
fred_adifd_LDADD    = ccReg_adifd.o libfred.a register/libccreg.a $(COMMON_LIBS)

fred_pifd_SOURCES   = whois.cc

fred_pifd_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
fred_pifd_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
fred_pifd_LDADD     = ccReg_pifd.o libfred.a register/libccreg.a $(COMMON_LIBS)

banking_SOURCES = gpc.cc banking.cc log.cc csv.cc
banking_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
banking_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
banking_LDADD     = libfred.a register/libccreg.a $(COMMON_LIBS)

expiration_SOURCES = expiration.cc 
expiration_CPPFLAGS  = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) $(AM_CPPFLAGS)
expiration_LDFLAGS   = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
expiration_LDADD     = libfred.a register/libccreg.a $(COMMON_LIBS)

ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)

ccReg_rifd.o:  ccReg.hh
	$(CXXCOMPILE) $(POSTGRESQL_CFLAGS) -DRIFD  -o ccReg_rifd.o -g -c  ccReg_server.cc

ccReg_pifd.o: ccReg.hh
	$(CXXCOMPILE) $(POSTGRESQL_CFLAGS) -DPIFD  -o ccReg_pifd.o -g -c  ccReg_server.cc

ccReg_adifd.o:  ccReg.hh
	$(CXXCOMPILE) $(POSTGRESQL_CFLAGS) -DADIF  -o ccReg_adifd.o -g -c  ccReg_server.cc

clean::
	-@rm -f ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh
