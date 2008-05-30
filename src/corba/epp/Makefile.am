noinst_LIBRARIES = libepp_impl.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) -I$(top_srcdir)/util \
              -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)

libepp_impl_a_SOURCES = epp_impl.cc messages.cc notifier.cc tech_check.cc countrycode.cc \
                        epp_impl.h messages.h notifier.h tech_check.h countrycode.h \
			action.h reason.h response.h
