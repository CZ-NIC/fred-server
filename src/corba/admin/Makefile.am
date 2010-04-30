noinst_LIBRARIES = libadmin_impl.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) 	\
			  $(BOOST_CPPFLAGS) 	\
			  $(AM_CFLAGS) 			\
			  $(XML2_CFLAGS) 		\
			  -I$(top_srcdir)/util 	\
              -I$(top_srcdir)/src 	\
			  -I$(top_builddir)/src \
			  -I$(top_srcdir)		\
			  -I$(top_srcdir)/util/db/model

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
			  			  pagetable_logger.cc			\
			  			  pagetable_logger.h			\
						  pagetable_logsession.cc		\
						  pagetable_logsession.h		\
						  pagetable_bank_payments.cc    \
						  pagetable_bank_payments.h     \
						  pagetable_bank_statements.cc  \
						  pagetable_bank_statements.h   \
                          pagetable_zones.cc            \
                          pagetable_zones.h             \
                          str_corbaout.h                \
						  usertype_conv.h				\
                          common.cc                     \
                          common.h 						\
                          zone_impl.cc			        \
                          zone_impl.h			        \
						  bankinginvoicing_impl.cc		\
						  bankinginvoicing_impl.h

noinst_PROGRAMS =  test-registrar-certification-group                     
test_registrar_certification_group_SOURCES = test-registrar-certification-group.cc \
                     test-registrar-certification-group.h
test_registrar_certification_group_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) $(OMNIORB4_LIBS) $(BOOST_DATE_TIME_LIB) \
                         $(BOOST_REGEX_LIB) \
                         $(BOOST_THREAD_LIB) \
                         $(XML2_LIBS)
test_registrar_certification_group_LDADD   = \ 
                         $(top_builddir)/src/register/libfred.a \
                         $(top_builddir)/src/model/libmodel.a \
                         $(top_builddir)/util/db/libdb.a \
                         $(top_builddir)/util/log/liblog.a \
                         $(top_builddir)/util/conf/libconf.a \
                         $(top_builddir)/util/types/libtypes.a \
                         $(top_builddir)/src/old_utils/liboldutils.a
