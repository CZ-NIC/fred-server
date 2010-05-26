noinst_LIBRARIES = libfred.a

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) $(LIBIDN_CFLAGS) \
			  $(XML2_CFLAGS) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/util \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/util/db/model

libfred_a_LIBADD = $(top_builddir)/src/old_utils/liboldutils.a


libfred_a_SOURCES = register.cc \
					register.h \
					db_settings.h \
					common_new.h \
					common_object.h \
					common_impl.cc \
					common_impl.h \
					object.h \
					object_impl.cc \
					object_impl.h \
					common_impl_new.cc \
					common_impl_new.h \
					model_files.cc \
					model_files.h \
					model_bank_statement.cc \
					model_bank_statement.h \
					model_bank_payment.cc \
					model_bank_payment.h \
					model_registrar_acl.cc \
					model_registrar_acl.h \
					model_registrar.cc \
					model_registrar.h \
					model_zone.cc \
					model_zone.h \
					model_zone_ns.cc \
					model_zone_ns.h \
					model_zone_soa.cc \
					model_zone_soa.h \
					model_price_list.cc \
					model_price_list.h \
					model_request.cc \
					model_request_data.cc \
					model_request_data.h \
					model_request.h \
					model_request_property.cc \
					model_request_property.h \
					model_request_property_value.cc \
					model_request_property_value.h \
					model_session.cc \
					model_session.h \
					model_registrar_certification.h \
					model_registrar_certification.cc \
					model_registrar_group.h \
					model_registrar_group.cc \
					model_registrar_group_map.h \
					model_registrar_group_map.cc \
					domain.cc \
					domain.h \
					contact.cc \
					contact.h \
					nsset.cc \
					nsset.h \
					keyset.cc \
					keyset.h \
					registrar.cc \
					registrar.h \
					public_request.cc \
					public_request.h \
					invoice.cc \
					invoice.h \
					invoice_list.h \
					invoice_manager.h \
					zone.cc \
					zone.h \
					blacklist.cc \
					blacklist.h \
					documents.cc \
					documents.h \
					info_buffer.cc \
					info_buffer.h \
					bank_common.cc \
					bank_common.h \
					bank_payment.h \
					bank_payment_impl.h \
					bank_payment_list.h \
					bank_payment_list_impl.h \
					bank_manager.cc \
					bank_manager.h \
					bank_exporter.cc \
					bank_exporter.h \
					bank_statement.h \
					bank_statement_impl.h \
					bank_statement_list.h \
					bank_statement_list_impl.h \
					mail.cc \
					mail.h \
					notify.h \
					notify.cc \
					poll.cc \
					poll.h \
					file_transferer.h \
					file.cc \
					file.h \
					filter.cc \
					filter.h \
					request.cc \
					request.h \
					request_impl.h \
					session.cc \
					session.h \
					mailer.h \
					sql.h \
					exceptions.h \
					types.h

