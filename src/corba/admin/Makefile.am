noinst_LIBRARIES = libadmin_impl.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) -I$(top_srcdir)/util \
              -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)

libadmin_impl_a_SOURCES = admin_impl.cc session_impl.cc filter_impl.cc      \
                          pagetable_impl.cc user_impl.cc common.cc          \
                          pagetable_publicrequests.cc pagetable_contacts.cc \
                          pagetable_domains.cc pagetable_eppactions.cc      \
			                    pagetable_filters.cc pagetable_invoices.cc        \
			                    pagetable_mails.cc pagetable_nssets.cc            \
			                    pagetable_registrars.cc pagetable_files.cc        \
          								pagetable_keysets.cc                              \
			                    admin_impl.h session_impl.h filter_impl.h         \
			                    pagetable_impl.h user_impl.h common.h             \
			                    pagetable_publicrequests.h pagetable_contacts.h   \
			                    pagetable_domains.h pagetable_eppactions.h        \
			                    pagetable_filters.h pagetable_invoices.h          \
			                    pagetable_mails.h pagetable_nssets.h              \
			                    pagetable_registrars.h pagetable_files.h          \
					          			pagetable_keysets.h
