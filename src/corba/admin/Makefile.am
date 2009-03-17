noinst_LIBRARIES = libadmin_impl.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(AM_CFLAGS) -I$(top_srcdir)/util \
              -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)

libadmin_impl_a_SOURCES = admin_impl.cc                 \
                          admin_impl.h                  \
                          session_impl.cc               \
                          session_impl.h                \
                          user_impl.cc                  \
                          user_impl.h                   \
                          filter_impl.cc                \
                          filter_impl.h                 \
                          pagetable_impl.cc             \
                          pagetable_impl.h              \
                          pagetable_publicrequests.cc   \
                          pagetable_publicrequests.h    \
                          pagetable_contacts.cc         \
                          pagetable_contacts.h          \
                          pagetable_domains.cc          \
                          pagetable_domains.h           \
                          pagetable_eppactions.cc       \
                          pagetable_eppactions.h        \
                          pagetable_filters.cc          \
                          pagetable_filters.h           \
                          pagetable_invoices.cc         \
                          pagetable_invoices.h          \
                          pagetable_mails.cc            \
                          pagetable_mails.h             \
                          pagetable_nssets.cc           \
                          pagetable_nssets.h            \
                          pagetable_registrars.cc       \
                          pagetable_registrars.h        \
                          pagetable_files.cc            \
                          pagetable_files.h             \
                          pagetable_keysets.cc          \
                          pagetable_keysets.h           \
			  pagetable_logger.cc		\
			  pagetable_logger.h		\
                          str_corbaout.h                \
                          common.cc                     \
                          common.h 

