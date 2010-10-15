
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
              -I$(top_builddir)/src

sbin_PROGRAMS = fred-mifd


fred_mifd_SOURCES = ../MojeIDSK.cc \
                    ../MojeIDDynSK.cc \
                    ../MailerSK.cc \
                    ../MailerDynSK.cc \
                    ../nameservice.cc \
                    ../mailer_manager.cc \
                    mojeid_impl.h \
                    mojeid_impl.cc \
                    mojeid_identification.h \
                    corba_conversion.h \
                    fred-mifd.cc


fred_mifd_LDFLAGS =  $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB) \
                     $(BOOST_SERIALIZATION_LIB)


fred_mifd_LDADD =    $(LIBIDN_LIBS) \
                     $(top_builddir)/src/register/libfred.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/libutils.a \
                     $(top_builddir)/src/register/mojeid/libmojeid.a \
                     $(top_builddir)/src/register/messages/libmessages.a


