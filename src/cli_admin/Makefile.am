AM_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(OMNIORB4_CFLAGS) \
			  -I$(top_srcdir)/util -I$(top_srcdir)/src -I$(top_builddir)/src \
			  -I$(top_srcdir)
LDADD = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_DATE_TIME_LIB) \
        $(BOOST_REGEX_LIB) $(OMNIORB4_LIBS) \
	$(LIBIDN_CFLAGS) $(LIBIDN_LIBS) \
        $(top_builddir)/src/register/libfred.a \
	$(top_builddir)/src/model/libmodel.a \
	$(top_builddir)/util/db/libdb.a \
	$(top_builddir)/util/types/libtypes.a \
	$(top_builddir)/src/corba/libcorba.a \
	$(top_builddir)/src/old_utils/liboldutils.a \
	$(top_builddir)/util/log/liblog.a \
	$(top_builddir)/util/conf/libconf.a \
	$(top_builddir)/util/libutil.a \
	$(BOOST_THREAD_LIB) $(BOOST_SERIALIZATION_LIB)
CLEANFILES=banking.sh

sbin_PROGRAMS = fred-banking fred-admin
dist_sbin_SCRIPTS = banking.sh

fred_banking_SOURCES = gpc.cc banking.cc csv.cc gpc.h csv.h

fred_admin_SOURCES = simple.cc simple.h commonclient.cc commonclient.h \
 keysetclient.cc keysetclient.h nssetclient.cc nssetclient.h domainclient.cc \
 domainclient.h contactclient.cc contactclient.h authinfoclient.cc \
 authinfoclient.h bankclient.cc bankclient.h invoiceclient.cc invoiceclient.h \
 pollclient.cc pollclient.h registrarclient.cc registrarclient.h \
 notifyclient.cc notifyclient.h objectclient.cc objectclient.h \
 infobuffclient.cc infobuffclient.h fileclient.cc fileclient.h mailclient.cc \
 mailclient.h publicreqclient.cc publicreqclient.h
