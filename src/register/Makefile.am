noinst_LIBRARIES = libfred.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(LIBIDN_CFLAGS) \
			  $(XML2_CFLAGS) 			\
			  -I$(top_srcdir)/src 		\
			  -I$(top_srcdir)/util 		\
			  -I$(top_srcdir) 			\
			  -I$(top_srcdir)/util/db/model

libfred_a_SOURCES = register.cc          \
					register.h           \
					common_object.h      \
					common_impl.cc       \
					common_impl.h        \
					object.h             \
					object_impl.cc       \
					object_impl.h        \
					common_impl_new.cc       \
					common_impl_new.h        \
					object_new.h             \
					model_enumfiletype.cc			\
					model_enumfiletype.h			\
					model_enumoperation.cc			\
					model_enumoperation.h			\
					model_enumbankcode.cc			\
					model_enumbankcode.h			\
					model_enum_tlds.cc			\
					model_enum_tlds.h			\
					model_invoiceobjectregistry.cc	\
					model_invoiceobjectregistry.h	\
					model_invoiceobjectregistrypricemap.cc	\
					model_invoiceobjectregistrypricemap.h	\
					model_invoicecreditpaymentmap.cc	\
					model_invoicecreditpaymentmap.h		\
					model_invoicegeneration.cc			\
					model_invoicegeneration.h			\
					model_files.cc				\
					model_files.h				\
					model_invoiceprefix.cc		\
					model_invoiceprefix.h		\
					model_invoice.cc			\
					model_invoice.h				\
					model_bankaccount.cc		\
					model_bankaccount.h			\
					model_bankstatementhead.cc	\
					model_bankstatementhead.h	\
					model_bankstatementitem.cc	\
					model_bankstatementitem.h	\
					model_enumbankcode.cc		\
					model_enumbankcode.h		\
					model_object.cc				\
					model_object.h				\
					model_object_registry.cc	\
					model_object_registry.h		\
					model_registrar_acl.cc		\
					model_registrar_acl.h		\
					model_registrarinvoice.cc		\
					model_registrarinvoice.h		\
					model_registrar.cc			\
					model_registrar.h			\
					model_zone.cc				\
					model_zone.h				\
					model_zone_ns.cc			\
					model_zone_ns.h				\
					model_zone_soa.cc			\
					model_zone_soa.h			\
					model_mailtype.cc			\
					model_mailtype.h			\
					model_mailarchive.cc		\
					model_mailarchive.h			\
					model_mailattachments.cc	\
					model_mailattachments.h		\
					model_mailhandles.cc		\
					model_mailhandles.h			\
					domain.cc            \
					domain.h             \
					contact.cc           \
					contact.h            \
					nsset.cc             \
					nsset.h              \
					keyset.cc            \
					keyset.h             \
					registrar.cc         \
					registrar.h          \
					public_request.cc    \
					public_request.h     \
					invoice_common.cc	\
					invoice_common.h	\
					invoice.cc           \
					invoice.h            \
					invoice_list.cc		\
					invoice_list.h		\
					invoice_manager.cc	\
					invoice_manager.h	\
					invoice_mails.cc	\
					invoice_mails.h		\
					invoice_exporter.cc	\
					invoice_exporter.h	\
					zone.cc              \
					zone.h               \
					blacklist.cc         \
					blacklist.h          \
					documents.cc         \
					documents.h          \
					info_buffer.cc       \
					info_buffer.h        \
					bank_common.cc		\
					bank_common.h		\
					bank_item.cc  \
					bank_item.h	\
					bank_head.cc	\
					bank_head.h	\
					bank_list.cc			\
					bank_list.h				\
					bank_item_list.cc		\
					bank_item_list.h		\
					bank_manager.cc			\
					bank_manager.h			\
					bank_exporter.cc		\
					bank_exporter.h			\
					mail.cc              \
					mail.h               \
					notify.h             \
					notify.cc            \
					poll.cc              \
					poll.h               \
					file.cc              \
					file.h               \
					filter.cc            \
					filter.h             \
		    		request.cc	 		 \
		    		request.h		 	 \
					session.cc		 \
					session.h		 \
					mailer.h             \
					sql.h                \
					exceptions.h         \
					types.h

noinst_PROGRAMS = test
test_SOURCES = test.cc
test_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
test_LDADD   = libfred.a \
                        $(top_builddir)/src/model/libmodel.a \
                        $(top_builddir)/util/libutil.a \
                        $(top_builddir)/util/db/libdb.a \
                        $(top_builddir)/util/log/liblog.a \
                        $(top_builddir)/util/conf/libconf.a \
                        $(top_builddir)/util/types/libtypes.a \
                        $(top_builddir)/src/old_utils/liboldutils.a \
                        $(BOOST_DATE_TIME_LIB) \
                        $(BOOST_REGEX_LIB) \
                        $(BOOST_THREAD_LIB) \
                        $(XML2_LIBS)

#noinst_PROGRAMS = testregzon
#testregzon_SOURCES = test_regzon.cc
#testregzon_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS) -lboost_unit_test_framework
#testregzon_LDADD   = libfred.a \
#                        $(top_builddir)/src/model/libmodel.a \
#                        $(top_builddir)/util/libutil.a \
#                        $(top_builddir)/util/db/libdb.a \
#                        $(top_builddir)/util/log/liblog.a \
#                        $(top_builddir)/util/conf/libconf.a \
#                        $(top_builddir)/util/types/libtypes.a \
#                        $(top_builddir)/src/old_utils/liboldutils.a \
#                        $(BOOST_DATE_TIME_LIB) \
#                        $(BOOST_REGEX_LIB) \
#                        $(BOOST_THREAD_LIB) \
#                        $(XML2_LIBS)


# noinst_PROGRAMS = test-register
# test_register_SOURCES = test.cc
# test_register_LDFLAGS = $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS) $(AM_LDFLAGS)
# test_register_LDADD   = libfred.a \
#                         $(top_builddir)/src/model/libmodel.a \
#                         $(top_builddir)/util/libutil.a \
#                         $(top_builddir)/util/db/libdb.a \
#                         $(top_builddir)/util/log/liblog.a \
#                         $(top_builddir)/util/conf/libconf.a \
#                         $(top_builddir)/util/types/libtypes.a \
#                         $(top_builddir)/src/old_utils/liboldutils.a \
#                         $(BOOST_DATE_TIME_LIB) \
#                         $(BOOST_REGEX_LIB) \
#                         $(BOOST_THREAD_LIB) \
#                         $(XML2_LIBS)

