noinst_LIBRARIES = libepp_impl.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) -I$(top_srcdir)/util \
              -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)

libepp_impl_a_SOURCES = epp_impl.cc           \
                        epp_impl.h            \
                        messages.cc           \
                        messages.h            \
                        notifier.cc           \
                        notifier.h            \
                        tech_check.cc         \
                        tech_check.h          \
                        countrycode.cc        \
                        countrycode.h         \
                        action.h              \
                        reason.h              \
                        response.h            \
                        notifier_changes.cc   \
                        notifier_changes.h 
