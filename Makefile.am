ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
		$(POSTGRESQL_CFLAGS) \
		${BOOST_CPPFLAGS} \
		$(AM_CFLAGS) \
		$(LIBIDN_CFLAGS) \
		$(OMNIORB4_CFLAGS) \
		$(XML2_CFLAGS) \
		-I $(top_srcdir)/util \
		-I $(top_srcdir) \
		-I $(top_srcdir)/util/db/model \
		-I $(top_srcdir)/util/db \
		-I $(top_srcdir)/util/decimal \
		-I $(top_srcdir)/include \
		-I $(top_builddir)

AM_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS)

include $(top_srcdir)/am_doxygen.am


CLEANFILES = \
		config/server.conf \
		config/hpmail.conf \
		config/optysmail.conf \
		init-scripts/fred-server \
		doxy.conf \
		src/corba/MessagesSK.cc \
		src/corba/MessagesDynSK.cc \
		src/corba/Messages.hh \
		src/corba/MojeIDSK.cc \
		src/corba/MojeIDDynSK.cc \
		src/corba/MojeID.hh \
		src/corba/AdministrativeBlockingSK.cc \
		src/corba/AdministrativeBlockingDynSK.cc \
		src/corba/AdministrativeBlockingSK.h \
		src/corba/AdministrativeBlocking.hh \
		src/corba/FileManagerSK.cc \
		src/corba/FileManagerDynSK.cc \
		src/corba/FileManager.hh \
		src/corba/MailerSK.cc \
		src/corba/MailerDynSK.cc \
		src/corba/Mailer.hh \
		src/corba/ZoneSK.cc \
		src/corba/ZoneDynSK.cc \
		src/corba/Zone.hh \
		src/corba/_dataTypesSK.cc \
		src/corba/_dataTypesDynSK.cc \
		src/corba/_dataTypes.hh \
		src/corba/DateTimeSK.cc \
		src/corba/DateTimeDynSK.cc \
		src/corba/DateTime.hh \
		src/corba/NullableTypesSK.cc \
		src/corba/NullableTypesDynSK.cc \
		src/corba/NullableTypes.hh \
		src/corba/FiltersSK.cc \
		src/corba/FiltersDynSK.cc \
		src/corba/Filters.hh \
		src/corba/RegistrySK.cc \
		src/corba/RegistryDynSK.cc \
		src/corba/Registry.hh \
		src/corba/WhoisSK.cc \
		src/corba/WhoisDynSK.cc \
		src/corba/Whois.hh \
		src/corba/Whois2SK.cc \
		src/corba/Whois2DynSK.cc \
		src/corba/Whois2.hh \
		src/corba/AdminSK.cc \
		src/corba/AdminDynSK.cc \
		src/corba/Admin.hh \
		src/corba/LoggerSK.cc \
		src/corba/LoggerDynSK.cc \
		src/corba/Logger.hh \
		src/corba/TechCheckSK.cc \
		src/corba/TechCheckDynSK.cc \
		src/corba/TechCheck.hh \
		src/corba/EPPSK.cc \
		src/corba/EPPDynSK.cc \
		src/corba/EPP.hh \
		src/corba/ZoneGeneratorSK.cc \
		src/corba/ZoneGeneratorDynSK.cc \
		src/corba/ZoneGenerator.hh \
		src/corba/ContactVerificationSK.cc \
		src/corba/ContactVerificationDynSK.cc \
		src/corba/ContactVerification.hh \
		src/corba/DomainBrowserSK.cc \
		src/corba/DomainBrowserDynSK.cc \
		src/corba/DomainBrowser.hh \
		src/corba/AdminContactVerificationSK.cc \
		src/corba/AdminContactVerificationDynSK.cc \
		src/corba/AdminContactVerificationSK.h \
		src/corba/AdminContactVerification.hh



fredconfdir = $(sysconfdir)/fred
rcconfdir = $(sysconfdir)/init.d


EXTRA_DIST = \
		fred-server.spec \
		util/sms_send.bash \
		tests/deprecated/test_file.pdf

dist_fredconf_DATA = \
		config/server.conf \
		config/hpmail.conf \
		config/optysmail.conf

dist_rcconf_SCRIPTS = \
		init-scripts/fred-server


if DX_COND_html
doc_DATA = @DX_DOCDIR@/html/*
$(doc_DATA): doxygen-doc
MOSTLYCLEANFILES = ${DX_CLEANFILES}
endif


noinst_HEADERS = \
		include/fredlib/contact.h \
		include/fredlib/domain.h \
		include/fredlib/fredlib.h \
		include/fredlib/keyset.h \
		include/fredlib/nsset.h \
		include/fredlib/admin_contact_verification.h \
		include/admin/admin_contact_verification.h \
		include/fredlib/registrar.h \
		util/util.h \
		util/singleton.h \
		util/time_clock.h \
		util/settings.h \
		util/file_lock.h \
		util/base_exception.h \
		util/random.h \
		util/concurrent_queue.h \
		util/concurrent_set.h \
		util/random_data_generator.h \
		util/corba_wrapper.h \
		util/subprocess.h \
		util/string_split.h \
		util/corba_wrapper_decl.h \
		util/map_at.h \
		util/setup_server.h \
		util/setup_server_decl.h \
		util/cfg/config_handler.h \
		util/cfg/faked_args.h \
		util/cfg/handle_args.h \
		util/cfg/handle_corbanameservice_args.h \
		util/decimal/decimal.h \
		util/cfg/handle_database_args.h \
		util/cfg/handle_general_args.h \
		util/cfg/handle_server_args.h \
		util/cfg/handle_threadgroup_args.h \
		util/cfg/test_custom_main.h \
		util/cfg/handle_logging_args.h \
		util/cfg/handle_registry_args.h \
		util/cfg/config_handler_decl.h \
		util/cfg/handle_mojeid_args.h \
		util/cfg/handle_administrativeblocking_args.h \
		util/cfg/handle_adifd_args.h \
		util/cfg/handle_logd_args.h \
		util/cfg/handle_rifd_args.h \
		util/cfg/command_selection_args.h \
		util/cfg/check_args.h \
		util/cfg/validate_args.h \
		util/cfg/handle_sms_args.h \
		util/cfg/handle_contactverification_args.h \
		util/optys/handle_optys_mail_args.h \
		util/types/optional.h \
		util/types/optional_from_program_options.h \
		util/types/money.h \
		util/xmlgen.h \
		tests/util/exception_test.h \
		util/factory.h \
		util/factory_check.h \
		util/map_get.h \
		util/uuid.h \
		src/corba/common_wrappers.h \
		src/corba/contact_verification/corba_conversion.h \
		util/optional_value.h \
		util/is_equal_optional_nullable.h \
		util/output_indenter.h \
		src/corba/util/corba_conversions_datetime.h \
		src/corba/util/corba_conversions_nullable_types.h \
		src/corba/util/corba_conversions_string.h \
		util/is_equal_optional_nullable.h \
		src/fredlib/contact/verification/enum_testsuite_handle.h \
		src/fredlib/contact/verification/enum_check_status.h \
		src/fredlib/contact/verification/enum_test_status.h \
		src/fredlib/contact/verification/exceptions.h \
		src/admin/contact/verification/test_impl/test_interface.h \
		src/admin/contact/verification/exceptions.h



noinst_LIBRARIES = \
		util/libutil.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		src/old_utils/liboldutils.a \
		src/model/libmodel.a \
		src/fredlib/libfred.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/banking/libbanking.a \
		src/corba/libcorba.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamojeid.a \
		src/corba/libcorbazone.a \
		src/corba/libfilemanager.a \
		src/corba/libmailer.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/whois/libwhois_impl.a \
		src/corba/whois/libwhois2_impl.a \
		src/corba/epp/libepp_impl.a \
		src/corba/log/liblog_impl.a \
		util/hp/libhp.a \
		src/fredlib/notifier/libnotifier.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorbanullabletypes.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbawhois2.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbazonegenerator.a \
		src/corba/libcorbaadministrativeblocking.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/corba/libcorbacontactverification.a \
		src/corba/libcorbadomainbrowser.a \
		src/corba/libcorbaadmincontactverification.a \
		src/corba/util/libcorbautil.a \
		src/admin/contact/verification/libadmin_contact_verification.a


noinst_PROGRAMS = \
		util/log/test-log \
		util/db/test-db \
		util/db/test-mockdb \
		util/hp/client-hp \
		util/hp/crc32 \
		src/model/test-model \
		$(SERVER_TESTS)


sbin_PROGRAMS = \
		src/corba/fred-rifd \
		src/corba/fred-adifd \
		src/corba/fred-pifd \
		src/corba/fred-logd \
		src/corba/messages/fred-msgd \
		src/corba/mojeid/fred-mifd \
		src/corba/domain_browser/fred-dbifd \
		src/cli_admin/fred-admin \
		tools/logd_migration/migrate_log


# util/libutil.a
#
util_libutil_a_SOURCES = \
		util/random.cc \
		util/util.cc \
		util/output_indenter.cc \
		util/printable.h \
		util/printable.cc \
		util/xmlgen.cc \
		util/subprocess.cc
#
# util/libutil.a


# util/log/liblog.a
#
util_log_liblog_a_SOURCES = \
		util/log/logger.cc \
		util/log/log.cc \
		util/log/log_types.cc \
		util/log/context.cc \
		util/log/log.h \
		util/log/log_types.h \
		util/log/logger.h \
		util/log/context.h
#
# util/log/liblog.a


# util/log/test-log
#
util_log_test_log_SOURCES = \
		util/log/test-log.cc \
		$(UTIL_LOG_SRC)

util_log_test_log_CPPFLAGS = \
		$(AM_CPPFLAGS)

util_log_test_log_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

util_log_test_log_LDADD = \
		util/log/liblog.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# util/log/test-log


# util/types/libtypes.a
#
util_types_libtypes_a_SOURCES = \
		util/types/date.cc \
		util/types/date.h \
		util/types/date_interval.cc \
		util/types/date_interval.h \
		util/types/datetime.cc \
		util/types/datetime.h \
		util/types/datetime_interval.cc \
		util/types/datetime_interval.h \
		util/types/datetime_common.h \
		util/types/common.h \
		util/types/id.cc \
		util/types/id.h \
		util/types/null.h \
		util/types/data_types.h \
		util/types/convert_common.h \
		util/types/convert_str_base.h \
		util/types/convert_str_pod.h \
		util/types/convert_str_boost_datetime.h \
		util/types/convert_sql_base.h \
		util/types/convert_sql_pod.h \
		util/types/convert_sql_boost_datetime.h \
		util/types/convert_sql_db_types.h \
		util/types/stringify.h \
		util/types/sqlize.h \
		util/types/birthdate.cc \
		util/types/birthdate.h
#
# util/types/libtypes.a


# util/db/libdb.a
#
util_db_libdb_a_SOURCES = \
		util/db/query/query_old.cc \
		util/db/query/filter.cc \
		util/db/query/compound_filter.cc \
		util/db/query/simple_filter.cc \
		util/db/query/base_filters.h \
		util/db/query/query.h \
		util/db/query/query_common.h \
		util/db/query/query_old.h \
		util/db/query/filter.h \
		util/db/query/compound_filter.h \
		util/db/query/simple_filter.h \
		util/db/query/simple_filters.h \
		util/db/query/sql_helper_objects.h \
		util/db/query/filter_it.h \
		util/db/query/sql_operators.h \
		util/db/psql/psql_connection.h \
		util/db/psql/psql_result.h \
		util/db/model/field.h \
		util/db/model/job_queue.h \
		util/db/model/mem_ptr.h \
		util/db/model/model_exception.h \
		util/db/model/model_field_attrs.h \
		util/db/model/model_field.h \
		util/db/model/model_field_list.h \
		util/db/model/model_field_macros.h \
		util/db/model/model.h \
		util/db/connection_factory.h \
		util/db/transaction.h \
		util/db/connection.h \
		util/db/query_param.h \
		util/db/result.h \
		util/db/row.h \
		util/db/value.h \
		util/db/value.cc \
		util/db/statement.h \
		util/db/sequence.h \
		util/db/manager.h \
		util/db/manager_tss.h \
		util/db/db_exceptions.h \
		util/db/database.h \
		util/db/manager_old_db.h
#
# util/db/libdb.a


# util/db/test-db
#
util_db_test_db_SOURCES = \
		util/db/test-db.cc

util_db_test_db_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

util_db_test_db_LDADD = \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# util/db/test-db


# util/db/mock
#
UTIL_MOCKDB_SRC = \
		util/db/query/query_old.cc \
		util/db/query/filter.cc \
		util/db/query/compound_filter.cc \
		util/db/query/simple_filter.cc \
		util/db/query/base_filters.h \
		util/db/query/query.h \
		util/db/query/query_common.h \
		util/db/query/query_old.h \
		util/db/query/filter.h \
		util/db/query/compound_filter.h \
		util/db/query/simple_filter.h \
		util/db/query/simple_filters.h \
		util/db/query/sql_helper_objects.h \
		util/db/query/filter_it.h \
		util/db/query/sql_operators.h \
		util/db/mock/mock_connection.h \
		util/db/mock/mock_result.h \
		util/db/model/field.h \
		util/db/model/job_queue.h \
		util/db/model/mem_ptr.h \
		util/db/model/model_exception.h \
		util/db/model/model_field_attrs.h \
		util/db/model/model_field.h \
		util/db/model/model_field_list.h \
		util/db/model/model_field_macros.h \
		util/db/model/model.h \
		util/db/connection_factory.h \
		util/db/transaction.h \
		util/db/connection.h \
		util/db/result.h \
		util/db/row.h \
		util/db/value.h \
		util/db/statement.h \
		util/db/sequence.h \
		util/db/manager.h \
		util/db/manager_tss.h \
		util/db/db_exceptions.h \
		util/db/manager_old_db.h


util_db_test_mockdb_SOURCES = \
		util/db/test-mockdb.cc \
		$(UTIL_MOCKDB_SRC)

util_db_test_mockdb_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

util_db_test_mockdb_LDADD = \
		util/log/liblog.a \
		util/types/libtypes.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SYSTEM_LIB)

#
# util/db/mock


# util/hp/libhp.a
#
util_hp_libhp_a_SOURCES = \
		util/hp/hp.cc \
		util/hp/hp.h \
		util/hp/hpmail.cc \
		util/hp/hpmail.h \
		util/hp/hpmailbatchstate.cc \
		util/hp/hpmailbatchstate.h
#
# util/hp/libhp.a


# util/hp/client-hp
#
util_hp_client_hp_SOURCES = \
		util/hp/client-hp.cc \
		util/hp/handle_hpmail_args.h \
		util/hp/handle_hpmailbatchstate_args.h \
		util/hp/handle_clienthp_args.h

util_hp_client_hp_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

util_hp_client_hp_LDADD = \
		util/hp/libhp.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB)
#
# util/hp/client-hp


# util/hp/crc32
#
util_hp_crc32_SOURCES = \
		util/hp/crc32.cc

util_hp_crc32_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

util_hp_crc32_LDADD = \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB)
#
# util/hp/crc32

# src/corba/util/libcorbautil.a
#
src_corba_util_libcorbautil_a_SOURCES = \
		src/corba/util/corba_conversions_datetime.cc \
		src/corba/util/corba_conversions_nullable_types.cc \
		src/corba/util/corba_conversions_string.cc
#
# src/corba/util/libcorbautil.a


# src/old_utils/liboldutils.a
#
src_old_utils_liboldutils_a_SOURCES = \
		src/old_utils/dbsql.cc \
		src/old_utils/log.cc \
		src/old_utils/pqsql.cc \
		src/old_utils/timeclock.cc \
		src/old_utils/util.cc \
		src/old_utils/dbsql.h \
		src/old_utils/log.h \
		src/old_utils/pqsql.h \
		src/old_utils/timeclock.h \
		src/old_utils/util.h
#
# src/old_utils/liboldutils.a


# src/model/libmodel.a
#
src_model_libmodel_a_SOURCES = \
		src/model/object_registry_filter.cc \
		src/model/object_registry_filter.h \
		src/model/object_filter.cc \
		src/model/object_filter.h \
		src/model/bank_statement_filter.cc \
		src/model/bank_statement_filter.h \
		src/model/bank_payment_filter.cc \
		src/model/bank_payment_filter.h \
		src/model/domain_filter.cc \
		src/model/domain_filter.h \
		src/model/contact_filter.cc \
		src/model/contact_filter.h \
		src/model/nsset_filter.cc \
		src/model/nsset_filter.h \
		src/model/registrar_filter.cc \
		src/model/registrar_filter.h \
		src/model/filter_filter.cc \
		src/model/filter_filter.h \
		src/model/object_state_filter.cc \
		src/model/object_state_filter.h \
		src/model/invoice_filter.cc \
		src/model/invoice_filter.h \
		src/model/public_request_filter.cc \
		src/model/public_request_filter.h \
		src/model/mail_filter.cc \
		src/model/mail_filter.h \
		src/model/file_filter.cc \
		src/model/file_filter.h \
		src/model/keyset_filter.cc \
		src/model/keyset_filter.h \
		src/model/online_statement_filter.cc \
		src/model/online_statement_filter.h \
		src/model/log_filter.cc \
		src/model/union_filter.cc \
		src/model/union_filter.h \
		src/model/zone_filter.cc \
		src/model/zone_filter.h \
		src/model/zone_ns_filter.cc \
		src/model/zone_ns_filter.h \
		src/model/zone_soa_filter.cc \
		src/model/zone_soa_filter.h \
		src/model/message_filter.cc \
		src/model/message_filter.h \
		src/model/log_filter.h \
		src/model/model_filters.h
#
# src/model/libmodel.a

# src/model/test-model
#
src_model_test_model_SOURCES = \
		src/model/test-model.cc

src_model_test_model_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_model_test_model_LDADD = \
		src/model/libmodel.a \
		src/fredlib/banking/libbanking.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/model/test-model


# src/fredlib/libfred.a
#
src_fredlib_libfred_a_SOURCES = \
		src/fredlib/registry.cc \
		src/fredlib/registry.h \
		src/fredlib/db_settings.h \
		src/fredlib/common_new.h \
		src/fredlib/common_object.h \
		src/fredlib/common_impl.cc \
		src/fredlib/common_impl.h \
		src/fredlib/object.h \
		src/fredlib/object_impl.cc \
		src/fredlib/object_impl.h \
		src/fredlib/common_impl_new.cc \
		src/fredlib/common_impl_new.h \
		src/fredlib/model_files.cc \
		src/fredlib/model_files.h \
		src/fredlib/model_registrar_acl.cc \
		src/fredlib/model_registrar_acl.h \
		src/fredlib/model_registrar.cc \
		src/fredlib/model_registrar.h \
		src/fredlib/model_zone.cc \
		src/fredlib/model_zone.h \
		src/fredlib/model_zone_ns.cc \
		src/fredlib/model_zone_ns.h \
		src/fredlib/model_zone_soa.cc \
		src/fredlib/model_zone_soa.h \
		src/fredlib/model_price_list.cc \
		src/fredlib/model_price_list.h \
		src/fredlib/model_registrar_certification.h \
		src/fredlib/model_registrar_certification.cc \
		src/fredlib/model_registrar_group.h \
		src/fredlib/model_registrar_group.cc \
		src/fredlib/model_registrar_group_map.h \
		src/fredlib/model_registrar_group_map.cc \
		src/fredlib/domain.cc \
		src/fredlib/domain.h \
		src/fredlib/contact.cc \
		src/fredlib/contact.h \
		src/fredlib/nsset.cc \
		src/fredlib/nsset.h \
		src/fredlib/keyset.cc \
		src/fredlib/keyset.h \
		src/fredlib/registrar.cc \
		src/fredlib/registrar.h \
		src/fredlib/credit.cc \
		src/fredlib/credit.h \
		src/fredlib/invoicing/invoice.cc \
		src/fredlib/invoicing/invoice.h \
		src/fredlib/zone.cc \
		src/fredlib/zone.h \
		src/fredlib/blacklist.cc \
		src/fredlib/blacklist.h \
		src/fredlib/documents.cc \
		src/fredlib/documents.h \
		src/fredlib/info_buffer.cc \
		src/fredlib/info_buffer.h \
		src/fredlib/mail.cc \
		src/fredlib/mail.h \
		src/fredlib/notify.h \
		src/fredlib/notify.cc \
		src/fredlib/logger_client.h \
		src/fredlib/epp_corba_client.h \
		src/fredlib/poll.cc \
		src/fredlib/poll.h \
		src/fredlib/file_transferer.h \
		src/fredlib/file.cc \
		src/fredlib/file.h \
		src/fredlib/filter.cc \
		src/fredlib/filter.h \
		src/fredlib/mailer.h \
		src/fredlib/sql.h \
		src/fredlib/exceptions.h \
		src/fredlib/types.h \
		src/fredlib/obj_types.h \
		src/fredlib/object_states.cc \
		src/fredlib/object_states.h \
		src/fredlib/contact_diff.cc \
		src/fredlib/contact_diff.h \
		src/fredlib/domain_diff.cc \
		src/fredlib/domain_diff.h \
		src/fredlib/nsset_diff.cc \
		src/fredlib/nsset_diff.h \
		src/fredlib/keyset_diff.cc \
		src/fredlib/keyset_diff.h \
		src/fredlib/common_diff.cc \
		src/fredlib/common_diff.h \
		src/fredlib/reminder.cc \
		src/fredlib/reminder.h \
		src/fredlib/opcontext.cc \
		src/fredlib/opcontext.h \
		src/fredlib/contact/copy_contact.h \
		src/fredlib/contact/copy_contact.cc \
		src/fredlib/domain/update_domain.h \
		src/fredlib/domain/update_domain.cc \
		src/fredlib/domain/info_domain_data.h \
		src/fredlib/domain/info_domain_data.cc \
		src/fredlib/domain/info_domain_diff.h \
		src/fredlib/domain/info_domain_diff.cc \
		src/fredlib/domain/info_domain.h \
		src/fredlib/domain/info_domain.cc \
		src/fredlib/domain/info_domain_impl.cc \
		src/fredlib/domain/info_domain_impl.h \
		src/fredlib/domain/info_domain_output.cc \
		src/fredlib/domain/info_domain_output.h \
		src/fredlib/domain/enum_validation_extension.h \
		src/fredlib/domain/create_domain_name_blacklist_id.h \
		src/fredlib/domain/create_domain_name_blacklist_id.cc \
		src/fredlib/nsset/update_nsset.h \
		src/fredlib/nsset/update_nsset.cc \
		src/fredlib/nsset/info_nsset.h \
		src/fredlib/nsset/info_nsset.cc \
		src/fredlib/nsset/info_nsset_data.h \
		src/fredlib/nsset/info_nsset_data.cc \
		src/fredlib/nsset/info_nsset_diff.h \
		src/fredlib/nsset/info_nsset_diff.cc \
		src/fredlib/nsset/info_nsset_impl.cc \
		src/fredlib/nsset/info_nsset_impl.h \
		src/fredlib/nsset/info_nsset_output.cc \
		src/fredlib/nsset/info_nsset_output.h \
		src/fredlib/keyset/update_keyset.h \
		src/fredlib/keyset/keyset_dns_key.h \
		src/fredlib/keyset/update_keyset.cc \
		src/fredlib/keyset/create_keyset.cc \
		src/fredlib/keyset/create_keyset.h \
		src/fredlib/keyset/info_keyset_data.h \
		src/fredlib/keyset/info_keyset_data.cc \
		src/fredlib/keyset/info_keyset_diff.h \
		src/fredlib/keyset/info_keyset_diff.cc \
		src/fredlib/keyset/info_keyset.h \
		src/fredlib/keyset/info_keyset.cc \
		src/fredlib/keyset/info_keyset_impl.cc \
		src/fredlib/keyset/info_keyset_impl.h \
		src/fredlib/keyset/info_keyset_output.cc \
		src/fredlib/keyset/info_keyset_output.h \
		src/fredlib/contact/delete_contact.h \
		src/fredlib/contact/delete_contact.cc \
		src/fredlib/contact/info_contact_data.h \
		src/fredlib/contact/info_contact_data.cc \
		src/fredlib/contact/info_contact_diff.h \
		src/fredlib/contact/info_contact_diff.cc \
		src/fredlib/contact/info_contact.h \
		src/fredlib/contact/info_contact.cc \
		src/fredlib/contact/info_contact_impl.cc \
		src/fredlib/contact/info_contact_impl.h \
		src/fredlib/contact/info_contact_output.cc \
		src/fredlib/contact/info_contact_output.h \
		src/fredlib/contact/place_address.cc \
		src/fredlib/contact/place_address.h \
		src/fredlib/object/object.h \
		src/fredlib/object/object.cc \
		src/fredlib/object/object_impl.h \
		src/fredlib/object/object_impl.cc \
		src/fredlib/poll/message_types.h \
		src/fredlib/poll/create_poll_message_impl.h \
		src/fredlib/poll/create_poll_message_impl.cc \
		src/fredlib/poll/create_epp_action_poll_message_impl.h \
		src/fredlib/poll/create_epp_action_poll_message_impl.cc \
		src/fredlib/poll/create_update_object_poll_message.h \
		src/fredlib/poll/create_update_object_poll_message.cc \
		src/fredlib/poll/create_delete_contact_poll_message.h \
		src/fredlib/poll/create_delete_contact_poll_message.cc \
		src/fredlib/poll/create_delete_domain_poll_message.h \
		src/fredlib/poll/create_delete_domain_poll_message.cc \
		src/fredlib/opexception.h \
		src/fredlib/contact/merge_contact.h \
		src/fredlib/contact/merge_contact.cc \
		src/fredlib/contact/create_contact.cc \
		src/fredlib/contact/create_contact.h \
		src/fredlib/contact/update_contact.cc \
		src/fredlib/contact/update_contact.h \
		src/admin/contact/verification/contact_states/delete_all.h \
		src/admin/contact/verification/contact_states/delete_all.cc \
		src/fredlib/contact/contact_enum.h \
		src/fredlib/nsset/create_nsset.cc \
		src/fredlib/nsset/create_nsset.h \
		src/fredlib/nsset/nsset_dns_host.h \
		src/fredlib/domain/create_domain.cc \
		src/fredlib/domain/create_domain.h \
		src/fredlib/contact/merge_contact_selection.cc \
		src/fredlib/contact/merge_contact_selection.h \
		src/fredlib/contact/find_contact_duplicates.cc \
		src/fredlib/contact/find_contact_duplicates.h \
		src/fredlib/contact/merge_contact_email_notification_data.cc \
		src/fredlib/contact/merge_contact_email_notification_data.h \
		src/fredlib/domain/delete_domain.h \
		src/fredlib/domain/delete_domain.cc \
		src/fredlib/domain/domain_name.h \
		src/fredlib/domain/domain_name.cc \
		src/fredlib/zone/zone.h \
		src/fredlib/zone/zone.cc \
		src/fredlib/contact/verification/create_check.h \
		src/fredlib/contact/verification/create_check.cc \
		src/fredlib/contact/verification/update_check.h \
		src/fredlib/contact/verification/update_check.cc \
		src/fredlib/contact/verification/info_check.h \
		src/fredlib/contact/verification/info_check.cc \
		src/fredlib/contact/verification/list_checks.h \
		src/fredlib/contact/verification/list_checks.cc \
		src/fredlib/contact/verification/list_enum_objects.h \
		src/fredlib/contact/verification/list_enum_objects.cc \
		src/fredlib/contact/verification/create_test.h \
		src/fredlib/contact/verification/create_test.cc \
		src/fredlib/contact/verification/update_test.h \
		src/fredlib/contact/verification/update_test.cc \
		src/fredlib/domain/check_domain.h \
		src/fredlib/domain/check_domain.cc \
		src/fredlib/contact/check_contact.cc \
		src/fredlib/contact/check_contact.h \
		src/fredlib/nsset/check_nsset.cc \
		src/fredlib/nsset/check_nsset.h \
		src/fredlib/nsset/delete_nsset.cc \
		src/fredlib/nsset/delete_nsset.h \
		src/fredlib/keyset/check_keyset.cc \
		src/fredlib/keyset/check_keyset.h \
		src/fredlib/object/check_handle.cc \
		src/fredlib/object/check_handle.h\
		src/fredlib/keyset/delete_keyset.cc \
		src/fredlib/keyset/delete_keyset.h \
		src/fredlib/registrar/registrar_impl.h \
		src/fredlib/registrar/registrar_impl.cc \
		src/fredlib/registrar/info_registrar.cc \
		src/fredlib/registrar/info_registrar.h \
		src/fredlib/registrar/info_registrar_data.cc \
		src/fredlib/registrar/info_registrar_data.h \
		src/fredlib/registrar/info_registrar_diff.cc \
		src/fredlib/registrar/info_registrar_diff.h \
		src/fredlib/registrar/registrar_zone_access.h \
		src/fredlib/registrar/create_registrar.cc \
		src/fredlib/registrar/create_registrar.h \
		src/fredlib/registrar/info_registrar_impl.cc \
		src/fredlib/registrar/info_registrar_impl.h \
		src/fredlib/registrar/info_registrar_output.cc \
		src/fredlib/registrar/info_registrar_output.h \
		src/fredlib/registrar/get_registrar_handles.cc \
		src/fredlib/registrar/get_registrar_handles.h \
		src/fredlib/object/object_id_handle_pair.h \
		src/fredlib/object/object_id_handle_pair.cc \
		src/fredlib/object_state/object_state_name.h \
		src/fredlib/object_state/object_has_state.h \
		src/fredlib/object_state/object_has_state.cc \
		src/fredlib/object_state/lock_multiple_object_state_request_lock.cc \
		src/fredlib/object_state/lock_multiple_object_state_request_lock.h \
		src/fredlib/object_state/lock_object_state_request_lock.h \
		src/fredlib/object_state/lock_object_state_request_lock.cc \
		src/fredlib/object_state/get_object_state_descriptions.h \
		src/fredlib/object_state/get_object_state_descriptions.cc \
		src/fredlib/object_state/get_object_states.h \
		src/fredlib/object_state/get_object_states.cc \
		src/fredlib/object_state/get_object_state_id_map.h \
		src/fredlib/object_state/get_object_state_id_map.cc \
		src/fredlib/object_state/get_blocking_status_desc_list.cc \
		src/fredlib/object_state/get_blocking_status_desc_list.h \
		src/fredlib/object_state/perform_object_state_request.h \
		src/fredlib/object_state/perform_object_state_request.cc \
		src/fredlib/object_state/cancel_object_state_request.cc \
		src/fredlib/object_state/cancel_object_state_request.h \
		src/fredlib/object_state/cancel_object_state_request_id.cc \
		src/fredlib/object_state/cancel_object_state_request_id.h \
		src/fredlib/object_state/create_object_state_request.cc \
		src/fredlib/object_state/create_object_state_request.h \
		src/fredlib/object_state/create_object_state_request_id.cc \
		src/fredlib/object_state/create_object_state_request_id.h \
		src/fredlib/object_state/create_admin_object_block_request.cc \
		src/fredlib/object_state/create_admin_object_block_request.h \
		src/fredlib/object_state/create_admin_object_block_request_id.cc \
		src/fredlib/object_state/create_admin_object_block_request_id.h \
		src/fredlib/object_state/create_admin_object_state_restore_request.cc \
		src/fredlib/object_state/create_admin_object_state_restore_request.h \
		src/fredlib/object_state/create_admin_object_state_restore_request_id.cc \
		src/fredlib/object_state/create_admin_object_state_restore_request_id.h \
		src/fredlib/object_state/clear_admin_object_state_request_id.cc \
		src/fredlib/object_state/clear_admin_object_state_request_id.h \
		src/fredlib/object_state/clear_object_state_request.cc \
		src/fredlib/object_state/clear_object_state_request.h \
		src/fredlib/object_state/clear_object_state_request_id.cc \
		src/fredlib/object_state/clear_object_state_request_id.h \
		src/fredlib/object/object_id_handle_pair.h \
		src/fredlib/object/object_id_handle_pair.cc
#
# src/fredlib/libfred.a


# src/fredlib/public_request/libpublic_request.a
#
src_fredlib_public_request_libpublic_request_a_SOURCES = \
		src/fredlib/public_request/public_request.cc \
		src/fredlib/public_request/public_request.h \
		src/fredlib/public_request/public_request_impl.cc \
		src/fredlib/public_request/public_request_impl.h \
		src/fredlib/public_request/public_request_authinfo_impl.cc \
		src/fredlib/public_request/public_request_authinfo_impl.h \
		src/fredlib/public_request/public_request_block_impl.cc \
		src/fredlib/public_request/public_request_block_impl.h
#
# src/fredlib/public_request/libpublic_request.a


# src/fredlib/banking/libbanking.a
#
src_fredlib_banking_libbanking_a_SOURCES = \
		src/fredlib/banking/model_bank_statement.cc \
		src/fredlib/banking/model_bank_statement.h \
		src/fredlib/banking/model_bank_payment.cc \
		src/fredlib/banking/model_bank_payment.h \
		src/fredlib/banking/bank_common.cc \
		src/fredlib/banking/bank_common.h \
		src/fredlib/banking/bank_payment.h \
		src/fredlib/banking/bank_payment_impl.h \
		src/fredlib/banking/bank_payment_list.h \
		src/fredlib/banking/bank_payment_list_impl.h \
		src/fredlib/banking/bank_manager.cc \
		src/fredlib/banking/bank_manager.h \
		src/fredlib/banking/bank_exporter.cc \
		src/fredlib/banking/bank_exporter.h \
		src/fredlib/banking/bank_statement.h \
		src/fredlib/banking/bank_statement_impl.h \
		src/fredlib/banking/bank_statement_list.h \
		src/fredlib/banking/bank_statement_list_impl.h
#
# src/fredlib/banking/libbanking.a


# src/fredlib/messages/libmessages.a
#
src_fredlib_messages_libmessages_a_SOURCES = \
		src/fredlib/messages/messages_impl.cc \
		src/fredlib/messages/messages_impl.h
#
# src/fredlib/messages/libmessages.a


# src/fredlib/requests/librequests.a
#
src_fredlib_requests_librequests_a_SOURCES = \
		src/fredlib/requests/model_request_property_name.h \
		src/fredlib/requests/model_request.cc \
		src/fredlib/requests/model_request_property_value.cc \
		src/fredlib/requests/model_request_data.cc \
		src/fredlib/requests/model_request_property_value.h \
		src/fredlib/requests/model_request_data.h \
		src/fredlib/requests/model_session.cc \
		src/fredlib/requests/model_request.h \
		src/fredlib/requests/model_session.h \
		src/fredlib/requests/model_request_property_name.cc \
		src/fredlib/requests/session.h \
		src/fredlib/requests/session.cc \
		src/fredlib/requests/request.h \
		src/fredlib/requests/request_impl.h \
		src/fredlib/requests/request_list.h \
		src/fredlib/requests/request_list.cc \
		src/fredlib/requests/request_manager.h \
		src/fredlib/requests/request_manager.cc \
		src/fredlib/requests/request_cache.h \
		src/fredlib/requests/request_cache.cc \
		src/fredlib/requests/request_property_name_cache.h \
		src/fredlib/requests/request_property_name_cache.cc \
		src/fredlib/requests/session_cache.h \
		src/fredlib/requests/session_cache.cc
#
# src/fredlib/requests/librequests.a


# src/fredlib/notifier/libnotifier.a
#
src_fredlib_notifier_libnotifier_a_SOURCES = \
		src/fredlib/notifier/request_notification.h \
		src/fredlib/notifier/request_notification.cc \
		src/fredlib/notifier/request_ntf_handler_contact.h \
		src/fredlib/notifier/request_ntf_handler_contact.cc \
		src/fredlib/notifier/request_ntf_sender_email.h \
		src/fredlib/notifier/ntf_except.h
#
# src/fredlib/notifier/libnotifier.a


# src/fredlib/contact_verification/libcontact_verification.a
#
src_fredlib_contact_verification_libcontact_verification_a_SOURCES = \
		src/contact_verification/public_request_contact_verification_impl.cc \
		src/contact_verification/public_request_contact_verification_impl.h \
		util/db/nullable.h \
		src/fredlib/contact_verification/contact.h \
		src/fredlib/contact_verification/contact.cc \
		src/fredlib/contact_verification/contact_verification_checkers.h \
		src/fredlib/contact_verification/contact_verification_checkers.cc \
		src/fredlib/contact_verification/contact_conditional_identification_impl.cc \
		src/fredlib/contact_verification/contact_conditional_identification_impl.h \
		src/fredlib/contact_verification/contact_identification_impl.cc \
		src/fredlib/contact_verification/contact_identification_impl.h \
		src/fredlib/contact_verification/contact_verification_validators.cc \
		src/fredlib/contact_verification/contact_verification_validators.h \
		src/fredlib/contact_verification/contact_verification_password.cc \
		src/fredlib/contact_verification/contact_verification_password.h \
		src/fredlib/contact_verification/contact_validator.h
#
# src/fredlib/contact_verification/libcontact_verification.a

# src/admin/contact/verification/libadmin_contact_verification.a
#
src_admin_contact_verification_libadmin_contact_verification_a_SOURCES = \
		src/admin/contact/verification/create_test_impl_prototypes.h \
		src/admin/contact/verification/create_test_impl_prototypes.cc \
		src/admin/contact/verification/delete_domains_of_invalid_contact.h \
		src/admin/contact/verification/delete_domains_of_invalid_contact.cc \
		src/admin/contact/verification/enqueue_check.h \
		src/admin/contact/verification/enqueue_check.cc \
		src/admin/contact/verification/exceptions.h \
		src/admin/contact/verification/fill_check_queue.h \
		src/admin/contact/verification/fill_check_queue.cc \
		src/admin/contact/verification/list_active_checks.h \
		src/admin/contact/verification/list_active_checks.cc \
		src/admin/contact/verification/related_records.h \
		src/admin/contact/verification/related_records.cc \
		src/admin/contact/verification/resolve_check.h \
		src/admin/contact/verification/resolve_check.cc \
		src/admin/contact/verification/run_all_enqueued_checks.h \
		src/admin/contact/verification/run_all_enqueued_checks.cc \
		src/admin/contact/verification/update_tests.h \
		src/admin/contact/verification/update_tests.cc \
		src/admin/contact/verification/contact_states/enum.h \
		src/admin/contact/verification/contact_states/delete_all.h \
		src/admin/contact/verification/contact_states/delete_all.cc \
		src/admin/contact/verification/test_impl/test_contactability.h \
		src/admin/contact/verification/test_impl/test_contactability.cc \
		src/admin/contact/verification/test_impl/test_cz_address_exists.h \
		src/admin/contact/verification/test_impl/test_cz_address_exists.cc \
		src/admin/contact/verification/test_impl/test_email_exists.h \
		src/admin/contact/verification/test_impl/test_email_exists.cc \
		src/admin/contact/verification/test_impl/test_email_syntax.h \
		src/admin/contact/verification/test_impl/test_email_syntax.cc \
		src/admin/contact/verification/test_impl/test_interface.h \
		src/admin/contact/verification/test_impl/test_interface.cc \
		src/admin/contact/verification/test_impl/test_name_syntax.h \
		src/admin/contact/verification/test_impl/test_name_syntax.cc \
		src/admin/contact/verification/test_impl/test_phone_syntax.h \
		src/admin/contact/verification/test_impl/test_phone_syntax.cc \
		src/admin/contact/verification/test_impl/test_send_letter.h \
		src/admin/contact/verification/test_impl/test_send_letter.cc
#
# src/admin/contact/verification/libadmin_contact_verification.a

BUILT_SOURCES = \
		src/corba/MessagesSK.cc \
		src/corba/MojeIDSK.cc \
		src/corba/AdministrativeBlockingSK.cc \
		src/corba/FileManagerSK.cc \
		src/corba/MailerSK.cc \
		src/corba/ZoneSK.cc \
		src/corba/DateTimeSK.cc \
		src/corba/NullableTypesSK.cc \
		src/corba/_dataTypesSK.cc \
		src/corba/FiltersSK.cc src/corba/RegistrySK.cc \
		src/corba/WhoisSK.cc \
		src/corba/Whois2SK.cc \
		src/corba/AdminSK.cc \
		src/corba/LoggerSK.cc \
		src/corba/TechCheckSK.cc \
		src/corba/EPPSK.cc \
		src/corba/ZoneGeneratorSK.cc \
		src/corba/ContactVerificationSK.cc \
		src/corba/DomainBrowserSK.cc \
		src/corba/AdminContactVerificationSK.cc


# src/corba/libcorba.a
#
src_corba_libcorba_a_SOURCES = \
		src/corba/nameservice.cc \
		src/corba/mailer_manager.cc \
		src/corba/nameservice.h \
		src/corba/mailer_manager.h \
		src/corba/connection_releaser.h \
		src/corba/pidfile.h \
		src/corba/daemonize.h \
		src/corba/file_manager_client.cc \
		src/corba/file_manager_client.h \
		src/corba/logger_client_impl.cc \
		src/corba/logger_client_impl.h \
		src/corba/epp_corba_client_impl.cc \
		src/corba/epp_corba_client_impl.h

src_corba_libcorba_a_CPPFLAGS = \
		$(AM_CPPFLAGS)
#
# src/corba/libcorba.a


# nodist corba libs
#
nodist_src_corba_libcorbamessages_a_SOURCES = \
		src/corba/MessagesSK.cc \
		src/corba/MessagesDynSK.cc \
		src/corba/Messages.hh

nodist_src_corba_libcorbamojeid_a_SOURCES = \
		src/corba/MojeIDSK.cc \
		src/corba/MojeIDDynSK.cc \
		src/corba/MojeID.hh

nodist_src_corba_libfilemanager_a_SOURCES = \
		src/corba/FileManagerSK.cc \
		src/corba/FileManagerDynSK.cc \
		src/corba/FileManager.hh

nodist_src_corba_libmailer_a_SOURCES = \
		src/corba/MailerSK.cc \
		src/corba/MailerDynSK.cc \
		src/corba/Mailer.hh

nodist_src_corba_libcorbazone_a_SOURCES = \
		src/corba/ZoneSK.cc \
		src/corba/ZoneDynSK.cc \
		src/corba/Zone.hh

nodist_src_corba_libcorbadatatypes_a_SOURCES = \
		src/corba/_dataTypesSK.cc \
		src/corba/_dataTypesDynSK.cc \
		src/corba/_dataTypes.hh \
		src/corba/DateTimeSK.cc \
		src/corba/DateTimeDynSK.cc \
		src/corba/DateTime.hh

nodist_src_corba_libcorbanullabletypes_a_SOURCES = \
		src/corba/NullableTypesSK.cc \
		src/corba/NullableTypesDynSK.cc \
		src/corba/NullableTypes.hh

nodist_src_corba_libcorbafilters_a_SOURCES = \
		src/corba/FiltersSK.cc \
		src/corba/FiltersDynSK.cc \
		src/corba/Filters.hh

nodist_src_corba_libcorbaregistry_a_SOURCES = \
		src/corba/RegistrySK.cc \
		src/corba/RegistryDynSK.cc \
		src/corba/Registry.hh

nodist_src_corba_libcorbawhois_a_SOURCES = \
		src/corba/WhoisSK.cc \
		src/corba/WhoisDynSK.cc \
		src/corba/Whois.hh
		
nodist_src_corba_libcorbawhois2_a_SOURCES = \
		src/corba/Whois2SK.cc \
		src/corba/Whois2DynSK.cc \
		src/corba/Whois2.hh

nodist_src_corba_libcorbaadmin_a_SOURCES = \
		src/corba/AdminSK.cc \
		src/corba/AdminDynSK.cc \
		src/corba/Admin.hh

nodist_src_corba_libcorbalogger_a_SOURCES = \
		src/corba/LoggerSK.cc \
		src/corba/LoggerDynSK.cc \
		src/corba/Logger.hh

nodist_src_corba_libcorbatechcheck_a_SOURCES = \
		src/corba/TechCheckSK.cc \
		src/corba/TechCheckDynSK.cc \
		src/corba/TechCheck.hh

nodist_src_corba_libcorbaepp_a_SOURCES = \
		src/corba/EPPSK.cc \
		src/corba/EPPDynSK.cc \
		src/corba/EPP.hh

nodist_src_corba_libcorbazonegenerator_a_SOURCES = \
		src/corba/ZoneGeneratorSK.cc \
		src/corba/ZoneGeneratorDynSK.cc \
		src/corba/ZoneGenerator.hh

nodist_src_corba_libcorbacontactverification_a_SOURCES = \
		src/corba/ContactVerificationSK.cc \
		src/corba/ContactVerificationDynSK.cc \
		src/corba/ContactVerification.hh

nodist_src_corba_libcorbaadmincontactverification_a_SOURCES = \
		src/corba/AdminContactVerificationSK.cc \
		src/corba/AdminContactVerificationDynSK.cc \
		src/corba/AdminContactVerificationSK.h \
		src/corba/AdminContactVerification.hh

nodist_src_corba_libcorbaadministrativeblocking_a_SOURCES = \
		src/corba/AdministrativeBlockingSK.cc \
		src/corba/AdministrativeBlockingDynSK.cc \
		src/corba/AdministrativeBlocking.hh

nodist_src_corba_libcorbadomainbrowser_a_SOURCES = \
		src/corba/DomainBrowserSK.cc \
		src/corba/DomainBrowserDynSK.cc \
		src/corba/DomainBrowser.hh

#
# nodist corba libs


# idl build
#
src/corba/MessagesSK.cc: $(IDLDIR)/Messages.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Messages.idl

src/corba/MojeIDSK.cc: $(IDLDIR)/MojeID.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/MojeID.idl

src/corba/AdministrativeBlockingSK.cc: $(IDLDIR)/AdministrativeBlocking.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/AdministrativeBlocking.idl

src/corba/FileManagerSK.cc: $(IDLDIR)/FileManager.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/FileManager.idl

src/corba/MailerSK.cc: $(IDLDIR)/Mailer.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Mailer.idl

src/corba/ZoneSK.cc: $(IDLDIR)/Zone.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Zone.idl

src/corba/DateTimeSK.cc: $(IDLDIR)/DateTime.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/DateTime.idl

src/corba/NullableTypesSK.cc: $(IDLDIR)/NullableTypes.idl $(IDLDIR)/DateTime.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/NullableTypes.idl

src/corba/_dataTypesSK.cc: $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/_dataTypes.idl

src/corba/FiltersSK.cc: $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Filters.idl

src/corba/RegistrySK.cc: $(IDLDIR)/Registry.idl $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Registry.idl

src/corba/WhoisSK.cc: $(IDLDIR)/Whois.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Whois.idl

src/corba/Whois2SK.cc: $(IDLDIR)/Whois2.idl $(IDLDIR)/NullableTypes.idl $(IDLDIR)/DateTime.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Whois2.idl
		
src/corba/AdminSK.cc: $(IDLDIR)/Admin.idl $(IDLDIR)/Whois.idl $(IDLDIR)/Filters.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Admin.idl

src/corba/LoggerSK.cc: $(IDLDIR)/Logger.idl $(IDLDIR)/Admin.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Logger.idl

src/corba/TechCheckSK.cc: $(IDLDIR)/TechCheck.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/TechCheck.idl

src/corba/EPPSK.cc: $(IDLDIR)/EPP.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/EPP.idl

src/corba/ZoneGeneratorSK.cc: $(IDLDIR)/ZoneGenerator.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/ZoneGenerator.idl

src/corba/ContactVerificationSK.cc: $(IDLDIR)/ContactVerification.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/ContactVerification.idl

src/corba/DomainBrowserSK.cc: $(IDLDIR)/DomainBrowser.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/DomainBrowser.idl

src/corba/AdminContactVerificationSK.cc: $(IDLDIR)/AdminContactVerification.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba $(IDLDIR)/AdminContactVerification.idl

#
# idl build


# src/corba/admin/libadmin_impl.a
#
src_corba_admin_libadmin_impl_a_SOURCES = \
		src/corba/admin/admin_impl.cc \
		src/corba/admin/admin_impl.h \
		src/corba/admin/session_impl.cc \
		src/corba/admin/session_impl.h \
		src/corba/admin/user_impl.cc \
		src/corba/admin/user_impl.h \
		src/corba/admin/filter_impl.cc \
		src/corba/admin/filter_impl.h \
		src/corba/admin/pagetable_impl.cc \
		src/corba/admin/pagetable_impl.h \
		src/corba/admin/pagetable_publicrequests.cc \
		src/corba/admin/pagetable_publicrequests.h \
		src/corba/admin/pagetable_contacts.cc \
		src/corba/admin/pagetable_contacts.h \
		src/corba/admin/pagetable_domains.cc \
		src/corba/admin/pagetable_domains.h \
		src/corba/admin/pagetable_filters.cc \
		src/corba/admin/pagetable_filters.h \
		src/corba/admin/pagetable_invoices.cc \
		src/corba/admin/pagetable_invoices.h \
		src/corba/admin/pagetable_mails.cc \
		src/corba/admin/pagetable_mails.h \
		src/corba/admin/pagetable_nssets.cc \
		src/corba/admin/pagetable_nssets.h \
		src/corba/admin/pagetable_registrars.cc \
		src/corba/admin/pagetable_registrars.h \
		src/corba/admin/pagetable_files.cc \
		src/corba/admin/pagetable_files.h \
		src/corba/admin/pagetable_keysets.cc \
		src/corba/admin/pagetable_keysets.h \
		src/corba/admin/pagetable_logger.cc \
		src/corba/admin/pagetable_logger.h \
		src/corba/admin/pagetable_logsession.cc \
		src/corba/admin/pagetable_logsession.h \
		src/corba/admin/pagetable_bank_payments.cc \
		src/corba/admin/pagetable_bank_payments.h \
		src/corba/admin/pagetable_bank_statements.cc \
		src/corba/admin/pagetable_bank_statements.h \
		src/corba/admin/pagetable_zones.cc \
		src/corba/admin/pagetable_zones.h \
		src/corba/admin/pagetable_messages.cc \
		src/corba/admin/pagetable_messages.h \
		src/corba/admin/str_corbaout.h \
		src/corba/admin/usertype_conv.h \
		src/corba/admin/common.cc \
		src/corba/admin/common.h \
		src/corba/admin/zone_impl.cc \
		src/corba/admin/zone_impl.h \
		src/corba/admin/bankinginvoicing_impl.cc \
		src/corba/admin/bankinginvoicing_impl.h
#
# src/corba/admin/libadmin_impl.a


# src/corba/whois/libwhois_impl.a
#
src_corba_whois_libwhois_impl_a_SOURCES = \
		src/corba/whois/whois_impl.cc \
		src/corba/whois/whois_impl.h
#
# src/corba/whois/libwhois_impl.a


# src/corba/whois/libwhois2_impl.a
#
src_corba_whois_libwhois2_impl_a_SOURCES = \
		src/corba/whois/whois2_impl.cc \
		src/corba/whois/whois2_impl.h \
		src/whois/nameserver_exists.h \
		src/whois/nameserver_exists.cc
#
# src/corba/whois/libwhois2_impl.a


# src/corba/epp/libepp_impl.a
#
src_corba_epp_libepp_impl_a_SOURCES = \
		src/fredlib/contact_verification/cancel_contact_verification.cc \
		src/fredlib/contact_verification/cancel_contact_verification.h \
		src/corba/epp/epp_session.h \
		src/corba/epp/epp_session.cc \
		src/corba/epp/epp_impl.cc \
		src/corba/epp/epp_impl.h \
		src/corba/epp/messages.cc \
		src/corba/epp/messages.h \
		src/corba/epp/notifier.cc \
		src/corba/epp/notifier.h \
		src/corba/epp/tech_check.cc \
		src/corba/epp/tech_check.h \
		src/corba/epp/countrycode.cc \
		src/corba/epp/countrycode.h \
		src/corba/epp/action.h \
		src/corba/epp/reason.h \
		src/corba/epp/response.h \
		src/corba/epp/notifier_changes.cc \
		src/corba/epp/notifier_changes.h
#
# src/corba/epp/libepp_impl.a


# src/corba/log/liblog_impl.a
#
src_corba_log_liblog_impl_a_SOURCES = \
		src/corba/log/log_impl_wrap.h \
		src/corba/log/log_impl_wrap.cc
#
# src/corba/log/liblog_impl.a



# src/corba/fred-rifd
#
src_corba_fred_rifd_SOURCES = \
		src/corba/fred-rifd.cc

src_corba_fred_rifd_LDADD = \
		src/corba/epp/libepp_impl.a \
		src/admin/contact/verification/libadmin_contact_verification.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/libfred.a \
		src/fredlib/messages/libmessages.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		util/log/liblog.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		-lmpdec -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/fred-rifd


# src/corba/fred-adifd
#
src_corba_fred_adifd_SOURCES = \
		src/mojeid/mojeid_validators.cc \
		src/mojeid/mojeid_validators.h \
		src/mojeid/mojeid_checkers.cc \
		src/mojeid/mojeid_checkers.h \
		src/mojeid/mojeid_contact_transfer_request_impl.cc \
		src/mojeid/public_request_verification_impl.cc \
		src/corba/admin_contact_verification/server_i.h \
		src/corba/admin_contact_verification/server_i.cc \
		src/fredlib/contact/util.h \
		src/fredlib/contact/util.cc \
		src/corba/fred-adifd.cc \
		src/admin_block/administrativeblocking.cc \
		src/admin_block/administrativeblocking.h \
		src/fredlib/domain/create_domain_name_blacklist.cc \
		src/fredlib/domain/create_domain_name_blacklist.h \
		src/fredlib/domain/create_domain_name_blacklist_id.cc \
		src/fredlib/domain/create_domain_name_blacklist_id.h \
		src/fredlib/domain/update_domain.cc \
		src/fredlib/domain/update_domain.h \
		src/fredlib/contact/copy_contact.cc \
		src/fredlib/contact/copy_contact.h \
		src/fredlib/contact/info_contact_data.h \
		src/fredlib/contact/info_contact_data.cc \
		src/fredlib/contact/info_contact.h \
		src/fredlib/contact/info_contact.cc \
		src/fredlib/contact/create_contact.h \
		src/fredlib/contact/create_contact.cc \
		src/fredlib/contact/place_address.cc \
		src/fredlib/contact/place_address.h \
		src/fredlib/opcontext.cc \
		src/fredlib/opcontext.h \
		src/corba/nameservice.cc \
		src/corba/mailer_manager.cc \
		src/corba/admin_block/corba_conversion.h \
		src/corba/admin_block/server_i.cc \
		src/corba/admin_block/server_i.h \
		src/admin/contact/verification/create_test_impl_prototypes.h \
		src/admin/contact/verification/create_test_impl_prototypes.cc \
		src/admin/contact/verification/delete_domains_of_invalid_contact.h \
		src/admin/contact/verification/delete_domains_of_invalid_contact.cc \
		src/admin/contact/verification/enqueue_check.h \
		src/admin/contact/verification/enqueue_check.cc \
		src/admin/contact/verification/exceptions.h \
		src/admin/contact/verification/fill_check_queue.h \
		src/admin/contact/verification/fill_check_queue.cc \
		src/admin/contact/verification/list_active_checks.h \
		src/admin/contact/verification/list_active_checks.cc \
		src/admin/contact/verification/related_records.h \
		src/admin/contact/verification/related_records.cc \
		src/admin/contact/verification/resolve_check.h \
		src/admin/contact/verification/resolve_check.cc \
		src/admin/contact/verification/run_all_enqueued_checks.h \
		src/admin/contact/verification/run_all_enqueued_checks.cc \
		src/admin/contact/verification/update_tests.h \
		src/admin/contact/verification/update_tests.cc \
		src/admin/contact/verification/contact_states/enum.h \
		src/admin/contact/verification/contact_states/delete_all.h \
		src/admin/contact/verification/contact_states/delete_all.cc \
		src/admin/contact/verification/test_impl/test_contactability.h \
		src/admin/contact/verification/test_impl/test_contactability.cc \
		src/admin/contact/verification/test_impl/test_cz_address_exists.h \
		src/admin/contact/verification/test_impl/test_cz_address_exists.cc \
		src/admin/contact/verification/test_impl/test_email_exists.h \
		src/admin/contact/verification/test_impl/test_email_exists.cc \
		src/admin/contact/verification/test_impl/test_email_syntax.h \
		src/admin/contact/verification/test_impl/test_email_syntax.cc \
		src/admin/contact/verification/test_impl/test_interface.h \
		src/admin/contact/verification/test_impl/test_interface.cc \
		src/admin/contact/verification/test_impl/test_name_syntax.h \
		src/admin/contact/verification/test_impl/test_name_syntax.cc \
		src/admin/contact/verification/test_impl/test_phone_syntax.h \
		src/admin/contact/verification/test_impl/test_phone_syntax.cc \
		src/admin/contact/verification/test_impl/test_send_letter.h \
		src/admin/contact/verification/test_impl/test_send_letter.cc

src_corba_fred_adifd_LDADD = \
		src/corba/admin/libadmin_impl.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbazone.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorbanullabletypes.a \
		src/corba/util/libcorbautil.a \
		src/corba/libcorba.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbaadmincontactverification.a \
		src/corba/libfilemanager.a \
		src/corba/libcorbaadministrativeblocking.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/banking/libbanking.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		util/log/liblog.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		-lmpdec -lm \
		-lresolv \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/fred-adifd


# src/corba/fred-pifd
#
src_corba_fred_pifd_SOURCES = \
		src/mojeid/mojeid_validators.cc \
		src/mojeid/mojeid_validators.h \
		src/mojeid/mojeid_checkers.cc \
		src/mojeid/mojeid_checkers.h \
		src/mojeid/mojeid_contact_transfer_request_impl.cc \
		src/mojeid/public_request_verification_impl.cc \
		src/corba/contact_verification/contact_verification_i.cc \
		src/corba/contact_verification/contact_verification_i.h \
		src/contact_verification/contact_verification_impl.cc \
		src/contact_verification/contact_verification_impl.h \
		src/contact_verification/public_request_contact_verification_wrapper.h \
		src/domain_browser/domain_browser.cc \
		src/domain_browser/domain_browser.h \
		src/corba/domain_browser/domain_browser_i.cc \
		src/corba/domain_browser/domain_browser_i.h \
		src/corba/fred-pifd.cc

src_corba_fred_pifd_LDADD = \
		src/corba/whois/libwhois_impl.a \
		src/corba/whois/libwhois2_impl.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbawhois2.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbazone.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/corba/libcorbaepp.a \
		src/corba/libfilemanager.a \
		src/corba/libcorbadomainbrowser.a \
		src/corba/libcorbalogger.a \
		src/corba/util/libcorbautil.a \
		src/corba/libcorbanullabletypes.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/corba/libcorbacontactverification.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/banking/libbanking.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		src/old_utils/liboldutils.a \
		util/log/liblog.a \
		util/libutil.a \
		-lmpdec -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/fred-pifd


# src/corba/fred-logd
#
src_corba_fred_logd_SOURCES = \
		src/corba/fred-logd.cc \
		src/corba/admin/common.cc \
		src/corba/admin/filter_impl.cc \
		src/corba/admin/pagetable_impl.cc \
		src/corba/admin/pagetable_logger.cc

src_corba_fred_logd_LDADD = \
		src/corba/log/liblog_impl.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/fredlib/libfred.a \
		src/fredlib/requests/librequests.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		util/log/liblog.a \
		util/libutil.a \
		src/old_utils/liboldutils.a \
		-lmpdec -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/fred-logd


# src/corba/messages/fred-msgd
#
src_corba_messages_fred_msgd_SOURCES = \
		src/corba/messages/fred-msgd.cc \
		src/corba/messages/messages_filemanager.cc \
		src/corba/messages/messages_filemanager.h \
		src/corba/nameservice.cc \
		src/corba/messages/messages.cc \
		src/corba/messages/messages.h \
		src/corba/file_manager_client.cc

nodist_src_corba_messages_fred_msgd_SOURCES = \
		src/corba/MessagesSK.cc \
		src/corba/FileManagerSK.cc

src_corba_messages_fred_msgd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_corba_messages_fred_msgd_LDADD = \
		src/fredlib/messages/libmessages.a \
		src/model/libmodel.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		src/corba/libcorbadatatypes.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/messages/fred-msgd


# src/corba/mojeid/fred-mifd
#
src_corba_mojeid_fred_mifd_SOURCES = \
		src/mojeid/mojeid_validators.cc \
		src/mojeid/mojeid_validators.h \
		src/mojeid/mojeid_checkers.cc \
		src/mojeid/mojeid_checkers.h \
		src/mojeid/request.h \
		src/mojeid/mojeid_contact_transfer_request_impl.h \
		src/mojeid/mojeid_contact_transfer_request_impl.cc \
		src/mojeid/public_request_verification_impl.cc \
		src/mojeid/public_request_verification_impl.h \
		src/mojeid/mojeid_notifier.cc \
		src/mojeid/mojeid_notifier.h \
		src/mojeid/mojeid_contact_states.h \
		src/mojeid/mojeid_disclose_policy.h \
		src/mojeid/mojeid_identification.h \
		src/mojeid/mojeid.cc \
		src/mojeid/mojeid.h \
		src/corba/nameservice.cc \
		src/corba/mailer_manager.cc \
		src/corba/corba_auto_garbaged_list.h \
		src/corba/mojeid/corba_conversion.h \
		src/corba/mojeid/server_i.cc \
		src/corba/mojeid/server_i.h \
		src/corba/mojeid/fred-mifd.cc

nodist_src_corba_mojeid_fred_mifd_SOURCES = \
		src/corba/MojeIDSK.cc \
		src/corba/MailerSK.cc

src_corba_mojeid_fred_mifd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_corba_mojeid_fred_mifd_LDADD = \
		src/corba/libcorbadatatypes.a \
		src/fredlib/notifier/libnotifier.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/messages/libmessages.a \
		util/libutil.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		src/old_utils/liboldutils.a \
		-lmpdec -lm \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/mojeid/fred-mifd

#
# src/corba/domain_browser/fred-dbifd
src_corba_domain_browser_fred_dbifd_SOURCES = \
		util/cfg/handle_domainbrowser_args.h \
		src/domain_browser/domain_browser.cc \
		src/domain_browser/domain_browser.h \
		src/corba/nameservice.cc \
		src/corba/domain_browser/domain_browser_i.cc \
		src/corba/domain_browser/domain_browser_i.h \
		src/corba/domain_browser/fred-dbifd.cc

src_corba_domain_browser_fred_dbifd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_corba_domain_browser_fred_dbifd_LDADD = \
		src/corba/libcorbadomainbrowser.a \
		src/fredlib/libfred.a \
		util/libutil.a \
		util/log/liblog.a \
		util/db/libdb.a \
		-lmpdec -lm \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/corba/domain_browser/fred-dbifd


# src/cli_admin/fred-admin
#
src_cli_admin_fred_admin_SOURCES = \
		src/cli_admin/fred-admin.cc \
		src/cli_admin/handle_adminclientselection_args.h \
		src/cli_admin/baseclient.h \
		src/cli_admin/commonclient.cc \
		src/cli_admin/commonclient.h \
		src/cli_admin/domainclient.cc \
		src/cli_admin/domainclient.h \
		src/cli_admin/domain_params.h \
		src/cli_admin/domain_client_impl.h \
		src/cli_admin/keysetclient.cc \
		src/cli_admin/keysetclient.h \
		src/cli_admin/keyset_params.h \
		src/cli_admin/keyset_client_impl.h \
		src/cli_admin/contactclient.cc \
		src/cli_admin/contactclient.h \
		src/cli_admin/contact_params.h \
		src/cli_admin/contact_client_impl.h \
		src/cli_admin/invoiceclient.cc \
		src/cli_admin/invoiceclient.h \
		src/cli_admin/invoice_params.h \
		src/cli_admin/invoice_client_impl.h \
		src/cli_admin/bankclient.cc \
		src/cli_admin/bankclient.h \
		src/cli_admin/bank_params.h \
		src/cli_admin/bank_client_impl.h \
		src/cli_admin/pollclient.cc \
		src/cli_admin/pollclient.h \
		src/cli_admin/poll_params.h \
		src/cli_admin/poll_client_impl.h \
		src/cli_admin/registrarclient.cc \
		src/cli_admin/registrarclient.h \
		src/cli_admin/registrar_params.h \
		src/cli_admin/registrar_client_impl.h \
		src/cli_admin/notifyclient.cc \
		src/cli_admin/notifyclient.h \
		src/cli_admin/notify_params.h \
		src/cli_admin/notify_client_impl.h \
		src/cli_admin/enumparamclient.cc \
		src/cli_admin/enumparamclient.h \
		src/cli_admin/enumparam_params.h \
		src/cli_admin/enumparam_client_impl.h \
		src/cli_admin/objectclient.h \
		src/cli_admin/objectclient.cc \
		src/cli_admin/object_params.h \
		src/cli_admin/object_client_impl.h \
		src/cli_admin/fileclient.h \
		src/cli_admin/fileclient.cc \
		src/cli_admin/file_params.h \
		src/cli_admin/file_client_impl.h \
		src/cli_admin/regblock_params.h \
		src/cli_admin/regblock_client.h \
		src/cli_admin/regblock_client_impl.h \
		src/cli_admin/charge_params.h \
		src/cli_admin/chargeclient.cc \
		src/cli_admin/chargeclient.h \
		src/cli_admin/charge_client_impl.h \
		src/admin/contact/merge_contact.cc \
		src/admin/contact/merge_contact.h \
		src/admin/contact/merge_contact_logger.cc \
		src/admin/contact/merge_contact_logger.h \
		src/admin/contact/merge_contact_auto_procedure.cc \
		src/admin/contact/merge_contact_auto_procedure.h \
		src/admin/contact/merge_contact_reporting.cc \
		src/admin/contact/merge_contact_reporting.h \
		src/cli_admin/domain_name_validation_params.h \
		src/cli_admin/domain_name_validation_init.h \
		src/fredlib/object_state/create_object_state_request.cc \
		src/fredlib/object_state/create_object_state_request.h \
		src/fredlib/object_state/create_object_state_request_id.cc \
		src/fredlib/object_state/create_object_state_request_id.h \
		src/fredlib/object_state/get_object_state_id_map.cc \
		src/fredlib/object_state/get_object_state_id_map.h \
		src/fredlib/object_state/cancel_object_state_request.cc \
		src/fredlib/object_state/cancel_object_state_request.h \
		src/fredlib/object_state/cancel_object_state_request_id.cc \
		src/fredlib/object_state/cancel_object_state_request_id.h \
		util/optys/upload_client.cc \
		util/optys/upload_client.h

src_cli_admin_fred_admin_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(XML2_LIBS)	# using xpath in contact verification test for czech address

src_cli_admin_fred_admin_LDADD = \
		src/admin/contact/verification/libadmin_contact_verification.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		src/fredlib/banking/libbanking.a \
		src/corba/libcorba.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libmailer.a \
		src/corba/libfilemanager.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/requests/librequests.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		util/hp/libhp.a \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(OMNIORB4_LIBS) \
		$(XML2_LIBS) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		-lresolv
#
# src/cli_admin/fred-admin


# tools/logd_migration
#
tools_logd_migration_migrate_log_SOURCES = \
		tools/logd_migration/migrate.cc \
		tools/logd_migration/log_if.cc \
		tools/logd_migration/m_epp_parser.cc \
		tools/logd_migration/migrate.h \
		tools/logd_migration/m_epp_parser.h

tools_logd_migration_migrate_log_CPPFLAGS = \
		$(AM_CPPFLAGS) \
		$(LIBIDN_CFLAGS)

tools_logd_migration_migrate_log_LDFLAGS = \
		$(AM_LDFLAGS)

tools_logd_migration_migrate_log_LDADD = \
		src/fredlib/requests/librequests.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		src/fredlib/banking/libbanking.a \
		src/corba/log/liblog_impl.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		util/libutil.a \
		src/old_utils/liboldutils.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(XML2_LIBS) \
		$(LIBIDN_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# tools/logd_migration


# tests
#
SERVER_TESTS = \
		tests/test-server \
		tests/interfaces/test-admin-contact-verification \
		tests/test-exception \
		tests/test-blocking \
		tests/test-fredlib


# tests/interfaces/test-admin-contact-verification
#
tests_interfaces_test_admin_contact_verification_SOURCES = \
		tests/interfaces/admin/contact/verification/main.cc \
		tests/interfaces/admin/contact/verification/setup_utils.h \
		tests/fredlib/contact/verification/setup_utils.h \
		tests/fredlib/contact/verification/setup_utils.cc \
		tests/interfaces/admin/contact/verification/test_run_all_enqueued_checks.cc \
		tests/interfaces/admin/contact/verification/test_fill_check_queue.cc \
		tests/interfaces/admin/contact/verification/test_resolve_check.cc \
		tests/interfaces/admin/contact/verification/test_related_records.cc \
		tests/interfaces/admin/contact/verification/test_enqueue_check.cc \
		tests/interfaces/admin/contact/verification/test_delete_domains_of_invalid_contact.cc \
		tests/setup/fixtures_utils.h \
		tests/setup/fixtures_utils.cc \
		tests/setup/fixtures.h \
		tests/setup/fixtures.cc

tests_interfaces_test_admin_contact_verification_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_interfaces_test_admin_contact_verification_LDADD = \
		src/admin/contact/verification/libadmin_contact_verification.a \
		src/fredlib/libfred.a \
		util/log/liblog.a \
		util/libutil.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# tests/interfaces/test-admin-contact-verification


# tests/test_server
#
tests_test_server_SOURCES = \
		tests/deprecated/test_server.cc \
		tests/deprecated/test_server.h \
		tests/deprecated/test_conv.cc \
		tests/deprecated/test_conv.h \
		tests/deprecated/test_file_manager_client.cc \
		tests/deprecated/test_file_manager_client.h \
		tests/deprecated/test_db.cc \
		tests/deprecated/test_model.cc \
		tests/deprecated/test_model.h \
		tests/deprecated/test_registrar_group.cc \
		tests/deprecated/test_registrar_group.h \
		tests/deprecated/test_registrar_blocking.cc \
		src/corba/admin/filter_impl.cc \
		src/corba/admin/filter_impl.h \
		tests/deprecated/test_registrar_certification.cc \
		tests/deprecated/test_registrar_certification.h \
		tests/deprecated/test_logd.cc \
		tests/deprecated/test_msgd.cc \
		tests/deprecated/test_msgd.h \
		tests/deprecated/test_admin.cc \
		tests/deprecated/test_corbans.cc \
		tests/deprecated/test_whois.cc \
		tests/deprecated/test_cache.cc \
		tests/setup/tests_common.cc \
		tests/setup/tests_common.h \
		tests/deprecated/test_filters_date_time.cc \
		tests/deprecated/test_invoice_common.h \
		tests/deprecated/test_invoice_common.cc \
		tests/deprecated/test_invoice.cc \
		tests/deprecated/test_invoice_account.cc \
		tests/setup/test_common_threaded.h \
		tests/deprecated/test_common_registry.h \
		tests/deprecated/test_common_registry.cc \
		tests/deprecated/test_invoice2.cc \
		tests/util/test_decimal.cc \
		tests/util/test_shellcmd.cc \
		tests/deprecated/test_contact_verification.cc \
		tests/deprecated/test_locking_trigger.cc \
		tests/deprecated/test_mojeid.cc \
		tests/util/test_nullable.cc \
		tests/util/test_optional.cc \
		src/corba/epp_corba_client_impl.cc \
		src/corba/epp_corba_client_impl.h \
		src/contact_verification/contact_verification_impl.cc \
		src/contact_verification/contact_verification_impl.h \
		src/fredlib/contact_verification/cancel_contact_verification.cc \
		src/fredlib/contact_verification/cancel_contact_verification.h \
		src/mojeid/mojeid_validators.cc \
		src/mojeid/mojeid_validators.h \
		src/mojeid/mojeid_checkers.cc \
		src/mojeid/mojeid_checkers.h \
		src/mojeid/request.h \
		src/mojeid/mojeid_contact_transfer_request_impl.h \
		src/mojeid/mojeid_contact_transfer_request_impl.cc \
		src/mojeid/public_request_verification_impl.cc \
		src/mojeid/public_request_verification_impl.h \
		src/mojeid/mojeid.cc \
		src/mojeid/mojeid.h \
		src/mojeid/mojeid_notifier.cc \
		src/mojeid/mojeid_notifier.h \
		tests/deprecated/checks.h \
		tests/util/test_printable.cc \
		tests/util/test_is_equal_optional_nullable.cc \
		src/fredlib/object_state/create_object_state_request.cc \
		src/fredlib/object_state/create_object_state_request.h \
		src/fredlib/object_state/create_object_state_request_id.cc \
		src/fredlib/object_state/create_object_state_request_id.h \
		src/fredlib/object_state/get_object_state_id_map.cc \
		src/fredlib/object_state/get_object_state_id_map.h \
		src/fredlib/object_state/cancel_object_state_request.cc \
		src/fredlib/object_state/cancel_object_state_request.h \
		src/fredlib/object_state/cancel_object_state_request_id.cc \
		src/fredlib/object_state/cancel_object_state_request_id.h \
		src/domain_browser/domain_browser.cc \
		src/domain_browser/domain_browser.h \
		tests/interfaces/domain_browser/test_domain_browser.cc \
		tests/setup/fixtures.h

tests_test_server_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_server_LDADD = \
		src/fredlib/notifier/libnotifier.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/epp/libepp_impl.a \
		src/corba/libcorba.a \
		src/corba/whois/libwhois_impl.a \
		src/fredlib/public_request/libpublic_request.a \
		src/admin/contact/verification/libadmin_contact_verification.a \
		src/fredlib/libfred.a \
		src/fredlib/messages/libmessages.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		src/old_utils/liboldutils.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/banking/libbanking.a \
		util/log/liblog.a \
		util/libutil.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamojeid.a \
		src/corba/libcorbazone.a \
		src/corba/libfilemanager.a \
		src/corba/libmailer.a \
		src/corba/libcorbazonegenerator.a \
		src/corba/libcorbaadministrativeblocking.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libmailer.a \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# tests/test_server


# tests/test_blocking
#
tests_test_blocking_SOURCES = \
		tests/deprecated/test_server.cc \
		tests/deprecated/test_server.h \
		src/corba/admin/filter_impl.cc \
		src/corba/admin/filter_impl.h \
		tests/setup/tests_common.cc \
		tests/setup/tests_common.h \
		src/corba/epp_corba_client_impl.cc \
		src/corba/epp_corba_client_impl.h \
		src/contact_verification/contact_verification_impl.cc \
		src/contact_verification/contact_verification_impl.h \
		src/fredlib/contact_verification/cancel_contact_verification.cc \
		src/fredlib/contact_verification/cancel_contact_verification.h \
		src/mojeid/mojeid_validators.cc \
		src/mojeid/mojeid_validators.h \
		src/mojeid/mojeid_checkers.cc \
		src/mojeid/mojeid_checkers.h \
		src/mojeid/request.h \
		src/mojeid/mojeid_contact_transfer_request_impl.h \
		src/mojeid/mojeid_contact_transfer_request_impl.cc \
		src/mojeid/public_request_verification_impl.cc \
		src/mojeid/public_request_verification_impl.h \
		src/mojeid/mojeid.cc \
		src/mojeid/mojeid.h \
		src/mojeid/mojeid_notifier.cc \
		src/mojeid/mojeid_notifier.h \
		src/fredlib/object_state/clear_admin_object_state_request_id.cc \
		src/fredlib/object_state/clear_object_state_request_id.cc \
		src/fredlib/object_state/create_object_state_request_id.cc \
		src/fredlib/object_state/create_object_state_request.cc \
		src/fredlib/object_state/create_admin_object_block_request_id.cc \
		src/fredlib/object_state/create_admin_object_state_restore_request_id.cc \
		src/fredlib/object_state/cancel_object_state_request_id.cc \
		src/fredlib/object_state/get_object_state_id_map.cc \
		src/fredlib/object_state/get_blocking_status_desc_list.cc \
		src/fredlib/domain/create_domain_name_blacklist_id.cc \
		tests/checks.h
		
tests_test_blocking_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_blocking_LDADD = \
		src/fredlib/notifier/libnotifier.a \
		src/fredlib/contact_verification/libcontact_verification.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/epp/libepp_impl.a \
		src/corba/libcorba.a \
		src/corba/whois/libwhois_impl.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/libfred.a \
		src/fredlib/messages/libmessages.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		src/old_utils/liboldutils.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/banking/libbanking.a \
		util/log/liblog.a \
		util/libutil.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamojeid.a \
		src/corba/libcorbazone.a \
		src/corba/libfilemanager.a \
		src/corba/libmailer.a \
		src/corba/libcorbazonegenerator.a \
		src/corba/libcorbaadministrativeblocking.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libmailer.a \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# tests/test_blocking

# tests/test_fredlib
#
tests_test_fredlib_SOURCES = \
		tests/fredlib/main.cc \
		tests/fredlib/registrar/test_info_registrar.cc \
		tests/fredlib/registrar/test_create_registrar.cc \
		tests/fredlib/contact/test_delete_contact.cc \
		tests/fredlib/contact/test_create_contact.cc \
		tests/fredlib/contact/test_info_contact.cc \
		tests/fredlib/contact/test_merge_contact.cc \
		tests/fredlib/contact/test_update_contact.cc \
		tests/fredlib/contact/test_copy_contact.cc \
		tests/fredlib/domain/test_update_domain.cc \
		tests/fredlib/domain/test_info_domain.cc \
		tests/fredlib/domain/test_delete_domain.cc \
		tests/fredlib/domain/test_create_domain.cc \
		tests/fredlib/domain/test_check_domain.cc \
		tests/fredlib/domain/test_domain_name.cc \
		tests/fredlib/domain/test_create_domain_name_blacklist_id.cc \
		tests/fredlib/keyset/test_delete_keyset.cc \
		tests/fredlib/keyset/test_info_keyset.cc \
		tests/fredlib/keyset/test_update_keyset.cc \
		tests/fredlib/nsset/test_update_nsset.cc \
		tests/fredlib/nsset/test_delete_nsset.cc \
		tests/fredlib/nsset/test_info_nsset.cc \
		tests/fredlib/contact/verification/setup_utils.h \
		tests/fredlib/contact/verification/setup_utils.cc \
		tests/fredlib/contact/verification/test_create_check.cc \
		tests/fredlib/contact/verification/test_update_check.cc \
		tests/fredlib/contact/verification/test_create_test.cc \
		tests/fredlib/contact/verification/test_update_test.cc \
		tests/fredlib/contact/verification/test_info_check.cc \
		tests/fredlib/contact/verification/test_list_checks.cc \
		tests/fredlib/contact/verification/test_list_enum_objects.cc \
		tests/fredlib/poll/test_create_delete_contact_poll_message.cc \
		tests/fredlib/poll/test_create_delete_domain_poll_message.cc \
		tests/fredlib/poll/test_create_update_object_poll_message.cc \
		tests/fredlib/object_state/test_object_state.cc \
		tests/fredlib/object_state/test_create_object_state_request_id.cc \
		tests/fredlib/object_state/test_create_admin_object_block_request_id.cc \
		tests/fredlib/object_state/test_create_admin_object_state_restore_request_id.cc \
		tests/fredlib/object_state/test_cancel_object_state_request_id.cc \
		tests/fredlib/object_state/test_clear_admin_object_state_request_id.cc \
		tests/fredlib/test_opexception.cc \
		tests/fredlib/test_check_handle.cc \
		tests/setup/fixtures_utils.h \
		tests/setup/fixtures_utils.cc \
		tests/setup/fixtures.h \
		tests/setup/fixtures.cc


tests_test_fredlib_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_fredlib_LDADD = \
		src/fredlib/libfred.a \
		util/libutil.a \
		util/log/liblog.a \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(POSTGRESQL_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# tests/test_fredlib


# non-corba tests with overloaded global operator new
#
tests_test_exception_SOURCES = \
		tests/util/test_exception.cc

tests_test_exception_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_exception_LDADD = \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(BOOST_SYSTEM_LIB)
#
# non-corba tests with overloaded global operator new




install-tests: $(SERVER_TESTS)
		test -z "$(bindir)" || $(MKDIR_P) "$(bindir)"
		$(INSTALL_PROGRAM) $(SERVER_TESTS) $(bindir)

testdatadir = $(docdir)

dist_testdata_DATA = \
		tests/deprecated/example_payments.xml \
		tests/deprecated/test_file.pdf

