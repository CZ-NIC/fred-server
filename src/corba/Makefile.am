SUBDIRS = admin epp log
CLEANFILES=ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh

AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) 		\
			  $(BOOST_CPPFLAGS) 		\
			  $(OMNIORB4_CFLAGS) 		\
			  $(XML2_CFLAGS)			\
			  -I$(top_srcdir)/util 		\
			  -I$(top_srcdir)/src 		\
			  -I$(top_builddir)/src 	\
			  -I$(top_srcdir)			\
			  -I$(top_srcdir)/util/db/model

LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) \
        $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS) \
	$(LIBIDN_CFLAGS) $(LIBIDN_LIBS) \
	epp/libepp_impl.a \
	log/liblog_impl.a \
	admin/libadmin_impl.a \
	libcorba.a \
	$(top_builddir)/src/register/libfred.a \
	$(top_builddir)/src/model/libmodel.a \
	$(top_builddir)/util/db/libdb.a \
	$(top_builddir)/util/types/libtypes.a \
	$(top_builddir)/src/old_utils/liboldutils.a \
	$(top_builddir)/util/log/liblog.a \
	$(top_builddir)/util/conf/libconf.a \
	$(top_builddir)/util/libutil.a \
	$(BOOST_THREAD_LIB) $(BOOST_SERIALIZATION_LIB) $(XML2_LIBS)
	
	

IDLFILE = $(IDLDIR)/ccReg.idl

sbin_PROGRAMS = fred-rifd fred-adifd fred-pifd fred-logd

noinst_LIBRARIES   = libcorba.a

noinst_PROGRAMS = test

test_SOURCES = test-admin.cc
test_LDADD = $(XML2_LIBS) $(LDADD)

BUILT_SOURCES      = ccRegSK.cc ccRegDynSK.cc ccReg.hh
libcorba_a_SOURCES = nameservice.cc         \
					 mailer_manager.cc      \
					 nameservice.h          \
					 mailer_manager.h       \
					 connection_releaser.h  \
					 pidfile.h              \
					 daemonize.h            \
					 file_manager_client.cc \
					 file_manager_client.h
#		     epp/epp_impl.cc epp/messages.cc epp/notifier.cc epp/tech_check.cc epp/countrycode.cc \
#		     epp/epp_impl.h epp/messages.h epp/notifier.h epp/tech_check.h epp/countrycode.h epp/action.h epp/reason.h epp/response.h 
nodist_libcorba_a_SOURCES = ccRegSK.cc ccRegDynSK.cc

fred_rifd_SOURCES   = server.cc
fred_rifd_CPPFLAGS  = $(AM_CPPFLAGS) -DRIFD

fred_adifd_SOURCES  = server.cc
fred_adifd_CPPFLAGS = $(AM_CPPFLAGS) -DADIF

fred_pifd_SOURCES   = server.cc
fred_pifd_CPPFLAGS  = $(AM_CPPFLAGS) -DPIFD

fred_logd_SOURCES   = server.cc
fred_logd_CPPFLAGS  = $(AM_CPPFLAGS) -DLOGD

ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)


