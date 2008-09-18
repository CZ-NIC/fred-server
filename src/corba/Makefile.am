SUBDIRS = admin epp
CLEANFILES=ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) -I$(top_srcdir)/util -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)

LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) \
        $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS) \
	$(LIBIDN_CFLAGS) $(LIBIDN_LIBS) \
	admin/libadmin_impl.a epp/libepp_impl.a \
	libcorba.a \
        $(top_builddir)/src/register/libfred.a \
	$(top_builddir)/src/model/libmodel.a \
	$(top_builddir)/util/db/libdb.a \
	$(top_builddir)/util/types/libtypes.a \
	$(top_builddir)/src/old_utils/liboldutils.a \
	$(top_builddir)/util/log/liblog.a \
	$(top_builddir)/util/conf/libconf.a \
	$(top_builddir)/util/libutil.a \
	$(BOOST_THREAD_LIB) $(BOOST_SERIALIZATION_LIB)
	
	

IDLFILE = $(IDLDIR)/ccReg.idl

sbin_PROGRAMS = fred-rifd fred-adifd fred-pifd 

noinst_LIBRARIES   = libcorba.a
BUILT_SOURCES      = ccRegSK.cc ccRegDynSK.cc ccReg.hh
libcorba_a_SOURCES = nameservice.cc mailer_manager.cc nameservice.h mailer_manager.h 
#		     epp/epp_impl.cc epp/messages.cc epp/notifier.cc epp/tech_check.cc epp/countrycode.cc \
#		     epp/epp_impl.h epp/messages.h epp/notifier.h epp/tech_check.h epp/countrycode.h epp/action.h epp/reason.h epp/response.h 
nodist_libcorba_a_SOURCES = ccRegSK.cc ccRegDynSK.cc

fred_rifd_SOURCES   = server.cc
fred_rifd_CPPFLAGS  = $(AM_CPPFLAGS) -DRIFD

fred_adifd_SOURCES  = server.cc
fred_adifd_CPPFLAGS = $(AM_CPPFLAGS) -DADIF

fred_pifd_SOURCES   = server.cc
fred_pifd_CPPFLAGS  = $(AM_CPPFLAGS) -DPIFD

ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)


