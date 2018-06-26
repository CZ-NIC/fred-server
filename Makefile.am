ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
		$(POSTGRESQL_CFLAGS) \
		${BOOST_CPPFLAGS} \
		$(AM_CFLAGS) \
		$(LIBIDN_CFLAGS) \
		$(OMNIORB4_CFLAGS) \
		$(XML2_CFLAGS) \
		-I $(top_srcdir) \
		-I $(top_builddir) \
		-D __STDC_LIMIT_MACROS

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
		src/bin/corba/MessagesSK.cc \
		src/bin/corba/MessagesDynSK.cc \
		src/bin/corba/Messages.hh \
		src/bin/corba/MojeIDSK.cc \
		src/bin/corba/MojeIDDynSK.cc \
		src/bin/corba/MojeID.hh \
		src/bin/corba/AdministrativeBlockingSK.cc \
		src/bin/corba/AdministrativeBlockingDynSK.cc \
		src/bin/corba/AdministrativeBlockingSK.h \
		src/bin/corba/AdministrativeBlocking.hh \
		src/bin/corba/FileManagerSK.cc \
		src/bin/corba/FileManagerDynSK.cc \
		src/bin/corba/FileManager.hh \
		src/bin/corba/MailerSK.cc \
		src/bin/corba/MailerDynSK.cc \
		src/bin/corba/Mailer.hh \
		src/bin/corba/ZoneSK.cc \
		src/bin/corba/ZoneDynSK.cc \
		src/bin/corba/Zone.hh \
		src/bin/corba/_dataTypesSK.cc \
		src/bin/corba/_dataTypesDynSK.cc \
		src/bin/corba/_dataTypes.hh \
		src/bin/corba/DateTimeSK.cc \
		src/bin/corba/DateTimeDynSK.cc \
		src/bin/corba/DateTime.hh \
		src/bin/corba/NullableTypesSK.cc \
		src/bin/corba/NullableTypesDynSK.cc \
		src/bin/corba/NullableTypes.hh \
		src/bin/corba/FiltersSK.cc \
		src/bin/corba/FiltersDynSK.cc \
		src/bin/corba/Filters.hh \
		src/bin/corba/RegistrySK.cc \
		src/bin/corba/RegistryDynSK.cc \
		src/bin/corba/Registry.hh \
		src/bin/corba/WhoisSK.cc \
		src/bin/corba/WhoisDynSK.cc \
		src/bin/corba/Whois.hh \
		src/bin/corba/Whois2SK.cc \
		src/bin/corba/Whois2DynSK.cc \
		src/bin/corba/Whois2.hh \
		src/bin/corba/AdminSK.cc \
		src/bin/corba/AdminDynSK.cc \
		src/bin/corba/Admin.hh \
		src/bin/corba/LoggerSK.cc \
		src/bin/corba/LoggerDynSK.cc \
		src/bin/corba/Logger.hh \
		src/bin/corba/TechCheckSK.cc \
		src/bin/corba/TechCheckDynSK.cc \
		src/bin/corba/TechCheck.hh \
		src/bin/corba/EPPSK.cc \
		src/bin/corba/EPPDynSK.cc \
		src/bin/corba/EPP.hh \
		src/bin/corba/ZoneGeneratorSK.cc \
		src/bin/corba/ZoneGeneratorDynSK.cc \
		src/bin/corba/ZoneGenerator.hh \
		src/bin/corba/ContactVerificationSK.cc \
		src/bin/corba/ContactVerificationDynSK.cc \
		src/bin/corba/ContactVerification.hh \
		src/bin/corba/DomainBrowserSK.cc \
		src/bin/corba/DomainBrowserDynSK.cc \
		src/bin/corba/DomainBrowser.hh \
		src/bin/corba/PublicRequestSK.cc \
		src/bin/corba/PublicRequestDynSK.cc \
		src/bin/corba/PublicRequest.hh \
		src/bin/corba/AdminContactVerificationSK.cc \
		src/bin/corba/AdminContactVerificationDynSK.cc \
		src/bin/corba/AdminContactVerificationSK.h \
		src/bin/corba/AdminContactVerification.hh \
		src/bin/corba/corba_conversion_testSK.cc \
		src/bin/corba/corba_conversion_testDynSK.cc \
		src/bin/corba/corba_conversion_testSK.h \
		src/bin/corba/corba_conversion_test.hh \
		src/bin/corba/NotificationSK.cc \
		src/bin/corba/NotificationDynSK.cc \
		src/bin/corba/NotificationSK.h \
		src/bin/corba/Notification.hh \
		src/bin/corba/AutomaticKeysetManagementSK.cc \
		src/bin/corba/AutomaticKeysetManagementDynSK.cc \
		src/bin/corba/AutomaticKeysetManagementSK.h \
		src/bin/corba/AutomaticKeysetManagement.hh \
		src/bin/corba/RecordStatementSK.cc \
		src/bin/corba/RecordStatementDynSK.cc \
		src/bin/corba/RecordStatementSK.h \
		src/bin/corba/RecordStatement.hh \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/BufferDynSK.cc \
		src/bin/corba/BufferSK.h \
		src/bin/corba/Buffer.hh \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/IsoDateDynSK.cc \
		src/bin/corba/IsoDateSK.h \
		src/bin/corba/IsoDate.hh \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/NullableIsoDateDynSK.cc \
		src/bin/corba/NullableIsoDateSK.h \
		src/bin/corba/NullableIsoDate.hh \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTimeSK.h \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/NullableIsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeDynSK.cc \
		src/bin/corba/NullableIsoDateTimeSK.h \
		src/bin/corba/NullableIsoDateTime.hh



fredconfdir = $(sysconfdir)/fred
rcconfdir = $(sysconfdir)/init.d


EXTRA_DIST = \
		contrib/fedora/fred-server.spec \
		src/util/sms_send.bash \
		test/deprecated/test_file.pdf

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
		src/util/util.hh \
		src/util/singleton.hh \
		src/util/time_clock.hh \
		src/util/settings.hh \
		src/util/file_lock.hh \
		src/util/base_exception.hh \
		src/util/random.hh \
		src/util/concurrent_queue.hh \
		src/util/concurrent_set.hh \
		src/util/random_data_generator.hh \
		src/util/corba_wrapper.hh \
		src/util/subprocess.hh \
		src/util/string_split.hh \
		src/util/corba_wrapper_decl.hh \
		src/util/map_at.hh \
		src/util/setup_server.hh \
		src/util/setup_server_decl.hh \
		src/util/cfg/config_handler.hh \
		src/util/cfg/faked_args.hh \
		src/util/cfg/handle_args.hh \
		src/util/cfg/handle_corbanameservice_args.hh \
		src/util/decimal/decimal.hh \
		src/util/cfg/handle_database_args.hh \
		src/util/cfg/handle_general_args.hh \
		src/util/cfg/handle_server_args.hh \
		src/util/cfg/handle_threadgroup_args.hh \
		src/util/cfg/test_custom_main.hh \
		src/util/cfg/handle_logging_args.hh \
		src/util/cfg/handle_registry_args.hh \
		src/util/cfg/config_handler_decl.hh \
		src/util/cfg/handle_mojeid_args.hh \
		src/util/cfg/handle_administrativeblocking_args.hh \
		src/util/cfg/handle_adifd_args.hh \
		src/util/cfg/handle_logd_args.hh \
		src/util/cfg/handle_rifd_args.hh \
		src/util/cfg/command_selection_args.hh \
		src/util/cfg/check_args.hh \
		src/util/cfg/checked_types.hh \
		src/util/cfg/validate_args.hh \
		src/util/cfg/handle_sms_args.hh \
		src/util/cfg/handle_contactverification_args.hh \
		src/util/cfg/handle_createexpireddomain_args.hh \
		src/util/cfg/handle_tests_args.hh \
		src/util/optys/handle_optys_mail_args.hh \
		src/util/cfg/handle_akmd_args.hh \
		src/util/types/optional.hh \
		src/util/types/optional_from_program_options.hh \
		src/util/types/money.hh \
		src/util/xmlgen.hh \
		test/util/exception_test.hh \
		src/util/factory.hh \
		src/util/factory_check.hh \
		src/util/map_get.hh \
		src/util/uuid.hh \
		src/bin/corba/common_wrappers.hh \
		src/bin/corba/contact_verification/corba_conversion.hh \
		src/util/optional_value.hh \
		src/util/is_equal_optional_nullable.hh \
		src/util/output_indenter.hh \
		src/bin/corba/util/corba_conversions_buffer.hh \
		src/bin/corba/util/corba_conversions_nullable_types.hh \
		src/bin/corba/util/corba_conversions_int.hh \
		src/bin/corba/util/corba_conversions_string.hh \
		src/bin/corba/util/corba_conversions_isodate.hh \
		src/bin/corba/util/corba_conversions_isodatetime.hh \
		src/bin/corba/automatic_keyset_management/corba_conversions.cc \
		src/bin/corba/automatic_keyset_management/corba_conversions.hh \
		src/util/is_equal_optional_nullable.hh \
		test/libfred/util.hh \
		test/libfred/enum_to_db_handle_conversion.hh \
		test/mockup/logger_client_dummy.hh \
		src/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh \
		src/libfred/registrable_object/contact/verification/enum_check_status.hh \
		src/libfred/registrable_object/contact/verification/enum_test_status.hh \
		src/libfred/registrable_object/contact/verification/exceptions.hh \
		src/backend/admin/contact/verification/test_impl/test_interface.hh \
		src/backend/admin/contact/verification/exceptions.hh \
		src/libfred/object/registry_object_type.hh \
		src/libfred/notifier/enqueue_notification.hh \
		src/libfred/notifier/event_on_object_enum.hh \
		src/libfred/notifier/exception.hh \
		src/libfred/notifier/process_one_notification_request.hh \
		src/libfred/notifier/gather_email_data/gather_email_addresses.hh \
		src/libfred/notifier/gather_email_data/gather_email_content.hh \
		src/libfred/notifier/gather_email_data/notification_request.hh \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/contact.hh \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/domain.hh \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.hh \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/nsset.hh \
		src/libfred/notifier/util/add_old_new_suffix_pair.hh \
		src/libfred/notifier/util/bool_to_string.hh \
		src/libfred/notifier/util/boost_date_to_cz_string.hh \
		src/libfred/notifier/util/get_previous_object_historyid.hh \
		src/libfred/notifier/util/string_list_utils.hh \
		test/libfred/notifier/fixture_data.hh \
		test/libfred/notifier/util.hh \
		src/util/enum_conversion.hh \
		src/libfred/registrable_object/contact/handle_state.hh \
		src/libfred/registrable_object/contact/ssntype.hh \
		src/libfred/exception.hh \
		test/util/corba_conversion_test.idl \
		src/libfred/object/object_type.hh \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/util.hh \
		src/backend/epp/contact/contact_change.hh \
		src/backend/epp/exception.hh \
		src/backend/epp/session_lang.hh \
		src/backend/epp/get_registrar_session_data.hh \
		src/bin/corba/epp/epp_legacy_compatibility.hh \
		src/backend/epp/registrar_session_data.hh \
		src/backend/epp/request_params.hh \
		src/backend/epp/epp_response_failure.hh \
		src/backend/epp/epp_response_success.hh \
		src/backend/epp/epp_result_failure.hh \
		src/backend/epp/epp_result_success.hh \
		src/backend/epp/epp_result_code.hh \
		src/backend/epp/impl/action.hh \
		src/backend/epp/impl/util.hh \
		src/libfred/object/object_states_info.hh \
		src/libfred/object/object_state.hh \
		src/backend/epp/contact/contact_handle_state_to_check_result.hh \
		src/libfred/public_request/public_request_status.hh \
		src/libfred/public_request/get_opened_public_request.hh \
		src/libfred/public_request/info_public_request.hh \
		src/libfred/public_request/info_public_request_auth.hh \
		test/backend/epp/util.hh \
		test/backend/epp/fixture.hh \
		test/backend/epp/contact/fixture.hh \
		test/backend/epp/domain/fixture.hh \
		test/backend/epp/poll/fixture.hh \
		src/bin/corba/mojeid/mojeid_corba_conversion.hh \
		src/backend/mojeid/mojeid_impl_data.hh \
		src/backend/mojeid/mojeid_public_request.hh \
		src/backend/mojeid/mojeid.hh \
		src/backend/mojeid/mojeid_checkers.hh \
		src/backend/mojeid/mojeid_impl_data_conversion.hh \
		src/backend/mojeid/mojeid_impl_internal.hh \
		src/backend/mojeid/safe_data_storage.hh \
		src/backend/mojeid/messages/generate.hh \
		src/bin/corba/mojeid/server_i.hh \
		src/bin/corba/mojeid/service_name.hh \
		src/libfred/registrable_object/nsset/handle_state.hh \
		src/libfred/registrable_object/keyset/handle_state.hh \
		src/backend/epp/nsset/dns_host_output.hh \
		src/backend/epp/nsset/dns_host_input.hh \
		src/backend/epp/nsset/nsset_handle_state_to_check_result.hh \
		src/backend/epp/nsset/impl/limits.hh \
		test/backend/epp/nsset/fixture.hh \
		src/backend/public_request/process_public_request_authinfo.hh

noinst_LIBRARIES = \
		src/util/libutil.a \
		src/util/libpassword_storage.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		src/deprecated/util/liboldutils.a \
		src/deprecated/model/libmodel.a \
		src/libfred/libfred.a \
		src/libfred/messages/libmessages.a \
		src/libfred/requests/librequests.a \
		src/libfred/banking/libbanking.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbamessages.a \
		src/bin/corba/libcorbamojeid.a \
		src/bin/corba/libcorbazone.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/admin/libadmin_impl.a \
		src/bin/corba/whois/libwhois_impl.a \
		src/bin/corba/whois/libwhois2_impl.a \
		src/bin/corba/epp/libepp_impl.a \
		src/bin/corba/log/liblog_impl.a \
		src/util/hp/libhp.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbanullabletypes.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbawhois.a \
		src/bin/corba/libcorbawhois2.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbanotification.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbatechcheck.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbazonegenerator.a \
		src/bin/corba/libcorbaadministrativeblocking.a \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/contact_verification/libcontact_verification.a \
		src/bin/corba/libcorbacontactverification.a \
		src/bin/corba/libcorbadomainbrowser.a \
		src/bin/corba/libcorbapublicrequest.a \
		src/bin/corba/libcorbaautomatickeysetmanagement.a \
		src/bin/corba/libcorbarecordstatement.a \
		src/bin/corba/libcorbaadmincontactverification.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorba_conversion_test.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbanullableisodatetime.a \
		src/backend/admin/contact/verification/libadmin_contact_verification.a \
		src/backend/admin/notification/libadmin_notification.a \
		src/backend/admin/domain/libadmin_create_expired_domain.a \
		src/epp/impl/registraracl/libauthentic_registrar.a


noinst_PROGRAMS = \
		src/util/log/test-log \
		src/util/db/test-db \
		src/util/db/test-mockdb \
		src/util/hp/crc32 \
		src/deprecated/model/test-model \
		$(SERVER_TESTS)


sbin_PROGRAMS = \
		src/bin/corba/fred-rifd \
		src/bin/corba/fred-adifd \
		src/bin/corba/fred-pifd \
		src/bin/corba/fred-logd \
		src/bin/corba/messages/fred-msgd \
		src/bin/corba/mojeid/fred-mifd \
		src/bin/corba/mojeid/messages/fred-mojeid-msggen \
		src/bin/corba/domain_browser/fred-dbifd \
		src/bin/corba/automatic_keyset_management/fred-akmd \
		src/bin/corba/record_statement/fred-rsifd \
		src/bin/cli/fred-admin \
		tools/logd_migration/migrate_log \
		tools/contact_migration/mojeid_contact_address_import \
		tools/contact_migration/mojeid_contact_ssn_fix \
		tools/disclose_flags_updater/fred-disclose-flags-updater


# util/libutil.a
#
src_util_libutil_a_SOURCES = \
		src/util/random.cc \
		src/util/util.cc \
		src/util/output_indenter.cc \
		src/util/printable.hh \
		src/util/printable.cc \
		src/util/csv_parser.hh \
		src/util/csv_parser.cc \
		src/util/xmlgen.cc \
		src/util/subprocess.cc \
		src/util/idn_utils.hh \
		src/util/idn_utils.cc \
		src/util/nonconvertible.hh
#
# util/libutil.a


# util/log/liblog.a
#
src_util_log_liblog_a_SOURCES = \
		src/util/log/logger.cc \
		src/util/log/log.cc \
		src/util/log/log_types.cc \
		src/util/log/context.cc \
		src/util/log/log.hh \
		src/util/log/log_types.hh \
		src/util/log/logger.hh \
		src/util/log/context.hh
#
# util/log/liblog.a


# util/log/test-log
#
src_util_log_test_log_SOURCES = \
		src/util/log/test-log.cc \
		$(UTIL_LOG_SRC)

src_util_log_test_log_CPPFLAGS = \
		$(AM_CPPFLAGS)

src_util_log_test_log_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_util_log_test_log_LDADD = \
		src/util/log/liblog.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# util/log/test-log


# util/types/libtypes.a
#
src_util_types_libtypes_a_SOURCES = \
		src/util/types/date.cc \
		src/util/types/date.hh \
		src/util/types/date_interval.cc \
		src/util/types/date_interval.hh \
		src/util/types/datetime.cc \
		src/util/types/datetime.hh \
		src/util/types/datetime_interval.cc \
		src/util/types/datetime_interval.hh \
		src/util/types/datetime_common.hh \
		src/util/types/common.hh \
		src/util/types/id.cc \
		src/util/types/id.hh \
		src/util/types/null.hh \
		src/util/types/data_types.hh \
		src/util/types/convert_common.hh \
		src/util/types/convert_str_base.hh \
		src/util/types/convert_str_pod.hh \
		src/util/types/convert_str_boost_datetime.hh \
		src/util/types/convert_sql_base.hh \
		src/util/types/convert_sql_pod.hh \
		src/util/types/convert_sql_boost_datetime.hh \
		src/util/types/convert_sql_db_types.hh \
		src/util/types/stringify.hh \
		src/util/types/sqlize.hh \
		src/util/types/birthdate.cc \
		src/util/types/birthdate.hh
#
# util/types/libtypes.a


# util/db/libdb.a
#
src_util_db_libdb_a_SOURCES = \
		src/util/db/query/query_old.cc \
		src/util/db/query/filter.cc \
		src/util/db/query/compound_filter.cc \
		src/util/db/query/simple_filter.cc \
		src/util/db/query/base_filters.hh \
		src/util/db/query/query.hh \
		src/util/db/query/query_common.hh \
		src/util/db/query/query_old.hh \
		src/util/db/query/filter.hh \
		src/util/db/query/compound_filter.hh \
		src/util/db/query/simple_filter.hh \
		src/util/db/query/simple_filters.hh \
		src/util/db/query/sql_helper_objects.hh \
		src/util/db/query/filter_it.hh \
		src/util/db/query/sql_operators.hh \
		src/util/db/psql/psql_connection.hh \
		src/util/db/psql/psql_result.hh \
		src/util/db/model/field.hh \
		src/util/db/model/job_queue.hh \
		src/util/db/model/mem_ptr.hh \
		src/util/db/model/model_exception.hh \
		src/util/db/model/model_field_attrs.hh \
		src/util/db/model/model_field.hh \
		src/util/db/model/model_field_list.hh \
		src/util/db/model/model_field_macros.hh \
		src/util/db/model/model.hh \
		src/util/db/connection_factory.hh \
		src/util/db/transaction.hh \
		src/util/db/connection.hh \
		src/util/db/query_param.hh \
		src/util/db/param_query_composition.hh \
		src/util/db/param_query_composition.cc \
		src/util/db/result.hh \
		src/util/db/row.hh \
		src/util/db/value.hh \
		src/util/db/value.cc \
		src/util/db/statement.hh \
		src/util/db/sequence.hh \
		src/util/db/manager.hh \
		src/util/db/manager_tss.hh \
		src/util/db/db_exceptions.hh \
		src/util/db/database.hh \
		src/util/db/manager_old_db.hh
#
# util/db/libdb.a


# util/tz/libtimezones.a
#
src_util_tz_libtimezones_a_SOURCES = \
		src/util/timezones.hh \
		src/util/tz/local_timestamp.hh \
		src/util/tz/local_timestamp.cc \
		src/util/tz/get_psql_handle_of.hh \
		src/util/tz/utc.cc \
		src/util/tz/utc.hh \
		src/util/tz/europe/prague.cc \
		src/util/tz/europe/prague.hh \
		src/util/tz/europe/timezones.hh
#
# util/tz/libtimezones.a


# util/db/test-db
#
src_util_db_test_db_SOURCES = \
		src/util/db/test-db.cc

src_util_db_test_db_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_util_db_test_db_LDADD = \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# util/db/test-db


# util/db/mock
#
UTIL_MOCKDB_SRC = \
		src/util/db/query/query_old.cc \
		src/util/db/query/filter.cc \
		src/util/db/query/compound_filter.cc \
		src/util/db/query/simple_filter.cc \
		src/util/db/query/base_filters.hh \
		src/util/db/query/query.hh \
		src/util/db/query/query_common.hh \
		src/util/db/query/query_old.hh \
		src/util/db/query/filter.hh \
		src/util/db/query/compound_filter.hh \
		src/util/db/query/simple_filter.hh \
		src/util/db/query/simple_filters.hh \
		src/util/db/query/sql_helper_objects.hh \
		src/util/db/query/filter_it.hh \
		src/util/db/query/sql_operators.hh \
		src/util/db/mock/mock_connection.hh \
		src/util/db/mock/mock_result.hh \
		src/util/db/model/field.hh \
		src/util/db/model/job_queue.hh \
		src/util/db/model/mem_ptr.hh \
		src/util/db/model/model_exception.hh \
		src/util/db/model/model_field_attrs.hh \
		src/util/db/model/model_field.hh \
		src/util/db/model/model_field_list.hh \
		src/util/db/model/model_field_macros.hh \
		src/util/db/model/model.hh \
		src/util/db/connection_factory.hh \
		src/util/db/transaction.hh \
		src/util/db/connection.hh \
		src/util/db/result.hh \
		src/util/db/row.hh \
		src/util/db/value.hh \
		src/util/db/statement.hh \
		src/util/db/sequence.hh \
		src/util/db/manager.hh \
		src/util/db/manager_tss.hh \
		src/util/db/db_exceptions.hh \
		src/util/db/manager_old_db.hh


src_util_db_test_mockdb_SOURCES = \
		src/util/db/test-mockdb.cc \
		$(UTIL_MOCKDB_SRC)

src_util_db_test_mockdb_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_util_db_test_mockdb_LDADD = \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_SYSTEM_LIB)

#
# util/db/mock


# util/hp/libhp.a
#
src_util_hp_libhp_a_SOURCES = \
		src/util/hp/hp.cc \
		src/util/hp/hp.hh \
		src/util/hp/hpmail.cc \
		src/util/hp/hpmail.hh \
		src/util/hp/hpmailbatchstate.cc \
		src/util/hp/hpmailbatchstate.hh
#
# util/hp/libhp.a

# util/hp/crc32
#
src_util_hp_crc32_SOURCES = \
		src/util/hp/crc32.cc

src_util_hp_crc32_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_util_hp_crc32_LDADD = \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB)
#
# util/hp/crc32

# src/bin/corba/util/libcorbautil.a
#
src_bin_corba_util_libcorbautil_a_SOURCES = \
		src/bin/corba/util/corba_conversions_buffer.cc \
		src/bin/corba/util/corba_conversions_nullable_types.cc \
		src/bin/corba/util/corba_conversions_string.cc \
		src/bin/corba/util/corba_conversions_isodate.cc \
		src/bin/corba/util/corba_conversions_isodatetime.cc
#
# src/bin/corba/util/libcorbautil.a


# src/old_utils/liboldutils.a
#
src_deprecated_util_liboldutils_a_SOURCES = \
		src/deprecated/util/dbsql.cc \
		src/deprecated/util/log.cc \
		src/deprecated/util/pqsql.cc \
		src/deprecated/util/timeclock.cc \
		src/deprecated/util/util.cc \
		src/deprecated/util/dbsql.hh \
		src/deprecated/util/log.hh \
		src/deprecated/util/pqsql.hh \
		src/deprecated/util/timeclock.hh \
		src/deprecated/util/util.hh
#
# src/old_utils/liboldutils.a


# src/model/libmodel.a
#
src_deprecated_model_libmodel_a_SOURCES = \
		src/deprecated/model/object_registry_filter.cc \
		src/deprecated/model/object_registry_filter.hh \
		src/deprecated/model/object_filter.cc \
		src/deprecated/model/object_filter.hh \
		src/deprecated/model/bank_statement_filter.cc \
		src/deprecated/model/bank_statement_filter.hh \
		src/deprecated/model/bank_payment_filter.cc \
		src/deprecated/model/bank_payment_filter.hh \
		src/deprecated/model/domain_filter.cc \
		src/deprecated/model/domain_filter.hh \
		src/deprecated/model/contact_filter.cc \
		src/deprecated/model/contact_filter.hh \
		src/deprecated/model/nsset_filter.cc \
		src/deprecated/model/nsset_filter.hh \
		src/deprecated/model/registrar_filter.cc \
		src/deprecated/model/registrar_filter.hh \
		src/deprecated/model/filter_filter.cc \
		src/deprecated/model/filter_filter.hh \
		src/deprecated/model/object_state_filter.cc \
		src/deprecated/model/object_state_filter.hh \
		src/deprecated/model/invoice_filter.cc \
		src/deprecated/model/invoice_filter.hh \
		src/deprecated/model/public_request_filter.cc \
		src/deprecated/model/public_request_filter.hh \
		src/deprecated/model/mail_filter.cc \
		src/deprecated/model/mail_filter.hh \
		src/deprecated/model/file_filter.cc \
		src/deprecated/model/file_filter.hh \
		src/deprecated/model/keyset_filter.cc \
		src/deprecated/model/keyset_filter.hh \
		src/deprecated/model/online_statement_filter.cc \
		src/deprecated/model/online_statement_filter.hh \
		src/deprecated/model/log_filter.cc \
		src/deprecated/model/union_filter.cc \
		src/deprecated/model/union_filter.hh \
		src/deprecated/model/zone_filter.cc \
		src/deprecated/model/zone_filter.hh \
		src/deprecated/model/zone_ns_filter.cc \
		src/deprecated/model/zone_ns_filter.hh \
		src/deprecated/model/zone_soa_filter.cc \
		src/deprecated/model/zone_soa_filter.hh \
		src/deprecated/model/message_filter.cc \
		src/deprecated/model/message_filter.hh \
		src/deprecated/model/log_filter.hh \
		src/deprecated/model/model_filters.hh
#
# src/model/libmodel.a

# src/model/test-model
#
src_deprecated_model_test_model_SOURCES = \
		src/deprecated/model/test-model.cc

src_deprecated_model_test_model_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_deprecated_model_test_model_LDADD = \
		src/deprecated/model/libmodel.a \
		src/libfred/banking/libbanking.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/model/test-model


# src/libfred/libfred.a
#
src_libfred_libfred_a_SOURCES = \
		src/libfred/registry.cc \
		src/libfred/registry.hh \
		src/libfred/db_settings.hh \
		src/libfred/common_new.hh \
		src/libfred/common_object.hh \
		src/libfred/common_impl.cc \
		src/libfred/common_impl.hh \
		src/libfred/object.hh \
		src/libfred/object_impl.cc \
		src/libfred/object_impl.hh \
		src/libfred/common_impl_new.cc \
		src/libfred/common_impl_new.hh \
		src/libfred/model_registrar_acl.cc \
		src/libfred/model_registrar_acl.hh \
		src/libfred/model_registrar.cc \
		src/libfred/model_registrar.hh \
		src/libfred/model_zone.cc \
		src/libfred/model_zone.hh \
		src/libfred/model_zone_ns.cc \
		src/libfred/model_zone_ns.hh \
		src/libfred/model_zone_soa.cc \
		src/libfred/model_zone_soa.hh \
		src/libfred/registrable_object/domain.cc \
		src/libfred/registrable_object/domain.hh \
		src/libfred/registrable_object/contact.cc \
		src/libfred/registrable_object/contact.hh \
		src/libfred/registrable_object/nsset.cc \
		src/libfred/registrable_object/nsset.hh \
		src/libfred/registrable_object/keyset.cc \
		src/libfred/registrable_object/keyset.hh \
		src/libfred/registrar.cc \
		src/libfred/registrar.hh \
		src/libfred/credit.cc \
		src/libfred/credit.hh \
		src/libfred/invoicing/invoice.cc \
		src/libfred/invoicing/invoice.hh \
		src/libfred/zone.cc \
		src/libfred/zone.hh \
		src/libfred/zone_ns/create_zone_ns.cc \
		src/libfred/zone_ns/create_zone_ns.hh \
		src/libfred/zone_ns/exceptions.cc \
		src/libfred/zone_ns/exceptions.hh \
		src/libfred/zone_ns/info_zone_ns_data.hh \
		src/libfred/zone_ns/info_zone_ns.cc \
		src/libfred/zone_ns/info_zone_ns.hh \
		src/libfred/zone_ns/update_zone_ns.cc \
		src/libfred/zone_ns/update_zone_ns.hh \
		src/libfred/zone_ns/util.cc \
		src/libfred/zone_ns/util.hh \
		src/libfred/blacklist.cc \
		src/libfred/blacklist.hh \
		src/libfred/documents.cc \
		src/libfred/documents.hh \
		src/libfred/info_buffer.cc \
		src/libfred/info_buffer.hh \
		src/libfred/mail.cc \
		src/libfred/mail.hh \
		src/libfred/notify.hh \
		src/libfred/notify.cc \
		src/libfred/logger_client.hh \
		src/libfred/epp_corba_client.hh \
		src/libfred/file_transferer.hh \
		src/libfred/file.cc \
		src/libfred/file.hh \
		src/libfred/filter.cc \
		src/libfred/filter.hh \
		src/libfred/mailer.hh \
		src/libfred/sql.hh \
		src/libfred/exceptions.hh \
		src/libfred/types.hh \
		src/libfred/obj_types.hh \
		src/libfred/object_states.cc \
		src/libfred/object_states.hh \
		src/libfred/common_diff.cc \
		src/libfred/common_diff.hh \
		src/libfred/reminder.cc \
		src/libfred/reminder.hh \
		src/libfred/opcontext.cc \
		src/libfred/opcontext.hh \
		src/libfred/registrable_object/contact/copy_contact.hh \
		src/libfred/registrable_object/contact/copy_contact.cc \
		src/libfred/registrable_object/domain/renew_domain.hh \
		src/libfred/registrable_object/domain/renew_domain.cc \
		src/libfred/registrable_object/domain/update_domain.hh \
		src/libfred/registrable_object/domain/update_domain.cc \
		src/libfred/registrable_object/domain/info_domain_data.hh \
		src/libfred/registrable_object/domain/info_domain_data.cc \
		src/libfred/registrable_object/domain/info_domain_diff.hh \
		src/libfred/registrable_object/domain/info_domain_diff.cc \
		src/libfred/registrable_object/domain/info_domain.hh \
		src/libfred/registrable_object/domain/info_domain.cc \
		src/libfred/registrable_object/domain/info_domain_impl.cc \
		src/libfred/registrable_object/domain/info_domain_impl.hh \
		src/libfred/registrable_object/domain/info_domain_output.cc \
		src/libfred/registrable_object/domain/info_domain_output.hh \
		src/libfred/registrable_object/domain/enum_validation_extension.hh \
		src/libfred/registrable_object/domain/create_domain_name_blacklist_id.hh \
		src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc \
		src/libfred/registrable_object/domain/transfer_domain.hh \
		src/libfred/registrable_object/domain/transfer_domain.cc \
		src/libfred/registrable_object/domain/domain.hh \
		src/libfred/registrable_object/domain/domain.cc \
		src/libfred/registrable_object/nsset/update_nsset.hh \
		src/libfred/registrable_object/nsset/update_nsset.cc \
		src/libfred/registrable_object/nsset/info_nsset.hh \
		src/libfred/registrable_object/nsset/info_nsset.cc \
		src/libfred/registrable_object/nsset/info_nsset_data.hh \
		src/libfred/registrable_object/nsset/info_nsset_data.cc \
		src/libfred/registrable_object/nsset/info_nsset_diff.hh \
		src/libfred/registrable_object/nsset/info_nsset_diff.cc \
		src/libfred/registrable_object/nsset/info_nsset_impl.cc \
		src/libfred/registrable_object/nsset/info_nsset_impl.hh \
		src/libfred/registrable_object/nsset/info_nsset_output.cc \
		src/libfred/registrable_object/nsset/info_nsset_output.hh \
		src/libfred/registrable_object/keyset/update_keyset.hh \
		src/libfred/registrable_object/keyset/keyset_dns_key.hh \
		src/libfred/registrable_object/keyset/update_keyset.cc \
		src/libfred/registrable_object/keyset/create_keyset.cc \
		src/libfred/registrable_object/keyset/create_keyset.hh \
		src/libfred/registrable_object/keyset/info_keyset_data.hh \
		src/libfred/registrable_object/keyset/info_keyset_data.cc \
		src/libfred/registrable_object/keyset/info_keyset_diff.hh \
		src/libfred/registrable_object/keyset/info_keyset_diff.cc \
		src/libfred/registrable_object/keyset/info_keyset.hh \
		src/libfred/registrable_object/keyset/info_keyset.cc \
		src/libfred/registrable_object/keyset/info_keyset_impl.cc \
		src/libfred/registrable_object/keyset/info_keyset_impl.hh \
		src/libfred/registrable_object/keyset/info_keyset_output.cc \
		src/libfred/registrable_object/keyset/info_keyset_output.hh \
		src/libfred/registrable_object/keyset/transfer_keyset.hh \
		src/libfred/registrable_object/keyset/transfer_keyset.cc \
		src/libfred/registrable_object/contact/delete_contact.hh \
		src/libfred/registrable_object/contact/delete_contact.cc \
		src/libfred/registrable_object/contact/info_contact_data.hh \
		src/libfred/registrable_object/contact/info_contact_data.cc \
		src/libfred/registrable_object/contact/info_contact_diff.hh \
		src/libfred/registrable_object/contact/info_contact_diff.cc \
		src/libfred/registrable_object/contact/info_contact.hh \
		src/libfred/registrable_object/contact/info_contact.cc \
		src/libfred/registrable_object/contact/info_contact_impl.cc \
		src/libfred/registrable_object/contact/info_contact_impl.hh \
		src/libfred/registrable_object/contact/info_contact_output.cc \
		src/libfred/registrable_object/contact/info_contact_output.hh \
		src/libfred/registrable_object/contact/place_address.cc \
		src/libfred/registrable_object/contact/place_address.hh \
		src/libfred/registrable_object/contact/transfer_contact.hh \
		src/libfred/registrable_object/contact/transfer_contact.cc \
		src/libfred/registrable_object/contact/undisclose_address.hh \
		src/libfred/registrable_object/contact/undisclose_address.cc \
		src/backend/mojeid/checkers.cc \
		src/backend/mojeid/checkers.hh \
		src/backend/mojeid/check_collector.hh \
		src/libfred/object/object.hh \
		src/libfred/object/object.cc \
		src/libfred/object/object_impl.hh \
		src/libfred/object/object_impl.cc \
		src/libfred/object/generate_authinfo_password.hh \
		src/libfred/object/generate_authinfo_password.cc \
		src/libfred/object/generated_authinfo_password.hh \
		src/libfred/object/generated_authinfo_password.cc \
		src/libfred/object/generated_authinfo_password_exception.hh \
		src/libfred/object/get_id_of_registered.hh \
		src/libfred/object/get_id_of_registered.cc \
		src/libfred/object/transfer_object.hh \
		src/libfred/object/transfer_object.cc \
		src/libfred/object/transfer_object_exception.hh \
		src/libfred/object/object.cc \
		src/libfred/poll/message_type.hh \
		src/libfred/poll/message_type_set.hh \
		src/libfred/poll/create_poll_message.hh \
		src/libfred/poll/create_poll_message.cc \
		src/libfred/poll/create_update_object_poll_message.hh \
		src/libfred/poll/create_update_object_poll_message.cc \
		src/libfred/poll/get_request_fee_message.hh \
		src/libfred/poll/get_request_fee_message.cc \
		src/libfred/poll/create_request_fee_info_message.hh \
		src/libfred/poll/create_request_fee_info_message.cc \
		src/libfred/poll/create_request_fee_info_messages.hh \
		src/libfred/poll/create_request_fee_info_messages.cc \
		src/libfred/poll/create_low_credit_messages.hh \
		src/libfred/poll/create_low_credit_messages.cc \
		src/libfred/poll/create_state_messages.hh \
		src/libfred/poll/create_state_messages.cc \
		src/libfred/opexception.hh \
		src/libfred/registrable_object/contact/copy_history_impl.hh \
		src/libfred/registrable_object/contact/copy_history_impl.cc \
		src/libfred/registrable_object/contact/merge_contact.hh \
		src/libfred/registrable_object/contact/merge_contact.cc \
		src/libfred/registrable_object/contact/create_contact.cc \
		src/libfred/registrable_object/contact/create_contact.hh \
		src/libfred/registrable_object/contact/update_contact.cc \
		src/libfred/registrable_object/contact/update_contact.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.cc \
		src/libfred/registrable_object/contact/contact_enum.hh \
		src/libfred/registrable_object/nsset/create_nsset.cc \
		src/libfred/registrable_object/nsset/create_nsset.hh \
		src/libfred/registrable_object/nsset/nsset_dns_host.hh \
		src/libfred/registrable_object/nsset/transfer_nsset.hh \
		src/libfred/registrable_object/nsset/transfer_nsset.cc \
		src/libfred/registrable_object/domain/create_domain.cc \
		src/libfred/registrable_object/domain/create_domain.hh \
		src/libfred/registrable_object/domain/copy_history_impl.hh \
		src/libfred/registrable_object/domain/copy_history_impl.cc \
		src/libfred/registrable_object/contact/merge_contact_selection.cc \
		src/libfred/registrable_object/contact/merge_contact_selection.hh \
		src/libfred/registrable_object/contact/find_contact_duplicates.cc \
		src/libfred/registrable_object/contact/find_contact_duplicates.hh \
		src/libfred/registrable_object/contact/merge_contact_email_notification_data.cc \
		src/libfred/registrable_object/contact/merge_contact_email_notification_data.hh \
		src/libfred/registrable_object/domain/delete_domain.hh \
		src/libfred/registrable_object/domain/delete_domain.cc \
		src/libfred/registrable_object/domain/domain_name.hh \
		src/libfred/registrable_object/domain/domain_name.cc \
		src/libfred/zone/create_zone.hh \
		src/libfred/zone/create_zone.cc \
		src/libfred/zone/exceptions.hh \
		src/libfred/zone/exceptions.cc \
		src/libfred/zone/info_zone_data.hh \
		src/libfred/zone/info_zone_data.cc \
		src/libfred/zone/info_zone.hh \
		src/libfred/zone/info_zone.cc \
		src/libfred/zone/update_zone.hh \
		src/libfred/zone/update_zone.cc \
		src/libfred/zone/util.hh \
		src/libfred/zone/util.cc \
		src/libfred/zone/zone.hh \
		src/libfred/zone/zone.cc \
		src/libfred/zone_soa/create_zone_soa.hh \
		src/libfred/zone_soa/create_zone_soa.cc \
		src/libfred/zone_soa/exceptions.hh \
		src/libfred/zone_soa/info_zone_soa.hh \
		src/libfred/zone_soa/info_zone_soa.cc \
		src/libfred/zone_soa/info_zone_soa_data.hh \
		src/libfred/zone_soa/update_zone_soa.hh \
		src/libfred/zone_soa/update_zone_soa.cc \
		src/libfred/registrable_object/contact/verification/create_check.hh \
		src/libfred/registrable_object/contact/verification/create_check.cc \
		src/libfred/registrable_object/contact/verification/update_check.hh \
		src/libfred/registrable_object/contact/verification/update_check.cc \
		src/libfred/registrable_object/contact/verification/info_check.hh \
		src/libfred/registrable_object/contact/verification/info_check.cc \
		src/libfred/registrable_object/contact/verification/list_checks.hh \
		src/libfred/registrable_object/contact/verification/list_checks.cc \
		src/libfred/registrable_object/contact/verification/list_enum_objects.hh \
		src/libfred/registrable_object/contact/verification/list_enum_objects.cc \
		src/libfred/registrable_object/contact/verification/create_test.hh \
		src/libfred/registrable_object/contact/verification/create_test.cc \
		src/libfred/registrable_object/contact/verification/update_test.hh \
		src/libfred/registrable_object/contact/verification/update_test.cc \
		src/libfred/registrable_object/domain/check_domain.hh \
		src/libfred/registrable_object/domain/check_domain.cc \
		src/libfred/registrable_object/contact/check_contact.cc \
		src/libfred/registrable_object/contact/check_contact.hh \
		src/libfred/registrable_object/nsset/check_nsset.cc \
		src/libfred/registrable_object/nsset/check_nsset.hh \
		src/libfred/registrable_object/nsset/delete_nsset.cc \
		src/libfred/registrable_object/nsset/delete_nsset.hh \
		src/libfred/registrable_object/nsset/copy_history_impl.hh \
		src/libfred/registrable_object/nsset/copy_history_impl.cc \
		src/libfred/registrable_object/keyset/check_keyset.cc \
		src/libfred/registrable_object/keyset/check_keyset.hh \
		src/libfred/registrable_object/keyset/check_dns_key.cc \
		src/libfred/registrable_object/keyset/check_dns_key.hh \
		src/libfred/registrable_object/keyset/copy_history_impl.hh \
		src/libfred/registrable_object/keyset/copy_history_impl.cc \
		src/libfred/object/check_handle.cc \
		src/libfred/object/check_handle.hh\
		src/libfred/object/transfer_object.cc \
		src/libfred/object/transfer_object.hh\
		src/libfred/registrable_object/keyset/delete_keyset.cc \
		src/libfred/registrable_object/keyset/delete_keyset.hh \
		src/libfred/registrar/registrar_impl.hh \
		src/libfred/registrar/registrar_impl.cc \
		src/libfred/registrar/info_registrar.cc \
		src/libfred/registrar/info_registrar.hh \
		src/libfred/registrar/info_registrar_data.cc \
		src/libfred/registrar/info_registrar_data.hh \
		src/libfred/registrar/info_registrar_diff.cc \
		src/libfred/registrar/info_registrar_diff.hh \
		src/libfred/registrar/registrar_zone_access.hh \
		src/libfred/registrar/registrar_zone_access.cc \
		src/libfred/registrar/create_registrar.cc \
		src/libfred/registrar/create_registrar.hh \
		src/libfred/registrar/info_registrar_impl.cc \
		src/libfred/registrar/info_registrar_impl.hh \
		src/libfred/registrar/info_registrar_output.cc \
		src/libfred/registrar/info_registrar_output.hh \
		src/libfred/registrar/get_registrar_handles.cc \
		src/libfred/registrar/get_registrar_handles.hh \
		src/libfred/registrar/check_registrar.hh \
		src/libfred/registrar/check_registrar.cc \
		src/libfred/registrar/group/exceptions.hh \
		src/libfred/registrar/group/registrar_group_type.hh \
		src/libfred/registrar/group/create_registrar_group.hh \
		src/libfred/registrar/group/create_registrar_group.cc \
		src/libfred/registrar/group/update_registrar_group.hh \
		src/libfred/registrar/group/update_registrar_group.cc \
		src/libfred/registrar/group/cancel_registrar_group.hh \
		src/libfred/registrar/group/cancel_registrar_group.cc \
		src/libfred/registrar/group/get_registrar_groups.hh \
		src/libfred/registrar/group/get_registrar_groups.cc \
		src/libfred/registrar/group/membership/exceptions.hh \
		src/libfred/registrar/group/membership/registrar_group_membership_types.hh \
		src/libfred/registrar/group/membership/create_registrar_group_membership.hh \
		src/libfred/registrar/group/membership/create_registrar_group_membership.cc \
		src/libfred/registrar/group/membership/end_registrar_group_membership.hh \
		src/libfred/registrar/group/membership/end_registrar_group_membership.cc \
		src/libfred/registrar/group/membership/info_group_membership_by_group.hh\
		src/libfred/registrar/group/membership/info_group_membership_by_group.cc \
		src/libfred/registrar/group/membership/info_group_membership_by_registrar.hh \
		src/libfred/registrar/group/membership/info_group_membership_by_registrar.cc \
		src/libfred/registrar/certification/exceptions.hh \
		src/libfred/registrar/certification/registrar_certification_type.hh \
		src/libfred/registrar/certification/create_registrar_certification.hh \
		src/libfred/registrar/certification/create_registrar_certification.cc \
		src/libfred/registrar/certification/update_registrar_certification.hh \
		src/libfred/registrar/certification/update_registrar_certification.cc \
		src/libfred/registrar/certification/get_registrar_certifications.hh \
		src/libfred/registrar/certification/get_registrar_certifications.cc \
		src/libfred/registrar/zone_credit.hh \
		src/libfred/registrar/zone_credit.cc \
		src/libfred/registrar/registrar_zone_credit.hh \
		src/libfred/registrar/registrar_zone_credit.cc \
		src/libfred/registrar/get_registrar_zone_credit.hh \
		src/libfred/registrar/get_registrar_zone_credit.cc \
		src/libfred/object/object_id_handle_pair.hh \
		src/libfred/object/object_id_handle_pair.cc \
		src/libfred/object_state/object_state_name.hh \
		src/libfred/object_state/object_has_state.hh \
		src/libfred/object_state/object_has_state.cc \
		src/libfred/object_state/lock_object_state_request_lock.hh \
		src/libfred/object_state/lock_object_state_request_lock.cc \
		src/libfred/object_state/get_object_state_descriptions.hh \
		src/libfred/object_state/get_object_state_descriptions.cc \
		src/libfred/object_state/get_object_states.hh \
		src/libfred/object_state/get_object_states.cc \
		src/libfred/object_state/get_object_states_by_history_id.hh \
		src/libfred/object_state/get_object_states_by_history_id.cc \
		src/libfred/object_state/get_object_state_id_map.hh \
		src/libfred/object_state/get_object_state_id_map.cc \
		src/libfred/object_state/get_blocking_status_desc_list.cc \
		src/libfred/object_state/get_blocking_status_desc_list.hh \
		src/libfred/object_state/perform_object_state_request.hh \
		src/libfred/object_state/perform_object_state_request.cc \
		src/libfred/object_state/cancel_object_state_request_id.cc \
		src/libfred/object_state/cancel_object_state_request_id.hh \
		src/libfred/object_state/create_object_state_request_id.cc \
		src/libfred/object_state/create_object_state_request_id.hh \
		src/libfred/object_state/create_admin_object_block_request_id.cc \
		src/libfred/object_state/create_admin_object_block_request_id.hh \
		src/libfred/object_state/create_admin_object_state_restore_request_id.cc \
		src/libfred/object_state/create_admin_object_state_restore_request_id.hh \
		src/libfred/object_state/clear_admin_object_state_request_id.cc \
		src/libfred/object_state/clear_admin_object_state_request_id.hh \
		src/libfred/object_state/clear_object_state_request_id.cc \
		src/libfred/object_state/clear_object_state_request_id.hh \
		src/libfred/object_state/typedefs.hh \
		src/libfred/object/object_id_handle_pair.hh \
		src/libfred/object/object_id_handle_pair.cc \
		src/libfred/notifier/enqueue_notification.cc \
		src/libfred/notifier/gather_email_data/gather_email_addresses.cc \
		src/libfred/notifier/gather_email_data/gather_email_content.cc \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/contact.cc \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/domain.cc \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.cc \
		src/libfred/notifier/gather_email_data/objecttype_specific_impl/nsset.cc \
		src/libfred/notifier/process_one_notification_request.cc
#
# src/libfred/libfred.a


# util/libpassword_storage.a
#
src_util_libpassword_storage_a_SOURCES = \
		src/util/password_storage.cc \
		src/util/password_storage.hh \
		src/util/password_storage/base64.cc \
		src/util/password_storage/base64.hh \
		src/util/password_storage/password_data.hh \
		src/util/password_storage/impl/check_result.hh \
		src/util/password_storage/impl/generate_salt.hh \
		src/util/password_storage/impl/pbkdf2.cc \
		src/util/password_storage/impl/pbkdf2.hh \
		src/util/password_storage/impl/plaintext.cc \
		src/util/password_storage/impl/plaintext.hh \
		src/util/password_storage/impl/check_equality.cc \
		src/util/password_storage/impl/check_equality.hh
#
# util/libpassword_storage.a


# src/epp/impl/registraracl/libauthentic_registrar.a
#
src_epp_impl_registraracl_libauthentic_registrar_a_SOURCES = \
		src/backend/epp/impl/registraracl/authentic_registrar.cc \
		src/backend/epp/impl/registraracl/authentic_registrar.hh
#
# src/epp/impl/registraracl/libauthentic_registrar.a


# src/libfred/public_request/libpublic_request.a
#
src_libfred_public_request_libpublic_request_a_SOURCES = \
		src/libfred/public_request/public_request.cc \
		src/libfred/public_request/public_request.hh \
		src/libfred/public_request/public_request_impl.cc \
		src/libfred/public_request/public_request_impl.hh \
		src/libfred/public_request/public_request_type_iface.hh \
		src/libfred/public_request/public_request_auth_type_iface.hh \
		src/libfred/public_request/public_request_authinfo_impl.cc \
		src/libfred/public_request/public_request_authinfo_impl.hh \
		src/libfred/public_request/public_request_block_impl.cc \
		src/libfred/public_request/public_request_block_impl.hh \
		src/libfred/public_request/public_request_personalinfo_impl.cc \
		src/libfred/public_request/public_request_personalinfo_impl.hh \
		src/libfred/public_request/create_public_request.cc \
		src/libfred/public_request/create_public_request.hh \
		src/libfred/public_request/create_public_request_auth.cc \
		src/libfred/public_request/create_public_request_auth.hh \
		src/libfred/public_request/public_request_lock_guard.cc \
		src/libfred/public_request/public_request_lock_guard.hh \
		src/libfred/public_request/public_request_object_lock_guard.cc \
		src/libfred/public_request/public_request_object_lock_guard.hh \
		src/libfred/public_request/update_public_request.cc \
		src/libfred/public_request/update_public_request.hh \
		src/libfred/public_request/info_public_request.cc \
		src/libfred/public_request/info_public_request_auth.cc \
		src/libfred/public_request/get_opened_public_request.cc
#
# src/libfred/public_request/libpublic_request.a


# src/libfred/banking/libbanking.a
#
src_libfred_banking_libbanking_a_SOURCES = \
		src/libfred/banking/model_bank_statement.cc \
		src/libfred/banking/model_bank_statement.hh \
		src/libfred/banking/model_bank_payment.cc \
		src/libfred/banking/model_bank_payment.hh \
		src/libfred/banking/bank_common.cc \
		src/libfred/banking/bank_common.hh \
		src/libfred/banking/bank_payment.hh \
		src/libfred/banking/bank_payment_impl.hh \
		src/libfred/banking/bank_payment_list.hh \
		src/libfred/banking/bank_payment_list_impl.hh \
		src/libfred/banking/bank_manager.cc \
		src/libfred/banking/bank_manager.hh \
		src/libfred/banking/bank_exporter.cc \
		src/libfred/banking/bank_exporter.hh \
		src/libfred/banking/bank_statement.hh \
		src/libfred/banking/bank_statement_impl.hh \
		src/libfred/banking/bank_statement_list.hh \
		src/libfred/banking/bank_statement_list_impl.hh
#
# src/libfred/banking/libbanking.a


# src/libfred/messages/libmessages.a
#
src_libfred_messages_libmessages_a_SOURCES = \
		src/libfred/messages/messages_impl.cc \
		src/libfred/messages/messages_impl.hh
#
# src/libfred/messages/libmessages.a


# src/libfred/requests/librequests.a
#
src_libfred_requests_librequests_a_SOURCES = \
		src/libfred/requests/model_request_property_name.hh \
		src/libfred/requests/model_request.cc \
		src/libfred/requests/model_session.cc \
		src/libfred/requests/model_request.hh \
		src/libfred/requests/model_session.hh \
		src/libfred/requests/model_request_property_name.cc \
		src/libfred/requests/session.hh \
		src/libfred/requests/session.cc \
		src/libfred/requests/request.hh \
		src/libfred/requests/request_impl.hh \
		src/libfred/requests/request_list.hh \
		src/libfred/requests/request_list.cc \
		src/libfred/requests/request_manager.hh \
		src/libfred/requests/request_manager.cc \
		src/libfred/requests/request_cache.hh \
		src/libfred/requests/request_cache.cc \
		src/libfred/requests/request_property_name_cache.hh \
		src/libfred/requests/request_property_name_cache.cc \
		src/libfred/requests/session_cache.hh \
		src/libfred/requests/session_cache.cc
#
# src/libfred/requests/librequests.a


# src/libfred/contact_verification/libcontact_verification.a
#
src_libfred_contact_verification_libcontact_verification_a_SOURCES = \
		src/backend/contact_verification/public_request_contact_verification_impl.cc \
		src/backend/contact_verification/public_request_contact_verification_impl.hh \
		src/util/db/nullable.hh \
		src/libfred/contact_verification/contact.hh \
		src/libfred/contact_verification/contact.cc \
		src/libfred/contact_verification/django_email_format.hh \
		src/libfred/contact_verification/contact_verification_checkers.hh \
		src/libfred/contact_verification/contact_verification_checkers.cc \
		src/libfred/contact_verification/contact_conditional_identification_impl.cc \
		src/libfred/contact_verification/contact_conditional_identification_impl.hh \
		src/libfred/contact_verification/contact_identification_impl.cc \
		src/libfred/contact_verification/contact_identification_impl.hh \
		src/libfred/contact_verification/contact_verification_state.cc \
		src/libfred/contact_verification/contact_verification_state.hh \
		src/libfred/contact_verification/contact_verification_validators.cc \
		src/libfred/contact_verification/contact_verification_validators.hh \
		src/libfred/contact_verification/contact_verification_password.cc \
		src/libfred/contact_verification/contact_verification_password.hh \
		src/libfred/contact_verification/contact_validator.hh

#
# src/libfred/contact_verification/libcontact_verification.a

# src/admin/contact/verification/libadmin_contact_verification.a
#
src_backend_admin_contact_verification_libadmin_contact_verification_a_SOURCES = \
		src/backend/admin/contact/verification/create_test_impl_prototypes.hh \
		src/backend/admin/contact/verification/create_test_impl_prototypes.cc \
		src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh \
		src/backend/admin/contact/verification/delete_domains_of_invalid_contact.cc \
		src/backend/admin/contact/verification/enqueue_check.hh \
		src/backend/admin/contact/verification/enqueue_check.cc \
		src/backend/admin/contact/verification/exceptions.hh \
		src/backend/admin/contact/verification/fill_check_queue.hh \
		src/backend/admin/contact/verification/fill_check_queue.cc \
		src/backend/admin/contact/verification/list_active_checks.hh \
		src/backend/admin/contact/verification/list_active_checks.cc \
		src/backend/admin/contact/verification/related_records.hh \
		src/backend/admin/contact/verification/related_records.cc \
		src/backend/admin/contact/verification/resolve_check.hh \
		src/backend/admin/contact/verification/resolve_check.cc \
		src/backend/admin/contact/verification/run_all_enqueued_checks.hh \
		src/backend/admin/contact/verification/run_all_enqueued_checks.cc \
		src/backend/admin/contact/verification/update_tests.hh \
		src/backend/admin/contact/verification/update_tests.cc \
		src/backend/admin/contact/verification/contact_states/enum.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.cc \
		src/backend/admin/contact/verification/test_impl/util.hh \
		src/backend/admin/contact/verification/test_impl/test_contactability.hh \
		src/backend/admin/contact/verification/test_impl/test_contactability.cc \
		src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh \
		src/backend/admin/contact/verification/test_impl/test_cz_address_exists.cc \
		src/backend/admin/contact/verification/test_impl/test_email_exists.hh \
		src/backend/admin/contact/verification/test_impl/test_email_exists.cc \
		src/backend/admin/contact/verification/test_impl/test_email_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_email_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_interface.hh \
		src/backend/admin/contact/verification/test_impl/test_interface.cc \
		src/backend/admin/contact/verification/test_impl/test_name_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_name_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_phone_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_send_letter.hh \
		src/backend/admin/contact/verification/test_impl/test_send_letter.cc \
		src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh \
		src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.cc
#
# src/admin/contact/verification/libadmin_contact_verification.a

# src/admin/notification/libadmin_notification.a
#
src_backend_admin_notification_libadmin_notification_a_SOURCES = \
		src/backend/admin/notification/notification.hh \
		src/backend/admin/notification/notification.cc
#
# src/admin/notification/libadmin_notification.a

# src/admin/domain/libadmin_create_expired_domain.a
#
src_backend_admin_domain_libadmin_create_expired_domain_a_SOURCES = \
		src/backend/admin/domain/create_expired_domain.hh \
		src/backend/admin/domain/create_expired_domain.cc
#
# src/admin/domain/libadmin_create_expired_domain.a


BUILT_SOURCES = \
		src/bin/corba/MessagesSK.cc \
		src/bin/corba/MojeIDSK.cc \
		src/bin/corba/AdministrativeBlockingSK.cc \
		src/bin/corba/FileManagerSK.cc \
		src/bin/corba/MailerSK.cc \
		src/bin/corba/ZoneSK.cc \
		src/bin/corba/DateTimeSK.cc \
		src/bin/corba/NullableTypesSK.cc \
		src/bin/corba/_dataTypesSK.cc \
		src/bin/corba/FiltersSK.cc \
		src/bin/corba/RegistrySK.cc \
		src/bin/corba/WhoisSK.cc \
		src/bin/corba/Whois2SK.cc \
		src/bin/corba/AdminSK.cc \
		src/bin/corba/LoggerSK.cc \
		src/bin/corba/TechCheckSK.cc \
		src/bin/corba/EPPSK.cc \
		src/bin/corba/ZoneGeneratorSK.cc \
		src/bin/corba/ContactVerificationSK.cc \
		src/bin/corba/DomainBrowserSK.cc \
		src/bin/corba/PublicRequestSK.cc \
		src/bin/corba/AdminContactVerificationSK.cc \
		src/bin/corba/corba_conversion_testSK.cc \
		src/bin/corba/NotificationSK.cc \
		src/bin/corba/AutomaticKeysetManagementSK.cc \
		src/bin/corba/RecordStatementSK.cc \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeSK.cc


# src/bin/corba/libcorba.a
#
src_bin_corba_libcorba_a_SOURCES = \
		src/bin/corba/nameservice.cc \
		src/bin/corba/mailer_manager.cc \
		src/bin/corba/nameservice.hh \
		src/bin/corba/mailer_manager.hh \
		src/bin/corba/connection_releaser.hh \
		src/bin/corba/pidfile.hh \
		src/bin/corba/daemonize.hh \
		src/bin/corba/file_manager_client.cc \
		src/bin/corba/file_manager_client.hh \
		src/bin/corba/logger_client_impl.cc \
		src/bin/corba/logger_client_impl.hh \
		src/bin/corba/epp_corba_client_impl.cc \
		src/bin/corba/epp_corba_client_impl.hh

src_bin_corba_libcorba_a_CPPFLAGS = \
		$(AM_CPPFLAGS)
#
# src/bin/corba/libcorba.a


# nodist corba libs
#
nodist_src_bin_corba_libcorbamessages_a_SOURCES = \
		src/bin/corba/MessagesSK.cc \
		src/bin/corba/MessagesDynSK.cc \
		src/bin/corba/Messages.hh

nodist_src_bin_corba_libcorbamojeid_a_SOURCES = \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/BufferDynSK.cc \
		src/bin/corba/Buffer.hh \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/NullableIsoDateDynSK.cc \
		src/bin/corba/NullableIsoDate.hh \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/NullableIsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeDynSK.cc \
		src/bin/corba/NullableIsoDateTime.hh \
		src/bin/corba/MojeIDSK.cc \
		src/bin/corba/MojeIDDynSK.cc

nodist_src_bin_corba_libfilemanager_a_SOURCES = \
		src/bin/corba/FileManagerSK.cc \
		src/bin/corba/FileManagerDynSK.cc \
		src/bin/corba/FileManager.hh

nodist_src_bin_corba_libmailer_a_SOURCES = \
		src/bin/corba/MailerSK.cc \
		src/bin/corba/MailerDynSK.cc \
		src/bin/corba/Mailer.hh

nodist_src_bin_corba_libcorbazone_a_SOURCES = \
		src/bin/corba/ZoneSK.cc \
		src/bin/corba/ZoneDynSK.cc \
		src/bin/corba/Zone.hh

nodist_src_bin_corba_libcorbadatatypes_a_SOURCES = \
		src/bin/corba/_dataTypesSK.cc \
		src/bin/corba/_dataTypesDynSK.cc \
		src/bin/corba/_dataTypes.hh \
		src/bin/corba/DateTimeSK.cc \
		src/bin/corba/DateTimeDynSK.cc \
		src/bin/corba/DateTime.hh

nodist_src_bin_corba_libcorbanullabletypes_a_SOURCES = \
		src/bin/corba/NullableTypesSK.cc \
		src/bin/corba/NullableTypesDynSK.cc \
		src/bin/corba/NullableTypes.hh

nodist_src_bin_corba_libcorbafilters_a_SOURCES = \
		src/bin/corba/FiltersSK.cc \
		src/bin/corba/FiltersDynSK.cc \
		src/bin/corba/Filters.hh

nodist_src_bin_corba_libcorbaregistry_a_SOURCES = \
		src/bin/corba/RegistrySK.cc \
		src/bin/corba/RegistryDynSK.cc \
		src/bin/corba/Registry.hh

nodist_src_bin_corba_libcorbawhois_a_SOURCES = \
		src/bin/corba/WhoisSK.cc \
		src/bin/corba/WhoisDynSK.cc \
		src/bin/corba/Whois.hh

nodist_src_bin_corba_libcorbawhois2_a_SOURCES = \
		src/bin/corba/Whois2SK.cc \
		src/bin/corba/Whois2DynSK.cc \
		src/bin/corba/Whois2.hh

nodist_src_bin_corba_libcorbaadmin_a_SOURCES = \
		src/bin/corba/AdminSK.cc \
		src/bin/corba/AdminDynSK.cc \
		src/bin/corba/Admin.hh

nodist_src_bin_corba_libcorbalogger_a_SOURCES = \
		src/bin/corba/LoggerSK.cc \
		src/bin/corba/LoggerDynSK.cc \
		src/bin/corba/Logger.hh

nodist_src_bin_corba_libcorbatechcheck_a_SOURCES = \
		src/bin/corba/TechCheckSK.cc \
		src/bin/corba/TechCheckDynSK.cc \
		src/bin/corba/TechCheck.hh

nodist_src_bin_corba_libcorbaepp_a_SOURCES = \
		src/bin/corba/EPPSK.cc \
		src/bin/corba/EPPDynSK.cc \
		src/bin/corba/EPP.hh

nodist_src_bin_corba_libcorbazonegenerator_a_SOURCES = \
		src/bin/corba/ZoneGeneratorSK.cc \
		src/bin/corba/ZoneGeneratorDynSK.cc \
		src/bin/corba/ZoneGenerator.hh

nodist_src_bin_corba_libcorbacontactverification_a_SOURCES = \
		src/bin/corba/ContactVerificationSK.cc \
		src/bin/corba/ContactVerificationDynSK.cc \
		src/bin/corba/ContactVerification.hh

nodist_src_bin_corba_libcorbaadmincontactverification_a_SOURCES = \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/NullableIsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeDynSK.cc \
		src/bin/corba/NullableIsoDateTime.hh \
		src/bin/corba/AdminContactVerificationSK.cc \
		src/bin/corba/AdminContactVerificationDynSK.cc \
		src/bin/corba/AdminContactVerificationSK.h \
		src/bin/corba/AdminContactVerification.hh

nodist_src_bin_corba_libcorbaadministrativeblocking_a_SOURCES = \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/IsoDateDynSK.cc \
		src/bin/corba/IsoDate.hh \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/NullableIsoDateDynSK.cc \
		src/bin/corba/NullableIsoDate.hh \
		src/bin/corba/AdministrativeBlockingSK.cc \
		src/bin/corba/AdministrativeBlockingDynSK.cc \
		src/bin/corba/AdministrativeBlocking.hh

nodist_src_bin_corba_libcorbadomainbrowser_a_SOURCES = \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/IsoDateDynSK.cc \
		src/bin/corba/IsoDate.hh \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/NullableIsoDateDynSK.cc \
		src/bin/corba/NullableIsoDate.hh \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/NullableIsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeDynSK.cc \
		src/bin/corba/NullableIsoDateTime.hh \
		src/bin/corba/DomainBrowserSK.cc \
		src/bin/corba/DomainBrowserDynSK.cc \
		src/bin/corba/DomainBrowser.hh

nodist_src_bin_corba_libcorbapublicrequest_a_SOURCES = \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/BufferDynSK.cc \
		src/bin/corba/Buffer.hh \
		src/bin/corba/PublicRequestSK.cc \
		src/bin/corba/PublicRequestDynSK.cc \
		src/bin/corba/PublicRequest.hh

nodist_src_bin_corba_libcorba_conversion_test_a_SOURCES = \
		src/bin/corba/corba_conversion_testSK.cc \
		src/bin/corba/corba_conversion_testDynSK.cc \
		src/bin/corba/corba_conversion_test.hh

nodist_src_bin_corba_libcorbanotification_a_SOURCES = \
		src/bin/corba/NotificationSK.cc \
		src/bin/corba/NotificationDynSK.cc \
		src/bin/corba/Notification.hh

nodist_src_bin_corba_libcorbaautomatickeysetmanagement_a_SOURCES = \
		src/bin/corba/AutomaticKeysetManagementSK.cc \
		src/bin/corba/AutomaticKeysetManagementDynSK.cc \
		src/bin/corba/AutomaticKeysetManagement.hh

nodist_src_bin_corba_libcorbarecordstatement_a_SOURCES = \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/BufferDynSK.cc \
		src/bin/corba/Buffer.hh \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/RecordStatementSK.cc \
		src/bin/corba/RecordStatementDynSK.cc \
		src/bin/corba/RecordStatement.hh

nodist_src_bin_corba_libcorbabuffer_a_SOURCES = \
		src/bin/corba/BufferSK.cc \
		src/bin/corba/BufferDynSK.cc \
		src/bin/corba/Buffer.hh

nodist_src_bin_corba_libcorbaisodate_a_SOURCES = \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/IsoDateDynSK.cc \
		src/bin/corba/IsoDate.hh

nodist_src_bin_corba_libcorbanullableisodate_a_SOURCES = \
		src/bin/corba/IsoDateSK.cc \
		src/bin/corba/IsoDateDynSK.cc \
		src/bin/corba/IsoDate.hh \
		src/bin/corba/NullableIsoDateSK.cc \
		src/bin/corba/NullableIsoDateDynSK.cc \
		src/bin/corba/NullableIsoDate.hh

nodist_src_bin_corba_libcorbaisodatetime_a_SOURCES = \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh

nodist_src_bin_corba_libcorbanullableisodatetime_a_SOURCES = \
		src/bin/corba/IsoDateTimeSK.cc \
		src/bin/corba/IsoDateTimeDynSK.cc \
		src/bin/corba/IsoDateTime.hh \
		src/bin/corba/NullableIsoDateTimeSK.cc \
		src/bin/corba/NullableIsoDateTimeDynSK.cc \
		src/bin/corba/NullableIsoDateTime.hh

#
# nodist corba libs


# idl build
#
src/bin/corba/MessagesSK.cc: $(IDLDIR)/Messages.idl
src/bin/corba/MojeIDSK.cc: $(IDLDIR)/MojeID.idl $(IDLDIR)/NullableIsoDate.idl $(IDLDIR)/IsoDateTime.idl
src/bin/corba/AdministrativeBlockingSK.cc: $(IDLDIR)/AdministrativeBlocking.idl $(IDLDIR)/NullableIsoDate.idl $(IDLDIR)/IsoDate.idl
src/bin/corba/FileManagerSK.cc: $(IDLDIR)/FileManager.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/MailerSK.cc: $(IDLDIR)/Mailer.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/ZoneSK.cc: $(IDLDIR)/Zone.idl
src/bin/corba/DateTimeSK.cc: $(IDLDIR)/DateTime.idl
src/bin/corba/NullableTypesSK.cc: $(IDLDIR)/NullableTypes.idl $(IDLDIR)/DateTime.idl
src/bin/corba/_dataTypesSK.cc: $(IDLDIR)/_dataTypes.idl
src/bin/corba/FiltersSK.cc: $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/RegistrySK.cc: $(IDLDIR)/Registry.idl $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/WhoisSK.cc: $(IDLDIR)/Whois.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/Whois2SK.cc: $(IDLDIR)/Whois2.idl $(IDLDIR)/NullableTypes.idl $(IDLDIR)/DateTime.idl
src/bin/corba/AdminSK.cc: $(IDLDIR)/Admin.idl $(IDLDIR)/Whois.idl $(IDLDIR)/Filters.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/LoggerSK.cc: $(IDLDIR)/Logger.idl $(IDLDIR)/Admin.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/TechCheckSK.cc: $(IDLDIR)/TechCheck.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/EPPSK.cc: $(IDLDIR)/EPP.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/ZoneGeneratorSK.cc: $(IDLDIR)/ZoneGenerator.idl $(IDLDIR)/_dataTypes.idl
src/bin/corba/ContactVerificationSK.cc: $(IDLDIR)/ContactVerification.idl
src/bin/corba/DomainBrowserSK.cc: $(IDLDIR)/DomainBrowser.idl $(IDLDIR)/NullableIsoDate.idl $(IDLDIR)/NullableIsoDateTime.idl
src/bin/corba/AdminContactVerificationSK.cc: $(IDLDIR)/AdminContactVerification.idl $(IDLDIR)/_dataTypes.idl $(IDLDIR)/NullableIsoDateTime.idl $(IDLDIR)/IsoDateTime.idl
src/bin/corba/NotificationSK.cc: $(IDLDIR)/Notification.idl
src/bin/corba/PublicRequestSK.cc: $(IDLDIR)/PublicRequest.idl $(IDLDIR)/NullableTypes.idl $(IDLDIR)/Buffer.idl
src/bin/corba/AutomaticKeysetManagementSK.cc: $(IDLDIR)/AutomaticKeysetManagement.idl
src/bin/corba/RecordStatementSK.cc: $(IDLDIR)/RecordStatement.idl $(IDLDIR)/Buffer.idl $(IDLDIR)/IsoDate.idl $(IDLDIR)/IsoDateTime.idl
src/bin/corba/BufferSK.cc: $(IDLDIR)/Buffer.idl
src/bin/corba/IsoDateSK.cc: $(IDLDIR)/IsoDate.idl
src/bin/corba/NullableIsoDateSK.cc: $(IDLDIR)/NullableIsoDate.idl $(IDLDIR)/IsoDate.idl
src/bin/corba/IsoDateTimeSK.cc: $(IDLDIR)/IsoDateTime.idl
src/bin/corba/NullableIsoDateTimeSK.cc: $(IDLDIR)/NullableIsoDateTime.idl $(IDLDIR)/IsoDateTime.idl

src/bin/corba/%.cc:
		-echo "Trying to compile $@ ... should be specified in \"idl build\" section (*.idl) or list it in noinst_LIBRARIES (*.a) or noinst_HEADERS (*.hh)"
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/bin/corba $<

src/bin/corba/corba_conversion_testSK.cc: $(top_srcdir)/test/util/corba_conversion_test.idl
		omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/bin/corba $(top_srcdir)/test/util/corba_conversion_test.idl

#
# idl build


# src/bin/corba/admin/libadmin_impl.a
#
src_bin_corba_admin_libadmin_impl_a_SOURCES = \
		src/bin/corba/admin/admin_impl.cc \
		src/bin/corba/admin/admin_impl.hh \
		src/bin/corba/admin/session_impl.cc \
		src/bin/corba/admin/session_impl.hh \
		src/bin/corba/admin/user_impl.cc \
		src/bin/corba/admin/user_impl.hh \
		src/bin/corba/admin/filter_impl.cc \
		src/bin/corba/admin/filter_impl.hh \
		src/bin/corba/admin/pagetable_impl.cc \
		src/bin/corba/admin/pagetable_impl.hh \
		src/bin/corba/admin/pagetable_publicrequests.cc \
		src/bin/corba/admin/pagetable_publicrequests.hh \
		src/bin/corba/admin/pagetable_contacts.cc \
		src/bin/corba/admin/pagetable_contacts.hh \
		src/bin/corba/admin/pagetable_domains.cc \
		src/bin/corba/admin/pagetable_domains.hh \
		src/bin/corba/admin/pagetable_filters.cc \
		src/bin/corba/admin/pagetable_filters.hh \
		src/bin/corba/admin/pagetable_invoices.cc \
		src/bin/corba/admin/pagetable_invoices.hh \
		src/bin/corba/admin/pagetable_mails.cc \
		src/bin/corba/admin/pagetable_mails.hh \
		src/bin/corba/admin/pagetable_nssets.cc \
		src/bin/corba/admin/pagetable_nssets.hh \
		src/bin/corba/admin/pagetable_registrars.cc \
		src/bin/corba/admin/pagetable_registrars.hh \
		src/bin/corba/admin/pagetable_files.cc \
		src/bin/corba/admin/pagetable_files.hh \
		src/bin/corba/admin/pagetable_keysets.cc \
		src/bin/corba/admin/pagetable_keysets.hh \
		src/bin/corba/admin/pagetable_logger.cc \
		src/bin/corba/admin/pagetable_logger.hh \
		src/bin/corba/admin/pagetable_logsession.cc \
		src/bin/corba/admin/pagetable_logsession.hh \
		src/bin/corba/admin/pagetable_bank_payments.cc \
		src/bin/corba/admin/pagetable_bank_payments.hh \
		src/bin/corba/admin/pagetable_bank_statements.cc \
		src/bin/corba/admin/pagetable_bank_statements.hh \
		src/bin/corba/admin/pagetable_zones.cc \
		src/bin/corba/admin/pagetable_zones.hh \
		src/bin/corba/admin/pagetable_messages.cc \
		src/bin/corba/admin/pagetable_messages.hh \
		src/bin/corba/admin/str_corbaout.hh \
		src/bin/corba/admin/usertype_conv.hh \
		src/bin/corba/admin/common.cc \
		src/bin/corba/admin/common.hh \
		src/bin/corba/admin/zone_impl.cc \
		src/bin/corba/admin/zone_impl.hh \
		src/bin/corba/admin/bankinginvoicing_impl.cc \
		src/bin/corba/admin/bankinginvoicing_impl.hh
#
# src/bin/corba/admin/libadmin_impl.a


# src/bin/corba/whois/libwhois_impl.a
#
src_bin_corba_whois_libwhois_impl_a_SOURCES = \
		src/bin/corba/whois/whois_impl.cc \
		src/bin/corba/whois/whois_impl.hh
#
# src/bin/corba/whois/libwhois_impl.a


# src/bin/corba/whois/libwhois2_impl.a
#
src_bin_corba_whois_libwhois2_impl_a_SOURCES = \
		src/bin/corba/whois/whois2_impl.cc \
		src/bin/corba/whois/whois2_impl.hh \
		src/backend/whois/nameserver_exists.hh \
		src/backend/whois/nameserver_exists.cc \
		src/backend/whois/is_domain_delete_pending.hh \
		src/backend/whois/is_domain_delete_pending.cc \
		src/backend/whois/domain_expiration_datetime.hh \
		src/backend/whois/domain_expiration_datetime.cc \
		src/backend/whois/registrar_group.hh \
		src/backend/whois/registrar_group.cc \
		src/backend/whois/registrar_certification.hh \
		src/backend/whois/registrar_certification.cc \
		src/backend/whois/zone_list.hh \
		src/backend/whois/zone_list.cc \
		src/backend/whois/whois.hh \
		src/backend/whois/whois.cc

#
# src/bin/corba/whois/libwhois2_impl.a


# src/bin/corba/epp/libepp_impl.a
#
src_bin_corba_epp_libepp_impl_a_SOURCES = \
		src/libfred/contact_verification/cancel_contact_verification.cc \
		src/libfred/contact_verification/cancel_contact_verification.hh \
		src/libfred/contact_verification/contact_verification_state.cc \
		src/libfred/registrable_object/domain/domain.hh \
		src/libfred/registrable_object/domain/domain.cc \
		src/bin/corba/epp/epp_session.hh \
		src/bin/corba/epp/epp_session.cc \
		src/bin/corba/epp/epp_impl.cc \
		src/bin/corba/epp/epp_impl.hh \
		src/bin/corba/epp/contact/contact_corba_conversions.cc \
		src/bin/corba/epp/contact/contact_corba_conversions.hh \
		src/bin/corba/epp/credit/credit_corba_conversions.cc \
		src/bin/corba/epp/credit/credit_corba_conversions.hh \
		src/bin/corba/epp/domain/domain_corba_conversions.cc \
		src/bin/corba/epp/domain/domain_corba_conversions.hh \
		src/bin/corba/epp/poll/poll_corba_conversions.cc \
		src/bin/corba/epp/poll/poll_corba_conversions.hh \
		src/bin/corba/epp/keyset/keyset_corba_conversions.cc \
		src/bin/corba/epp/keyset/keyset_corba_conversions.hh \
		src/bin/corba/epp/nsset/nsset_corba_conversions.cc \
		src/bin/corba/epp/nsset/nsset_corba_conversions.hh \
		src/bin/corba/epp/corba_conversions.hh \
		src/bin/corba/epp/corba_conversions.cc \
		src/bin/corba/epp/messages.cc \
		src/bin/corba/epp/messages.hh \
		src/bin/corba/epp/tech_check.cc \
		src/bin/corba/epp/tech_check.hh \
		src/bin/corba/epp/action.hh \
		src/bin/corba/epp/reason.hh \
		src/bin/corba/epp/response.hh \
		src/bin/corba/util/corba_conversions_string.hh \
		src/bin/corba/util/corba_conversions_string.cc \
		src/bin/corba/util/corba_conversions_int.hh \
		src/backend/epp/impl/disclose_policy.hh \
		src/backend/epp/localization.hh \
		src/backend/epp/localization.cc \
		src/backend/epp/object_states_localized.hh \
		src/backend/epp/param.hh \
		src/backend/epp/reason.hh \
		src/backend/epp/epp_response_failure.hh \
		src/backend/epp/epp_response_failure_localized.hh \
		src/backend/epp/epp_response_failure_localized.cc \
		src/backend/epp/epp_response_success.hh \
		src/backend/epp/epp_response_success_localized.hh \
		src/backend/epp/epp_response_success_localized.cc \
		src/backend/epp/epp_result_failure.hh \
		src/backend/epp/epp_result_failure.cc \
		src/backend/epp/epp_result_failure_localized.hh \
		src/backend/epp/epp_result_failure_localized.cc \
		src/backend/epp/epp_result_success.hh \
		src/backend/epp/epp_result_success_localized.hh \
		src/backend/epp/epp_result_success_localized.cc \
		src/backend/epp/epp_extended_error.hh \
		src/backend/epp/epp_extended_error.cc \
		src/backend/epp/epp_extended_error_localized.hh \
		src/backend/epp/epp_extended_error_localized.cc \
		src/backend/epp/epp_result_code.hh \
		src/backend/epp/notification_data.hh \
		src/backend/epp/session_data.hh \
		src/backend/epp/session_data.cc \
		src/backend/epp/impl/conditionally_enqueue_notification.hh \
		src/backend/epp/impl/conditionally_enqueue_notification.cc \
		src/backend/epp/contact/create_contact_localized.cc \
		src/backend/epp/contact/create_contact_localized.hh \
		src/backend/epp/contact/create_contact.cc \
		src/backend/epp/contact/create_contact.hh \
		src/backend/epp/contact/util.cc \
		src/backend/epp/contact/util.hh \
		src/backend/epp/contact/contact_change.cc \
		src/backend/epp/contact/contact_change.hh \
		src/backend/epp/contact/contact_data.cc \
		src/backend/epp/contact/contact_data.hh \
		src/backend/epp/contact/impl/contact_output.cc \
		src/backend/epp/contact/impl/contact_output.hh \
		src/backend/epp/contact/update_contact_localized.cc \
		src/backend/epp/contact/update_contact_localized.hh \
		src/backend/epp/contact/update_contact.cc \
		src/backend/epp/contact/update_contact.hh \
		src/backend/epp/contact/update_contact_config_data.hh \
		src/backend/epp/contact/info_contact_localized.cc \
		src/backend/epp/contact/info_contact_localized.hh \
		src/backend/epp/contact/info_contact_localized_response.hh \
		src/backend/epp/contact/info_contact_localized_output_data.hh \
		src/backend/epp/contact/info_contact.cc \
		src/backend/epp/contact/info_contact.hh \
		src/backend/epp/contact/info_contact_config_data.hh \
		src/backend/epp/contact/delete_contact_localized.cc \
		src/backend/epp/contact/delete_contact_localized.hh \
		src/backend/epp/contact/delete_contact.cc \
		src/backend/epp/contact/delete_contact.hh \
		src/backend/epp/contact/delete_contact_config_data.hh \
		src/backend/epp/contact/check_contact_localized.cc \
		src/backend/epp/contact/check_contact_localized.hh \
		src/backend/epp/contact/check_contact_localized_response.hh \
		src/backend/epp/contact/check_contact.cc \
		src/backend/epp/contact/check_contact.hh \
		src/backend/epp/contact/check_contact_config_data.hh \
		src/backend/epp/contact/transfer_contact_localized.cc \
		src/backend/epp/contact/transfer_contact_localized.hh \
		src/backend/epp/contact/transfer_contact.cc \
		src/backend/epp/contact/transfer_contact.hh \
		src/backend/epp/contact/transfer_contact_config_data.hh \
		src/backend/epp/contact/contact_handle_registration_obstruction.hh \
		src/backend/epp/contact/contact_handle_registration_obstruction_localized.hh \
		src/backend/epp/contact/contact_disclose.hh \
		src/backend/epp/contact/update_contact_post_hooks.cc \
		src/backend/epp/contact/update_contact_post_hooks.hh \
		src/backend/epp/contact/create_contact_input_data.cc \
		src/backend/epp/contact/create_contact_input_data.hh \
		src/backend/epp/contact/create_contact_config_data.hh \
		src/backend/epp/contact/create_contact_localized_response.hh \
		src/backend/epp/contact/contact_ident.hh \
		src/backend/epp/contact/status_value.hh \
		src/backend/epp/credit/client_credit_output_data.cc \
		src/backend/epp/credit/client_credit_output_data.hh \
		src/backend/epp/credit/client_credit.cc \
		src/backend/epp/credit/client_credit.hh \
		src/backend/epp/credit/client_credit_localized_response.hh \
		src/backend/epp/credit/client_credit_localized.cc \
		src/backend/epp/credit/client_credit_localized.hh \
		src/backend/epp/domain/check_domain_localized.cc \
		src/backend/epp/domain/check_domain_localized.hh \
		src/backend/epp/domain/check_domain_localized_response.hh \
		src/backend/epp/domain/check_domain.cc \
		src/backend/epp/domain/check_domain.hh \
		src/backend/epp/poll/message_type.hh \
		src/backend/epp/poll/poll_acknowledgement_localized.cc \
		src/backend/epp/poll/poll_acknowledgement_localized.hh \
		src/backend/epp/poll/poll_acknowledgement.cc \
		src/backend/epp/poll/poll_acknowledgement.hh \
		src/backend/epp/poll/poll_request_localized.cc \
		src/backend/epp/poll/poll_request_localized.hh \
		src/backend/epp/poll/poll_request.cc \
		src/backend/epp/poll/poll_request.hh \
		src/backend/epp/poll/poll_request_get_update_contact_details.cc \
		src/backend/epp/poll/poll_request_get_update_contact_details.hh \
		src/backend/epp/poll/poll_request_get_update_domain_details.cc \
		src/backend/epp/poll/poll_request_get_update_domain_details.hh \
		src/backend/epp/poll/poll_request_get_update_keyset_details.cc \
		src/backend/epp/poll/poll_request_get_update_keyset_details.hh \
		src/backend/epp/poll/poll_request_get_update_nsset_details.cc \
		src/backend/epp/poll/poll_request_get_update_nsset_details.hh \
		src/backend/epp/poll/poll_request_get_update_contact_details_localized.cc \
		src/backend/epp/poll/poll_request_get_update_contact_details_localized.hh \
		src/backend/epp/poll/poll_request_get_update_domain_details_localized.cc \
		src/backend/epp/poll/poll_request_get_update_domain_details_localized.hh \
		src/backend/epp/poll/poll_request_get_update_keyset_details_localized.cc \
		src/backend/epp/poll/poll_request_get_update_keyset_details_localized.hh \
		src/backend/epp/poll/poll_request_get_update_nsset_details_localized.cc \
		src/backend/epp/poll/poll_request_get_update_nsset_details_localized.hh \
		src/backend/epp/domain/check_domain_config_data.hh \
		src/backend/epp/domain/delete_domain_localized.cc \
		src/backend/epp/domain/delete_domain_localized.hh \
		src/backend/epp/domain/delete_domain.cc \
		src/backend/epp/domain/delete_domain.hh \
		src/backend/epp/domain/delete_domain_config_data.hh \
		src/backend/epp/domain/info_domain_localized.cc \
		src/backend/epp/domain/info_domain_localized.hh \
		src/backend/epp/domain/info_domain_localized_response.hh \
		src/backend/epp/domain/info_domain_localized_output_data.hh \
		src/backend/epp/domain/info_domain.cc \
		src/backend/epp/domain/info_domain.hh \
		src/backend/epp/domain/info_domain_config_data.hh \
		src/backend/epp/domain/transfer_domain_localized.cc \
		src/backend/epp/domain/transfer_domain_localized.hh \
		src/backend/epp/domain/transfer_domain.cc \
		src/backend/epp/domain/transfer_domain.hh \
		src/backend/epp/domain/transfer_domain_config_data.hh \
		src/backend/epp/domain/create_domain_localized.cc \
		src/backend/epp/domain/create_domain_localized.hh \
		src/backend/epp/domain/create_domain_localized_response.hh \
		src/backend/epp/domain/domain_registration_time.hh \
		src/backend/epp/domain/domain_enum_validation.hh \
		src/backend/epp/domain/domain_enum_validation.cc \
		src/backend/epp/domain/create_domain.cc \
		src/backend/epp/domain/create_domain.hh \
		src/backend/epp/domain/create_domain_input_data.hh \
		src/backend/epp/domain/create_domain_config_data.hh \
		src/backend/epp/domain/renew_domain.cc \
		src/backend/epp/domain/renew_domain.hh \
		src/backend/epp/domain/renew_domain_localized.cc \
		src/backend/epp/domain/renew_domain_localized.hh \
		src/backend/epp/domain/renew_domain_localized_response.hh \
		src/backend/epp/domain/renew_domain_input_data.hh \
		src/backend/epp/domain/renew_domain_config_data.hh \
		src/backend/epp/domain/impl/domain_billing.cc \
		src/backend/epp/domain/impl/domain_billing.hh \
		src/backend/epp/domain/impl/domain_output.cc \
		src/backend/epp/domain/impl/domain_output.hh \
		src/backend/epp/domain/update_domain_localized.cc \
		src/backend/epp/domain/update_domain_localized.hh \
		src/backend/epp/domain/update_domain.cc \
		src/backend/epp/domain/update_domain.hh \
		src/backend/epp/domain/update_domain_config_data.hh \
		src/backend/epp/domain/domain_registration_obstruction.hh \
		src/backend/epp/domain/update_domain_input_data.hh \
		src/backend/epp/domain/status_value.hh \
		src/backend/epp/keyset/dns_key.cc \
		src/backend/epp/keyset/dns_key.hh \
		src/backend/epp/keyset/ds_record.hh \
		src/backend/epp/keyset/info_keyset_output_data.hh \
		src/backend/epp/keyset/info_keyset.cc \
		src/backend/epp/keyset/info_keyset.hh \
		src/backend/epp/keyset/info_keyset_config_data.hh \
		src/backend/epp/keyset/info_keyset_localized.cc \
		src/backend/epp/keyset/info_keyset_localized.hh \
		src/backend/epp/keyset/info_keyset_localized_response.hh \
		src/backend/epp/keyset/info_keyset_localized_output_data.hh \
		src/backend/epp/keyset/create_keyset.cc \
		src/backend/epp/keyset/create_keyset.hh \
		src/backend/epp/keyset/create_keyset_localized.cc \
		src/backend/epp/keyset/create_keyset_localized.hh \
		src/backend/epp/keyset/create_keyset_localized_response.hh \
		src/backend/epp/keyset/create_keyset_config_data.hh \
		src/backend/epp/keyset/create_keyset_input_data.hh \
		src/backend/epp/keyset/update_keyset.cc \
		src/backend/epp/keyset/update_keyset.hh \
		src/backend/epp/keyset/update_keyset_localized.cc \
		src/backend/epp/keyset/update_keyset_localized.hh \
		src/backend/epp/keyset/update_keyset_config_data.hh \
		src/backend/epp/keyset/update_keyset_input_data.hh \
		src/backend/epp/keyset/impl/limits.hh \
		src/backend/epp/keyset/impl/keyset_output.cc \
		src/backend/epp/keyset/impl/keyset_output.hh \
		src/backend/epp/keyset/check_keyset.cc \
		src/backend/epp/keyset/check_keyset.hh \
		src/backend/epp/keyset/check_keyset_config_data.hh \
		src/backend/epp/keyset/keyset_handle_registration_obstruction.hh \
		src/backend/epp/keyset/check_keyset_localized.cc \
		src/backend/epp/keyset/check_keyset_localized.hh \
		src/backend/epp/keyset/check_keyset_localized_response.hh \
		src/backend/epp/keyset/delete_keyset.cc \
		src/backend/epp/keyset/delete_keyset.hh \
		src/backend/epp/keyset/delete_keyset_localized.cc \
		src/backend/epp/keyset/delete_keyset_localized.hh \
		src/backend/epp/keyset/delete_keyset_config_data.hh \
		src/backend/epp/keyset/transfer_keyset.cc \
		src/backend/epp/keyset/transfer_keyset.hh \
		src/backend/epp/keyset/transfer_keyset_localized.cc \
		src/backend/epp/keyset/transfer_keyset_localized.hh \
		src/backend/epp/keyset/transfer_keyset_config_data.hh \
		src/backend/epp/keyset/status_value.hh \
		src/backend/epp/nsset/check_nsset_localized.cc \
		src/backend/epp/nsset/check_nsset_localized.hh \
		src/backend/epp/nsset/check_nsset_localized_response.hh \
		src/backend/epp/nsset/check_nsset.cc \
		src/backend/epp/nsset/check_nsset.hh \
		src/backend/epp/nsset/check_nsset_config_data.hh \
		src/backend/epp/nsset/nsset_handle_registration_obstruction.hh \
		src/backend/epp/nsset/nsset_handle_registration_obstruction_localized.hh \
		src/backend/epp/nsset/info_nsset_localized.cc \
		src/backend/epp/nsset/info_nsset_localized.hh \
		src/backend/epp/nsset/info_nsset_localized_response.hh \
		src/backend/epp/nsset/info_nsset_localized_output_data.hh \
		src/backend/epp/nsset/info_nsset.cc \
		src/backend/epp/nsset/info_nsset.hh \
		src/backend/epp/nsset/info_nsset_config_data.hh \
		src/backend/epp/nsset/delete_nsset_localized.cc \
		src/backend/epp/nsset/delete_nsset_localized.hh \
		src/backend/epp/nsset/delete_nsset.cc \
		src/backend/epp/nsset/delete_nsset.hh \
		src/backend/epp/nsset/delete_nsset_config_data.hh \
		src/backend/epp/nsset/create_nsset_localized.cc \
		src/backend/epp/nsset/create_nsset_localized.hh \
		src/backend/epp/nsset/create_nsset_localized_response.hh \
		src/backend/epp/nsset/create_nsset.cc \
		src/backend/epp/nsset/create_nsset.hh \
		src/backend/epp/nsset/create_nsset_config_data.hh \
		src/backend/epp/nsset/create_nsset_input_data.hh \
		src/backend/epp/nsset/update_nsset_localized.cc \
		src/backend/epp/nsset/update_nsset_localized.hh \
		src/backend/epp/nsset/update_nsset.cc \
		src/backend/epp/nsset/update_nsset.hh \
		src/backend/epp/nsset/update_nsset_config_data.hh \
		src/backend/epp/nsset/update_nsset_input_data.hh \
		src/backend/epp/nsset/transfer_nsset_localized.cc \
		src/backend/epp/nsset/transfer_nsset_localized.hh \
		src/backend/epp/nsset/transfer_nsset.cc \
		src/backend/epp/nsset/transfer_nsset.hh \
		src/backend/epp/nsset/transfer_nsset_config_data.hh \
		src/backend/epp/nsset/impl/nsset.cc \
		src/backend/epp/nsset/impl/nsset.hh \
		src/backend/epp/nsset/impl/nsset_output.cc \
		src/backend/epp/nsset/impl/nsset_output.hh \
		src/backend/epp/nsset/status_value.hh \
		src/backend/public_request/public_request.cc \
		src/backend/public_request/public_request.hh \
		src/backend/public_request/type/get_iface_of.hh \
		src/backend/public_request/type/impl/public_request_authinfo.cc \
		src/backend/public_request/type/public_request_authinfo.hh \
		src/backend/public_request/type/impl/public_request_blockunblock.cc \
		src/backend/public_request/type/public_request_blockunblock.hh \
		src/backend/public_request/type/impl/public_request_personalinfo.cc \
		src/backend/public_request/type/public_request_personalinfo.hh \
		src/backend/public_request/type/impl/implemented_by.hh \
		src/backend/public_request/process_public_request_authinfo.cc \
		src/backend/public_request/process_public_request_authinfo.hh \
		src/backend/buffer.cc \
		src/backend/buffer.hh
#
# src/bin/corba/epp/libepp_impl.a


# src/bin/corba/log/liblog_impl.a
#
src_bin_corba_log_liblog_impl_a_SOURCES = \
		src/bin/corba/log/log_impl_wrap.hh \
		src/bin/corba/log/log_impl_wrap.cc
#
# src/bin/corba/log/liblog_impl.a



# src/bin/corba/fred-rifd
#
src_bin_corba_fred_rifd_SOURCES = \
		src/bin/corba/fred-rifd.cc \
		src/util/corba_conversion.hh \
		src/util/corba_conversion.cc

src_bin_corba_fred_rifd_LDADD = \
		src/bin/corba/epp/libepp_impl.a \
		src/util/libpassword_storage.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/backend/admin/contact/verification/libadmin_contact_verification.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbatechcheck.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorba.a \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/libfred.a \
		src/libfred/messages/libmessages.a \
		src/deprecated/model/libmodel.a \
		src/util/tz/libtimezones.a \
		src/util/db/libdb.a \
		src/util/types/libtypes.a \
		src/util/log/liblog.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/fred-rifd


# src/bin/corba/fred-adifd
#
src_bin_corba_fred_adifd_SOURCES = \
		src/bin/corba/admin_contact_verification/server_i.hh \
		src/bin/corba/admin_contact_verification/server_i.cc \
		src/libfred/registrable_object/contact/util.hh \
		src/libfred/registrable_object/contact/util.cc \
		src/bin/corba/fred-adifd.cc \
		src/backend/mojeid/mojeid_public_request.cc \
		src/bin/corba/admin/public_request_mojeid.hh \
		src/bin/corba/admin/public_request_mojeid.cc \
		src/backend/admin_block/administrativeblocking.cc \
		src/backend/admin_block/administrativeblocking.hh \
		src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc \
		src/libfred/registrable_object/domain/create_domain_name_blacklist_id.hh \
		src/libfred/registrable_object/domain/update_domain.cc \
		src/libfred/registrable_object/domain/update_domain.hh \
		src/libfred/registrable_object/contact/copy_contact.cc \
		src/libfred/registrable_object/contact/copy_contact.hh \
		src/libfred/registrable_object/contact/info_contact_data.hh \
		src/libfred/registrable_object/contact/info_contact_data.cc \
		src/libfred/registrable_object/contact/info_contact.hh \
		src/libfred/registrable_object/contact/info_contact.cc \
		src/libfred/registrable_object/contact/create_contact.hh \
		src/libfred/registrable_object/contact/create_contact.cc \
		src/libfred/registrable_object/contact/place_address.cc \
		src/libfred/registrable_object/contact/place_address.hh \
		src/backend/mojeid/checkers.cc \
		src/backend/mojeid/checkers.hh \
		src/backend/mojeid/check_collector.hh \
		src/libfred/opcontext.cc \
		src/libfred/opcontext.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/mailer_manager.cc \
		src/bin/corba/admin_block/corba_conversion.hh \
		src/bin/corba/admin_block/server_i.cc \
		src/bin/corba/admin_block/server_i.hh \
		src/backend/admin/contact/verification/create_test_impl_prototypes.hh \
		src/backend/admin/contact/verification/create_test_impl_prototypes.cc \
		src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh \
		src/backend/admin/contact/verification/delete_domains_of_invalid_contact.cc \
		src/backend/admin/contact/verification/enqueue_check.hh \
		src/backend/admin/contact/verification/enqueue_check.cc \
		src/backend/admin/contact/verification/exceptions.hh \
		src/backend/admin/contact/verification/fill_check_queue.hh \
		src/backend/admin/contact/verification/fill_check_queue.cc \
		src/backend/admin/contact/verification/list_active_checks.hh \
		src/backend/admin/contact/verification/list_active_checks.cc \
		src/backend/admin/contact/verification/related_records.hh \
		src/backend/admin/contact/verification/related_records.cc \
		src/backend/admin/contact/verification/resolve_check.hh \
		src/backend/admin/contact/verification/resolve_check.cc \
		src/backend/admin/contact/verification/run_all_enqueued_checks.hh \
		src/backend/admin/contact/verification/run_all_enqueued_checks.cc \
		src/backend/admin/contact/verification/update_tests.hh \
		src/backend/admin/contact/verification/update_tests.cc \
		src/backend/admin/contact/verification/contact_states/enum.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.hh \
		src/backend/admin/contact/verification/contact_states/delete_all.cc \
		src/backend/admin/contact/verification/test_impl/test_contactability.hh \
		src/backend/admin/contact/verification/test_impl/test_contactability.cc \
		src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh \
		src/backend/admin/contact/verification/test_impl/test_cz_address_exists.cc \
		src/backend/admin/contact/verification/test_impl/test_email_exists.hh \
		src/backend/admin/contact/verification/test_impl/test_email_exists.cc \
		src/backend/admin/contact/verification/test_impl/test_email_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_email_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_interface.hh \
		src/backend/admin/contact/verification/test_impl/test_interface.cc \
		src/backend/admin/contact/verification/test_impl/test_name_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_name_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh \
		src/backend/admin/contact/verification/test_impl/test_phone_syntax.cc \
		src/backend/admin/contact/verification/test_impl/test_send_letter.hh \
		src/backend/admin/contact/verification/test_impl/test_send_letter.cc \
		src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh \
		src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.cc \
		src/bin/corba/notification/server_i.hh \
		src/bin/corba/notification/server_i.cc \
		src/bin/corba/notification/notification_corba_conversion.hh \
		src/bin/corba/notification/notification_corba_conversion.cc \
		src/backend/admin/notification/notification.hh \
		src/backend/admin/notification/notification.cc \
		src/backend/admin/domain/create_expired_domain.hh \
		src/backend/admin/domain/create_expired_domain.cc \
		src/util/corba_conversion.hh \
		src/util/corba_conversion.cc

src_bin_corba_fred_adifd_LDADD = \
		src/bin/corba/admin/libadmin_impl.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbanotification.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbazone.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbanullabletypes.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbaadmincontactverification.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libcorbaadministrativeblocking.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/libfred/requests/librequests.a \
		src/libfred/contact_verification/libcontact_verification.a \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/messages/libmessages.a \
		src/libfred/banking/libbanking.a \
		src/libfred/libfred.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/deprecated/model/libmodel.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		src/util/types/libtypes.a \
		src/util/log/liblog.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		-lmpdec -lm -lrt -lresolv \
		$(LIBCRYPTO_LIBS) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/fred-adifd


# src/bin/corba/fred-pifd
#
src_bin_corba_fred_pifd_SOURCES = \
		src/bin/corba/contact_verification/contact_verification_i.cc \
		src/bin/corba/contact_verification/contact_verification_i.hh \
		src/backend/mojeid/mojeid_public_request.cc \
		src/backend/contact_verification/contact_verification_impl.cc \
		src/backend/contact_verification/contact_verification_impl.hh \
		src/backend/contact_verification/public_request_contact_verification_wrapper.hh \
		src/backend/buffer.cc \
		src/backend/buffer.hh \
		src/backend/public_request/public_request.cc \
		src/backend/public_request/public_request.hh \
		src/backend/public_request/type/get_iface_of.hh \
		src/backend/public_request/type/impl/public_request_authinfo.cc \
		src/backend/public_request/type/public_request_authinfo.hh \
		src/backend/public_request/type/impl/public_request_blockunblock.cc \
		src/backend/public_request/type/public_request_blockunblock.hh \
		src/backend/public_request/type/impl/public_request_personalinfo.cc \
		src/backend/public_request/type/public_request_personalinfo.hh \
		src/backend/public_request/type/impl/implemented_by.hh \
		src/backend/public_request/send_email.cc \
		src/backend/public_request/send_email.hh \
		src/bin/corba/public_request/server_i.cc \
		src/bin/corba/public_request/server_i.hh \
		src/bin/corba/util/corba_conversions_buffer.cc \
		src/bin/corba/util/corba_conversions_buffer.hh \
		src/util/corba_conversion.hh \
		src/util/corba_conversion.cc \
		src/bin/corba/admin/common.hh \
		src/bin/corba/admin/common.cc \
		src/bin/corba/fred-pifd.cc

src_bin_corba_fred_pifd_LDADD = \
		src/bin/corba/whois/libwhois_impl.a \
		src/bin/corba/whois/libwhois2_impl.a \
		src/bin/corba/libcorbawhois.a \
		src/bin/corba/libcorbawhois2.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorbanullabletypes.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbanullableisodatetime.a \
		src/libfred/contact_verification/libcontact_verification.a \
		src/bin/corba/libcorbacontactverification.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbapublicrequest.a \
		src/libfred/requests/librequests.a \
		src/libfred/banking/libbanking.a \
		src/libfred/messages/libmessages.a \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/libfred.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/deprecated/model/libmodel.a \
		src/util/db/libdb.a \
		src/util/types/libtypes.a \
		src/deprecated/util/liboldutils.a \
		src/util/log/liblog.a \
		src/util/libutil.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/fred-pifd


# src/bin/corba/fred-logd
#
src_bin_corba_fred_logd_SOURCES = \
		src/bin/corba/fred-logd.cc \
		src/bin/corba/admin/common.cc \
		src/bin/corba/admin/filter_impl.cc \
		src/bin/corba/admin/pagetable_impl.cc \
		src/bin/corba/admin/pagetable_logger.cc

src_bin_corba_fred_logd_LDADD = \
		src/bin/corba/log/liblog_impl.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorba.a \
		src/libfred/libfred.a \
		src/libfred/requests/librequests.a \
		src/deprecated/model/libmodel.a \
		src/util/db/libdb.a \
		src/util/types/libtypes.a \
		src/util/log/liblog.a \
		src/util/libutil.a \
		src/deprecated/util/liboldutils.a \
		-lmpdec -lm -lrt \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/fred-logd


# src/bin/corba/messages/fred-msgd
#
src_bin_corba_messages_fred_msgd_SOURCES = \
		src/bin/corba/messages/fred-msgd.cc \
		src/bin/corba/messages/messages_filemanager.cc \
		src/bin/corba/messages/messages_filemanager.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/messages/messages.cc \
		src/bin/corba/messages/messages.hh \
		src/bin/corba/file_manager_client.cc

nodist_src_bin_corba_messages_fred_msgd_SOURCES = \
		src/bin/corba/MessagesSK.cc \
		src/bin/corba/FileManagerSK.cc

src_bin_corba_messages_fred_msgd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_bin_corba_messages_fred_msgd_LDADD = \
		src/libfred/messages/libmessages.a \
		src/deprecated/model/libmodel.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		src/bin/corba/libcorbadatatypes.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/messages/fred-msgd


# src/bin/corba/mojeid/fred-mifd
#
src_bin_corba_mojeid_fred_mifd_SOURCES = \
		src/backend/buffer.cc \
		src/backend/mojeid/mojeid.cc \
		src/backend/mojeid/mojeid_impl_data_conversion.cc \
		src/backend/mojeid/mojeid_impl_internal.cc \
		src/backend/mojeid/mojeid_public_request.cc \
		src/backend/mojeid/messages/generate.cc \
		src/bin/corba/nameservice.cc \
		src/bin/corba/mojeid/server_i.cc \
		src/util/corba_conversion.cc \
		src/bin/corba/util/corba_conversions_isodatetime.cc \
		src/bin/corba/mojeid/mojeid_corba_conversion.cc \
		src/bin/corba/mojeid/service_name.cc \
		src/bin/corba/mojeid/fred-mifd.cc

nodist_src_bin_corba_mojeid_fred_mifd_SOURCES = \
		src/bin/corba/MojeIDSK.cc \
		src/bin/corba/MailerSK.cc

src_bin_corba_mojeid_fred_mifd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_bin_corba_mojeid_fred_mifd_LDADD = \
		src/bin/corba/libcorbadatatypes.a \
		src/libfred/libfred.a \
		src/libfred/messages/libmessages.a \
		src/deprecated/model/libmodel.a \
		src/libfred/contact_verification/libcontact_verification.a \
		src/libfred/public_request/libpublic_request.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbanullableisodatetime.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorba.a \
		-lmpdec -lm -lrt \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/mojeid/fred-mifd

# src/bin/corba/mojeid/messages/fred-mojeid-msggen
#
src_bin_corba_mojeid_messages_fred_mojeid_msggen_SOURCES = \
		src/bin/corba/mojeid/service_name.cc \
		src/bin/corba/mojeid/messages/client.cc

nodist_src_bin_corba_mojeid_messages_fred_mojeid_msggen_SOURCES = \
		src/bin/corba/MojeIDSK.cc

src_bin_corba_mojeid_messages_fred_mojeid_msggen_LDFLAGS = \
		$(AM_LDFLAGS)

src_bin_corba_mojeid_messages_fred_mojeid_msggen_LDADD = \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB)
#
# src/bin/corba/mojeid/messages/fred-mojeid-msggen

#
# src/bin/corba/domain_browser/fred-dbifd
src_bin_corba_domain_browser_fred_dbifd_SOURCES = \
		src/util/cfg/handle_domainbrowser_args.hh \
		src/backend/domain_browser/domain_browser.cc \
		src/backend/domain_browser/domain_browser.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/domain_browser/domain_browser_i.cc \
		src/bin/corba/domain_browser/domain_browser_i.hh \
		src/bin/corba/domain_browser/fred-dbifd.cc

src_bin_corba_domain_browser_fred_dbifd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_bin_corba_domain_browser_fred_dbifd_LDADD = \
		src/bin/corba/libcorbadomainbrowser.a \
		src/libfred/libfred.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		-lmpdec -lm -lrt \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/domain_browser/fred-dbifd


# src/bin/corba/automatic_keyset_management/fred-akmd
#
src_bin_corba_automatic_keyset_management_fred_akmd_SOURCES = \
		src/backend/epp/keyset/dns_key.cc \
		src/backend/epp/keyset/dns_key.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/nameservice.hh \
		src/bin/corba/logger_client_impl.hh \
		src/bin/corba/logger_client_impl.cc \
		src/util/dummy_logger.hh \
		src/util/dummy_logger.cc \
		src/backend/automatic_keyset_management/automatic_keyset_management.cc \
		src/backend/automatic_keyset_management/automatic_keyset_management.hh \
		src/backend/automatic_keyset_management/keyset.hh \
		src/backend/automatic_keyset_management/nsset.hh \
		src/backend/automatic_keyset_management/dns_key.cc \
		src/backend/automatic_keyset_management/dns_key.hh \
		src/backend/automatic_keyset_management/impl/limits.hh \
		src/backend/automatic_keyset_management/impl/logger_request.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_rollover.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_turn_off.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_turn_on.hh \
		src/backend/automatic_keyset_management/impl/logger_request_data.cc \
		src/backend/automatic_keyset_management/impl/logger_request_data.hh \
		src/backend/automatic_keyset_management/impl/logger_request_object_type.cc \
		src/backend/automatic_keyset_management/impl/logger_request_object_type.hh \
		src/backend/automatic_keyset_management/impl/logger_request_property.cc \
		src/backend/automatic_keyset_management/impl/logger_request_property.hh \
		src/backend/automatic_keyset_management/impl/logger_request_result.hh \
		src/backend/automatic_keyset_management/impl/logger_request_type.cc \
		src/backend/automatic_keyset_management/impl/logger_request_type.hh \
		src/backend/automatic_keyset_management/impl/logger_service_type.hh \
		src/backend/automatic_keyset_management/impl/util.cc \
		src/backend/automatic_keyset_management/impl/util.hh \
		src/bin/corba/automatic_keyset_management/server_i.cc \
		src/bin/corba/automatic_keyset_management/server_i.hh \
		src/bin/corba/automatic_keyset_management/fred-akmd.cc


src_bin_corba_automatic_keyset_management_fred_akmd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_bin_corba_automatic_keyset_management_fred_akmd_LDADD = \
		src/bin/corba/libcorbaautomatickeysetmanagement.a \
		src/libfred/libfred.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		-lmpdec -lm -lrt \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/automatic_keyset_management/fred-akmd


# src/bin/corba/record_statement/fred-rsifd
#
src_bin_corba_record_statement_fred_rsifd_SOURCES = \
		src/backend/buffer.cc \
		src/backend/buffer.hh \
		src/backend/record_statement/record_statement.cc \
		src/backend/record_statement/record_statement.hh \
		src/backend/record_statement/purpose.hh \
		src/backend/record_statement/exceptions.cc \
		src/backend/record_statement/exceptions.hh \
		src/backend/record_statement/impl/record_statement_xml.cc \
		src/backend/record_statement/impl/record_statement_xml.hh \
		src/backend/record_statement/impl/templates_impl.hh \
		src/backend/record_statement/impl/factory.cc \
		src/backend/record_statement/impl/factory.hh \
		src/backend/record_statement/impl/utc_impl.cc \
		src/backend/record_statement/impl/europe_prague_impl.cc \
		src/backend/record_statement/impl/util.cc \
		src/backend/record_statement/impl/util.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/record_statement/record_statement_i.cc \
		src/bin/corba/record_statement/record_statement_i.hh \
		src/bin/corba/record_statement/fred-rsifd.cc

src_bin_corba_record_statement_fred_rsifd_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

src_bin_corba_record_statement_fred_rsifd_LDADD = \
		src/libfred/libfred.a \
		src/bin/corba/util/libcorbautil.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		-lmpdec -lm -lrt \
		src/bin/corba/libcorba.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbarecordstatement.a \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# src/bin/corba/record_statement/fred-rsifd


# src/cli_admin/fred-admin
#
src_bin_cli_fred_admin_SOURCES = \
		src/bin/cli/fred-admin.cc \
		src/bin/cli/handle_adminclientselection_args.hh \
		src/bin/cli/baseclient.hh \
		src/bin/cli/commonclient.cc \
		src/bin/cli/commonclient.hh \
		src/bin/cli/domainclient.cc \
		src/bin/cli/domainclient.hh \
		src/bin/cli/domain_params.hh \
		src/bin/cli/domain_client_impl.hh \
		src/bin/cli/keysetclient.cc \
		src/bin/cli/keysetclient.hh \
		src/bin/cli/keyset_params.hh \
		src/bin/cli/keyset_client_impl.hh \
		src/bin/cli/contactclient.cc \
		src/bin/cli/contactclient.hh \
		src/bin/cli/contact_params.hh \
		src/bin/cli/public_request_method.hh \
		src/bin/cli/public_request_method.cc \
		src/bin/cli/contact_client_impl.hh \
		src/bin/cli/invoiceclient.cc \
		src/bin/cli/invoiceclient.hh \
		src/bin/cli/invoice_params.hh \
		src/bin/cli/invoice_client_impl.hh \
		src/bin/cli/bankclient.cc \
		src/bin/cli/bankclient.hh \
		src/bin/cli/bank_params.hh \
		src/bin/cli/bank_client_impl.hh \
		src/bin/cli/pollclient.cc \
		src/bin/cli/pollclient.hh \
		src/bin/cli/poll_params.hh \
		src/bin/cli/poll_client_impl.hh \
		src/bin/cli/registrarclient.cc \
		src/bin/cli/registrarclient.hh \
		src/bin/cli/registrar_params.hh \
		src/bin/cli/registrar_client_impl.hh \
		src/bin/cli/notifyclient.cc \
		src/bin/cli/notifyclient.hh \
		src/bin/cli/read_config_file.hh \
		src/bin/cli/optys_get_undelivered.cc \
		src/bin/cli/optys_get_undelivered.hh \
		src/bin/cli/notify_params.hh \
		src/bin/cli/notify_client_impl.hh \
		src/bin/cli/enumparamclient.cc \
		src/bin/cli/enumparamclient.hh \
		src/bin/cli/enumparam_params.hh \
		src/bin/cli/enumparam_client_impl.hh \
		src/bin/cli/objectclient.hh \
		src/bin/cli/objectclient.cc \
		src/bin/cli/object_params.hh \
		src/bin/cli/object_client_impl.hh \
		src/bin/cli/fileclient.hh \
		src/bin/cli/fileclient.cc \
		src/bin/cli/file_params.hh \
		src/bin/cli/file_client_impl.hh \
		src/bin/cli/regblock_params.hh \
		src/bin/cli/regblock_client.hh \
		src/bin/cli/regblock_client_impl.hh \
		src/bin/cli/charge_params.hh \
		src/bin/cli/chargeclient.cc \
		src/bin/cli/chargeclient.hh \
		src/bin/cli/charge_client_impl.hh \
		src/bin/cli/remove_delete_candidates.cc \
		src/bin/cli/remove_delete_candidates.hh \
		src/backend/admin/contact/merge_contact.cc \
		src/backend/admin/contact/merge_contact.hh \
		src/backend/admin/contact/merge_contact_logger.cc \
		src/backend/admin/contact/merge_contact_logger.hh \
		src/backend/admin/contact/merge_contact_auto_procedure.cc \
		src/backend/admin/contact/merge_contact_auto_procedure.hh \
		src/backend/admin/contact/merge_contact_reporting.cc \
		src/backend/admin/contact/merge_contact_reporting.hh \
		src/bin/cli/domain_name_validation_params.hh \
		src/bin/cli/domain_name_validation_init.hh \
		src/libfred/object_state/create_object_state_request_id.cc \
		src/libfred/object_state/create_object_state_request_id.hh \
		src/libfred/object_state/get_object_state_id_map.cc \
		src/libfred/object_state/get_object_state_id_map.hh \
		src/libfred/object_state/cancel_object_state_request_id.cc \
		src/libfred/object_state/cancel_object_state_request_id.hh \
		src/util/hp/handle_hpmail_args.hh \
		src/util/optys/upload_client.cc \
		src/util/optys/upload_client.hh \
		src/util/optys/download_client.hh \
		src/util/optys/download_client.cc \
		src/backend/admin/notification/notification.hh \
		src/backend/admin/notification/notification.cc \
		src/backend/admin/domain/create_expired_domain.hh \
		src/backend/admin/domain/create_expired_domain.cc \
		src/backend/public_request/send_email.cc \
		src/backend/public_request/send_email.hh \
		src/backend/public_request/process_public_requests.cc \
		src/backend/public_request/process_public_requests.hh \
		src/backend/public_request/process_public_request_authinfo.cc \
		src/backend/public_request/process_public_request_authinfo.hh \
		src/backend/buffer.cc \
		src/backend/buffer.hh \
		src/backend/public_request/public_request.cc \
		src/backend/public_request/type/get_iface_of.hh \
		src/backend/public_request/type/impl/public_request_authinfo.cc \
		src/backend/public_request/type/public_request_authinfo.hh \
		src/backend/public_request/type/impl/public_request_blockunblock.cc \
		src/backend/public_request/type/public_request_blockunblock.hh \
		src/backend/public_request/type/impl/public_request_personalinfo.cc \
		src/backend/public_request/type/public_request_personalinfo.hh \
		src/backend/public_request/type/impl/implemented_by.hh \
		src/bin/cli/public_request_params.hh \
		src/libfred/public_request/public_request_on_status_action.hh \
		src/bin/cli/public_request_impl.hh \
		src/util/csv/csv.hh


src_bin_cli_fred_admin_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(XML2_LIBS)	# using xpath in contact verification test for czech address

src_bin_cli_fred_admin_LDADD = \
		src/backend/admin/contact/verification/libadmin_contact_verification.a \
		src/libfred/libfred.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/deprecated/model/libmodel.a \
		src/libfred/banking/libbanking.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libfilemanager.a \
		src/libfred/messages/libmessages.a \
		src/libfred/requests/librequests.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		src/util/hp/libhp.a \
		src/libfred/public_request/libpublic_request.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(OMNIORB4_LIBS) \
		$(XML2_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		-lresolv
#
# src/cli_admin/fred-admin

# test/test-delete-objects-marked-as-delete-candidate
#
test_test_delete_objects_marked_as_delete_candidate_SOURCES = \
		test/test_delete_objects_marked_as_delete_candidate.cc

test_test_delete_objects_marked_as_delete_candidate_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS)

test_test_delete_objects_marked_as_delete_candidate_LDADD = \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		-lmpdec -lm -lrt \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(LIBIDN_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		-lresolv
#
# test/test-delete-objects-marked-as-delete-candidate

# tools/disclose_flags_updater/fred-disclose-flag-updater
#
tools_disclose_flags_updater_fred_disclose_flags_updater_SOURCES = \
		tools/disclose_flags_updater/options.hh \
		tools/disclose_flags_updater/thread_safe_output.cc \
		tools/disclose_flags_updater/thread_safe_output.hh \
		tools/disclose_flags_updater/worker.hh \
		tools/disclose_flags_updater/worker.cc \
		tools/disclose_flags_updater/disclose_value.hh \
		tools/disclose_flags_updater/disclose_value.cc \
		tools/disclose_flags_updater/disclose_settings.hh \
		tools/disclose_flags_updater/disclose_settings.cc \
		tools/disclose_flags_updater/contact_search_query.hh \
		tools/disclose_flags_updater/contact_search_query.cc \
		tools/disclose_flags_updater/fred_disclose_flags_updater.cc

tools_disclose_flags_updater_fred_disclose_flags_updater_CPPFLAGS = \
		$(AM_CPPFLAGS)

tools_disclose_flags_updater_fred_disclose_flags_updater_LDFLAGS = \
		$(AM_LDFLAGS)

tools_disclose_flags_updater_fred_disclose_flags_updater_LDADD = \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_SYSTEM_LIB)

#
# tools/disclose_flags_updater/fred-disclose-flag-updater


# tools/logd_migration
#
tools_logd_migration_migrate_log_SOURCES = \
		tools/logd_migration/migrate.cc \
		tools/logd_migration/log_if.cc \
		tools/logd_migration/m_epp_parser.cc \
		tools/logd_migration/migrate.hh \
		tools/logd_migration/m_epp_parser.hh

tools_logd_migration_migrate_log_CPPFLAGS = \
		$(AM_CPPFLAGS)

tools_logd_migration_migrate_log_LDFLAGS = \
		$(AM_LDFLAGS)

tools_logd_migration_migrate_log_LDADD = \
		src/libfred/requests/librequests.a \
		src/libfred/libfred.a \
		src/deprecated/model/libmodel.a \
		src/libfred/banking/libbanking.a \
		src/bin/corba/log/liblog_impl.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/deprecated/util/liboldutils.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(XML2_LIBS) \
		$(LIBIDN_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# tools/logd_migration


# tools/contact_migration
#
tools_contact_migration_mojeid_contact_address_import_SOURCES = \
		tools/contact_migration/mojeid_contact_address_import.cc

tools_contact_migration_mojeid_contact_address_import_CPPFLAGS = \
		$(AM_CPPFLAGS)

tools_contact_migration_mojeid_contact_address_import_LDFLAGS = \
		$(AM_LDFLAGS)

tools_contact_migration_mojeid_contact_address_import_LDADD = \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(LIBIDN_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_SYSTEM_LIB)


tools_contact_migration_mojeid_contact_ssn_fix_SOURCES = \
		tools/contact_migration/mojeid_contact_ssn_fix.cc

tools_contact_migration_mojeid_contact_ssn_fix_CPPFLAGS = \
		$(AM_CPPFLAGS)

tools_contact_migration_mojeid_contact_ssn_fix_LDFLAGS = \
		$(AM_LDFLAGS)

tools_contact_migration_mojeid_contact_ssn_fix_LDADD = \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_THREAD_LIB) \
		$(LIBIDN_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# tools/contact_migration


# tests
#
SERVER_TESTS = \
		test/test-server \
		test/backend/test-admin-contact-verification \
		test/backend/test-admin-contact-automerge \
		test/backend/test-admin-notification \
		test/backend/test-admin-domain \
		test/backend/test-epp \
		test/backend/test-whois \
		test/backend/test-public-request \
		test/backend/test-automatic-keyset-management \
		test/test-exception \
		test/test-blocking \
		test/test-libfred \
		test/test-optys \
		test/test-corba-conversion \
		test/backend/test-record-statement \
		test/test-password-storage \
		test/test-delete-objects-marked-as-delete-candidate

# test/backend/test-admin-contact-verification
#
test_backend_test_admin_contact_verification_SOURCES = \
		test/backend/admin/contact/verification/main.cc \
		test/backend/admin/contact/verification/setup_utils.hh \
		test/libfred/contact/verification/setup_utils.hh \
		test/libfred/contact/verification/setup_utils.cc \
		test/backend/admin/contact/verification/test_run_all_enqueued_checks.cc \
		test/backend/admin/contact/verification/test_fill_check_queue.cc \
		test/backend/admin/contact/verification/test_resolve_check.cc \
		test/backend/admin/contact/verification/test_related_records.cc \
		test/backend/admin/contact/verification/test_enqueue_check.cc \
		test/backend/admin/contact/verification/test_delete_domains_of_invalid_contact.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_backend_test_admin_contact_verification_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_admin_contact_verification_LDADD = \
		src/backend/admin/contact/verification/libadmin_contact_verification.a \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/backend/test-admin-contact-verification

# test/backend/test-admin-contact-automerge
#
test_backend_test_admin_contact_automerge_SOURCES = \
		src/libfred/object/object_states_info.hh \
		test/backend/admin/contact/automerge/main.cc \
		test/backend/admin/contact/automerge/test-merge-auto-proc.cc \
		src/backend/admin/contact/merge_contact.cc \
		src/backend/admin/contact/merge_contact.hh \
		src/backend/admin/contact/merge_contact_logger.cc \
		src/backend/admin/contact/merge_contact_logger.hh \
		src/backend/admin/contact/merge_contact_auto_procedure.cc \
		src/backend/admin/contact/merge_contact_auto_procedure.hh \
		src/backend/admin/contact/merge_contact_reporting.cc \
		src/backend/admin/contact/merge_contact_reporting.hh \
		test/libfred/contact/util.hh \
		test/libfred/contact/fixture.hh \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_backend_test_admin_contact_automerge_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_admin_contact_automerge_LDADD = \
		src/bin/corba/libcorbadatatypes.a \
		src/libfred/libfred.a \
		src/deprecated/model/libmodel.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libfilemanager.a \
		src/libfred/requests/librequests.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/backend/test-admin-contact-automerge

# test/backend/test-admin-notification
#
test_backend_test_admin_notification_SOURCES = \
		test/backend/admin/notification/fixtures.hh \
		test/backend/admin/notification/main.cc \
		test/backend/admin/notification/test_set_domain_outzone_unguarded_warning_emails.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_backend_test_admin_notification_CPPFLAGS = \
		$(AM_CPPFLAGS)

test_backend_test_admin_notification_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_admin_notification_LDADD = \
		src/backend/admin/notification/libadmin_notification.a \
		src/bin/corba/libcorba.a \
		src/libfred/libfred.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
		$(LIBIDN_LIBS)
#
# test/backend/test-admin-notification

# test/backend/test-admin-domain
#
test_backend_test_admin_domain_SOURCES = \
		test/backend/admin/domain/fixtures.hh \
		test/backend/admin/domain/main.cc \
		test/backend/admin/domain/test_create_expired_domain.cc \
		src/backend/admin/domain/create_expired_domain.hh \
		src/backend/admin/domain/create_expired_domain.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc

test_backend_test_admin_domain_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_admin_domain_LDADD = \
		src/backend/admin/domain/libadmin_create_expired_domain.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libfilemanager.a \
		src/libfred/libfred.a \
		src/libfred/requests/librequests.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/util/types/libtypes.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
		$(LIBIDN_LIBS)
#
# test/backend/test-admin-domain

# test/backend/test-epp
#
test_backend_test_epp_SOURCES = \
		test/backend/epp/util.hh \
		test/backend/epp/fixture.hh \
		test/backend/epp/main.cc \
		test/backend/epp/test_conditionally_enqueue_notification.cc \
		test/backend/epp/contact/fixture.hh \
		test/backend/epp/contact/test_create_contact.cc \
		test/backend/epp/contact/test_delete_contact.cc \
		test/backend/epp/contact/test_info_contact.cc \
		test/backend/epp/contact/test_update_contact.cc \
		test/backend/epp/contact/test_check_contact.cc \
		test/backend/epp/contact/test_transfer_contact.cc \
		test/backend/epp/contact/test_contact_handle_state_to_check_result.cc \
		test/backend/epp/credit/test_client_credit.cc \
		test/backend/epp/domain/fixture.hh \
		test/backend/epp/domain/fixture.cc \
		test/backend/epp/domain/test_check_domain.cc \
		test/backend/epp/domain/test_delete_domain.cc \
		test/backend/epp/domain/test_info_domain.cc \
		test/backend/epp/domain/test_transfer_domain.cc \
		test/backend/epp/domain/test_update_domain.cc \
		test/backend/epp/domain/test_create_domain.cc \
		test/backend/epp/domain/test_renew_domain.cc \
		test/backend/epp/domain/test_domain_billing.cc \
		test/backend/epp/keyset/fixture.hh \
		test/backend/epp/keyset/fixture.cc \
		test/backend/epp/keyset/test_create_keyset.cc \
		test/backend/epp/nsset/test_check_nsset.cc \
		test/backend/epp/nsset/test_info_nsset.cc \
		test/backend/epp/nsset/test_delete_nsset.cc \
		test/backend/epp/nsset/test_create_nsset.cc \
		test/backend/epp/nsset/test_update_nsset.cc \
		test/backend/epp/nsset/test_transfer_nsset.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/backend/epp/poll/fixture.hh \
		test/backend/epp/poll/fixture.cc \
		test/backend/epp/poll/test_poll_request.cc \
		test/backend/epp/poll/test_poll_acknowledgement.cc \
		test/backend/epp/poll/test_poll_request_nsset_details.cc \
		test/backend/epp/poll/test_poll_request_domain_details.cc \
		test/backend/epp/poll/test_poll_request_keyset_details.cc \
		test/setup/fixtures.cc

test_backend_test_epp_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_epp_LDADD = \
		src/bin/corba/epp/libepp_impl.a \
		src/libfred/libfred.a \
		src/util/tz/libtimezones.a \
		src/util/libpassword_storage.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/util/log/liblog.a \
		src/util/libutil.a \
		$(LIBCRYPTO_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/backend/test-epp

# test/backend/test-whois
#
test_backend_test_whois_SOURCES = \
		test/backend/whois/main.cc \
		test/backend/whois/fixture_common.hh \
		test/backend/whois/test_whois_get_registrar_by_handle.cc \
		test/backend/whois/test_whois_get_contact_by_handle.cc \
		test/backend/whois/test_whois_get_domain_by_handle.cc \
		test/backend/whois/test_whois_get_domains_by_admin_contact.cc \
		test/backend/whois/test_whois_get_domains_by_keyset.cc \
		test/backend/whois/test_whois_get_domains_by_nsset.cc \
		test/backend/whois/test_whois_get_domains_by_registrant.cc \
		test/backend/whois/test_whois_get_keyset_by_handle.cc \
		test/backend/whois/test_whois_get_keysets_by_tech_c.cc \
		test/backend/whois/test_whois_get_managed_zone_list.cc \
		test/backend/whois/test_whois_get_nameserver_by_fqdn.cc \
		test/backend/whois/test_whois_get_nsset_by_handle.cc \
		test/backend/whois/test_whois_get_nssets_by_ns.cc \
		test/backend/whois/test_whois_get_nssets_by_tech_c.cc \
		test/backend/whois/test_whois_get_registrars.cc \
		test/backend/whois/test_whois_get_registrar_groups.cc \
		test/backend/whois/test_whois_get_registrar_certification_list.cc \
		test/backend/whois/test_whois_get_object_status_descriptions.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_backend_test_whois_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_whois_LDADD = \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/whois/libwhois2_impl.a \
		src/libfred/libfred.a \
		src/deprecated/model/libmodel.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbanullableisodatetime.a \
		src/libfred/requests/librequests.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/db/libdb.a \
		src/deprecated/util/liboldutils.a \
		src/util/libutil.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/backend/test-whois

# test/test-server
#
test_test_server_SOURCES = \
		test/deprecated/test_server.cc \
		test/deprecated/test_server.hh \
		test/deprecated/test_conv.cc \
		test/deprecated/test_conv.hh \
		test/deprecated/test_file_manager_client.cc \
		test/deprecated/test_file_manager_client.hh \
		test/deprecated/test_db.cc \
		test/deprecated/test_registrar_group.cc \
		test/deprecated/test_registrar_group.hh \
		test/deprecated/test_registrar_blocking.cc \
		src/bin/corba/admin/filter_impl.cc \
		src/bin/corba/admin/filter_impl.hh \
		test/deprecated/test_logd.cc \
		test/deprecated/test_msgd.cc \
		test/deprecated/test_msgd.hh \
		test/deprecated/test_admin.cc \
		test/deprecated/test_corbans.cc \
		test/deprecated/test_whois.cc \
		test/deprecated/test_cache.cc \
		test/setup/tests_common.cc \
		test/setup/tests_common.hh \
		test/deprecated/test_filters_date_time.cc \
		test/deprecated/test_invoice_common.hh \
		test/deprecated/test_invoice_common.cc \
		test/deprecated/test_invoice.cc \
		test/deprecated/test_invoice_account.cc \
		test/setup/test_common_threaded.hh \
		test/deprecated/test_common_registry.hh \
		test/deprecated/test_common_registry.cc \
		test/deprecated/test_invoice2.cc \
		test/util/test_decimal.cc \
		test/util/test_shellcmd.cc \
		test/deprecated/test_contact_verification.cc \
		test/deprecated/test_locking_trigger.cc \
		test/util/test_nullable.cc \
		test/util/test_optional.cc \
		src/backend/contact_verification/contact_verification_impl.cc \
		src/backend/contact_verification/contact_verification_impl.hh \
		src/libfred/contact_verification/cancel_contact_verification.cc \
		src/libfred/contact_verification/cancel_contact_verification.hh \
		src/libfred/contact_verification/contact_verification_state.cc \
		test/deprecated/checks.hh \
		test/util/test_printable.cc \
		test/util/test_csv_parser.cc \
		test/util/test_csv_serializer.cc \
		src/util/optys/download_client.cc \
		src/util/optys/download_client.hh \
		src/util/corba_conversion.hh \
		src/util/corba_conversion.cc \
		test/util/test_is_equal_optional_nullable.cc \
		src/backend/admin/contact/merge_contact.cc \
		src/backend/admin/contact/merge_contact.hh \
		src/backend/admin/contact/merge_contact_logger.cc \
		src/backend/admin/contact/merge_contact_logger.hh \
		src/backend/admin/contact/merge_contact_auto_procedure.cc \
		src/backend/admin/contact/merge_contact_auto_procedure.hh \
		src/backend/admin/contact/merge_contact_reporting.cc \
		src/backend/admin/contact/merge_contact_reporting.hh \
		src/libfred/object_state/create_object_state_request_id.cc \
		src/libfred/object_state/create_object_state_request_id.hh \
		src/libfred/object_state/get_object_state_id_map.cc \
		src/libfred/object_state/get_object_state_id_map.hh \
		src/libfred/object_state/cancel_object_state_request_id.cc \
		src/libfred/object_state/cancel_object_state_request_id.hh \
		src/backend/domain_browser/domain_browser.cc \
		src/backend/domain_browser/domain_browser.hh \
		test/backend/domain_browser/test_domain_browser.cc \
		test/setup/fixtures.cc \
		test/setup/fixtures.hh

test_test_server_CPPFLAGS = \
		$(AM_CPPFLAGS)

test_test_server_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_server_LDADD = \
		src/libfred/contact_verification/libcontact_verification.a \
		src/bin/corba/admin/libadmin_impl.a \
		src/bin/corba/epp/libepp_impl.a \
		src/util/libpassword_storage.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/whois/libwhois_impl.a \
		src/libfred/public_request/libpublic_request.a \
		src/backend/admin/contact/verification/libadmin_contact_verification.a \
		src/backend/admin/notification/libadmin_notification.a \
		src/backend/admin/domain/libadmin_create_expired_domain.a \
		src/libfred/libfred.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/libfred/messages/libmessages.a \
		src/deprecated/model/libmodel.a \
		src/util/tz/libtimezones.a \
		src/util/db/libdb.a \
		src/util/types/libtypes.a \
		src/deprecated/util/liboldutils.a \
		src/libfred/requests/librequests.a \
		src/libfred/banking/libbanking.a \
		src/util/log/liblog.a \
		src/util/libutil.a \
		src/bin/corba/libcorbamessages.a \
		src/bin/corba/libcorbamessages.a \
		src/bin/corba/libcorbazone.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbazonegenerator.a \
		src/bin/corba/libcorbaadministrativeblocking.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbatechcheck.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbawhois.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libmailer.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# test/test_server

# test/backend/test_public_request
#
test_backend_test_public_request_SOURCES = \
		test/backend/public_request/main.cc \
		src/backend/buffer.cc \
		src/backend/public_request/public_request.cc \
		src/backend/public_request/type/get_iface_of.hh \
		src/backend/public_request/type/impl/public_request_authinfo.cc \
		src/backend/public_request/type/public_request_authinfo.hh \
		src/backend/public_request/type/impl/public_request_blockunblock.cc \
		src/backend/public_request/type/public_request_blockunblock.hh \
		src/backend/public_request/type/impl/public_request_personalinfo.cc \
		src/backend/public_request/type/public_request_personalinfo.hh \
		src/backend/public_request/type/impl/implemented_by.hh \
		src/backend/public_request/send_email.cc \
		test/backend/public_request/test_registry_email.cc \
		test/backend/public_request/test_non_registry_email.cc \
		test/backend/public_request/test_create_block_unblock_request.cc \
		test/backend/public_request/test_create_public_request_pdf.cc \
		test/backend/public_request/fixture_common.hh \
		test/backend/public_request/fixture_common.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc

test_backend_test_public_request_CPPFLAGS = \
		$(AM_CPPFLAGS)

test_backend_test_public_request_LDFLAGS = \
		$(AM_LDFLAGS)

test_backend_test_public_request_LDADD = \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/libfred.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/libfred/requests/librequests.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/libutil.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_DATE_TIME_LIB)\
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/backend/test_public_request


# test/test_blocking
#
test_test_blocking_SOURCES = \
		test/deprecated/test_server.cc \
		test/deprecated/test_server.hh \
		src/bin/corba/admin/filter_impl.cc \
		src/bin/corba/admin/filter_impl.hh \
		test/setup/tests_common.cc \
		test/setup/tests_common.hh \
		src/backend/contact_verification/contact_verification_impl.cc \
		src/backend/contact_verification/contact_verification_impl.hh \
		src/libfred/contact_verification/cancel_contact_verification.cc \
		src/libfred/contact_verification/cancel_contact_verification.hh \
		src/libfred/contact_verification/contact_verification_state.cc \
		src/libfred/object_state/clear_admin_object_state_request_id.cc \
		src/libfred/object_state/clear_object_state_request_id.cc \
		src/libfred/object_state/create_object_state_request_id.cc \
		src/libfred/object_state/create_admin_object_block_request_id.cc \
		src/libfred/object_state/create_admin_object_state_restore_request_id.cc \
		src/libfred/object_state/cancel_object_state_request_id.cc \
		src/libfred/object_state/get_object_state_id_map.cc \
		src/libfred/object_state/get_blocking_status_desc_list.cc \
		src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc

test_test_blocking_CPPFLAGS = \
		$(AM_CPPFLAGS)

test_test_blocking_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_blocking_LDADD = \
		src/libfred/contact_verification/libcontact_verification.a \
		src/bin/corba/admin/libadmin_impl.a \
		src/bin/corba/epp/libepp_impl.a \
		src/util/libpassword_storage.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/whois/libwhois_impl.a \
		src/libfred/public_request/libpublic_request.a \
		src/libfred/libfred.a \
		src/epp/impl/registraracl/libauthentic_registrar.a \
		src/util/libpassword_storage.a \
		src/libfred/messages/libmessages.a \
		src/deprecated/model/libmodel.a \
		src/util/db/libdb.a \
		src/util/types/libtypes.a \
		src/deprecated/util/liboldutils.a \
		src/libfred/requests/librequests.a \
		src/libfred/banking/libbanking.a \
		src/util/log/liblog.a \
		src/util/libutil.a \
		src/bin/corba/libcorbamessages.a \
		src/bin/corba/libcorbamessages.a \
		src/bin/corba/libcorbazone.a \
		src/bin/corba/libfilemanager.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbazonegenerator.a \
		src/bin/corba/libcorbaadministrativeblocking.a \
		src/bin/corba/libcorbaepp.a \
		src/bin/corba/libcorbatechcheck.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbawhois.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libmailer.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB)
#
# test/test_blocking

# test/test_libfred
#
test_test_libfred_SOURCES = \
		test/libfred/main.cc \
		test/libfred/registrar/test_registrar_zone_access.cc \
		test/libfred/registrar/test_get_registrar_zone_credit.cc \
		test/libfred/registrar/test_info_registrar.cc \
		test/libfred/registrar/test_create_registrar.cc \
		test/libfred/registrar/group/test_create.cc \
		test/libfred/registrar/group/test_update.cc \
		test/libfred/registrar/group/test_cancel.cc \
		test/libfred/registrar/group/test_get.cc \
		test/libfred/registrar/group/membership/test_create.cc \
		test/libfred/registrar/group/membership/test_end.cc \
		test/libfred/registrar/group/membership/test_info_by_group.cc \
		test/libfred/registrar/group/membership/test_info_by_registrar.cc \
		test/libfred/contact/test_delete_contact.cc \
		test/libfred/contact/test_create_contact.cc \
		test/libfred/contact/test_info_contact.cc \
		test/libfred/contact/test_merge_contact.cc \
		test/libfred/contact/test_merge_contact_fixture.hh \
		test/libfred/contact/test_update_contact.cc \
		test/libfred/contact/test_copy_contact.cc \
		test/libfred/contact/test_transfer_contact.cc \
		test/libfred/contact/test_mojeid_checkers.cc \
		test/libfred/contact/test_undisclose_address.cc \
		src/util/types/birthdate.cc \
		test/libfred/domain/test_update_domain.cc \
		test/libfred/domain/test_renew_domain.cc \
		test/libfred/domain/test_info_domain.cc \
		test/libfred/domain/test_delete_domain.cc \
		test/libfred/domain/test_create_domain.cc \
		test/libfred/domain/test_check_domain.cc \
		test/libfred/domain/test_domain_name.cc \
		test/libfred/domain/test_create_domain_name_blacklist_id.cc \
		test/libfred/domain/test_transfer_domain.cc \
		test/libfred/keyset/test_delete_keyset.cc \
		test/libfred/keyset/test_info_keyset.cc \
		test/libfred/keyset/test_update_keyset.cc \
		test/libfred/keyset/test_transfer_keyset.cc \
		test/libfred/nsset/test_update_nsset.cc \
		test/libfred/nsset/test_delete_nsset.cc \
		test/libfred/nsset/test_info_nsset.cc \
		test/libfred/nsset/test_transfer_nsset.cc \
		test/libfred/public_request/test_create_public_request.cc \
		test/libfred/public_request/test_create_public_request_auth.cc \
		test/libfred/public_request/test_update_public_request.cc \
		test/libfred/contact/verification/setup_utils.hh \
		test/libfred/contact/verification/setup_utils.cc \
		test/libfred/contact/verification/test_create_check.cc \
		test/libfred/contact/verification/test_update_check.cc \
		test/libfred/contact/verification/test_create_test.cc \
		test/libfred/contact/verification/test_update_test.cc \
		test/libfred/contact/verification/test_info_check.cc \
		test/libfred/contact/verification/test_list_checks.cc \
		test/libfred/contact/verification/test_list_enum_objects.cc \
		test/libfred/contact/verification/test_django_email_format_check.cc \
		test/libfred/poll/test_create_delete_contact_poll_message.cc \
		test/libfred/poll/test_create_delete_domain_poll_message.cc \
		test/libfred/poll/test_create_update_object_poll_message.cc \
		test/libfred/poll/test_get_request_fee_info_message.cc \
		test/libfred/poll/test_create_state_messages.cc \
		test/libfred/object/test_generate_authinfo_password.cc \
		test/libfred/object/test_generated_authinfo_password.cc \
		test/libfred/object/test_transfer_object.cc \
		test/libfred/object_state/test_object_state.cc \
		test/libfred/object_state/test_create_object_state_request_id.cc \
		test/libfred/object_state/test_create_admin_object_block_request_id.cc \
		test/libfred/object_state/test_create_admin_object_state_restore_request_id.cc \
		test/libfred/object_state/test_cancel_object_state_request_id.cc \
		test/libfred/object/test_registry_object_type.cc \
		test/libfred/notifier/test_enqueue_notification.cc \
		test/libfred/notifier/test_event_on_object_enum.cc \
		test/libfred/notifier/test_gather_email_addresses_contact_create.cc \
		test/libfred/notifier/test_gather_email_addresses_contact_transfer.cc \
		test/libfred/notifier/test_gather_email_addresses_contact_delete.cc \
		test/libfred/notifier/test_gather_email_addresses_contact_update.cc \
		test/libfred/notifier/test_gather_email_addresses_domain_create.cc \
		test/libfred/notifier/test_gather_email_addresses_domain_update.cc \
		test/libfred/notifier/test_gather_email_addresses_domain_transfer.cc \
		test/libfred/notifier/test_gather_email_addresses_domain_delete.cc \
		test/libfred/notifier/test_gather_email_addresses_domain_renew.cc \
		test/libfred/notifier/test_gather_email_addresses_keyset_create.cc \
		test/libfred/notifier/test_gather_email_addresses_keyset_update.cc \
		test/libfred/notifier/test_gather_email_addresses_keyset_transfer.cc \
		test/libfred/notifier/test_gather_email_addresses_keyset_delete.cc \
		test/libfred/notifier/test_gather_email_addresses_nsset_create.cc \
		test/libfred/notifier/test_gather_email_addresses_nsset_update.cc \
		test/libfred/notifier/test_gather_email_addresses_nsset_transfer.cc \
		test/libfred/notifier/test_gather_email_addresses_nsset_delete.cc \
		test/libfred/notifier/test_gather_email_content_contact_create.cc \
		test/libfred/notifier/test_gather_email_content_contact_delete.cc \
		test/libfred/notifier/test_gather_email_content_contact_transfer.cc \
		test/libfred/notifier/test_gather_email_content_contact_update.cc \
		test/libfred/notifier/test_gather_email_content_domain_create.cc \
		test/libfred/notifier/test_gather_email_content_domain_delete.cc \
		test/libfred/notifier/test_gather_email_content_domain_renew.cc \
		test/libfred/notifier/test_gather_email_content_domain_transfer.cc \
		test/libfred/notifier/test_gather_email_content_domain_update.cc \
		test/libfred/notifier/test_gather_email_content_keyset_create.cc \
		test/libfred/notifier/test_gather_email_content_keyset_delete.cc \
		test/libfred/notifier/test_gather_email_content_keyset_transfer.cc \
		test/libfred/notifier/test_gather_email_content_keyset_update.cc \
		test/libfred/notifier/test_gather_email_content_nsset_create.cc \
		test/libfred/notifier/test_gather_email_content_nsset_delete.cc \
		test/libfred/notifier/test_gather_email_content_nsset_transfer.cc \
		test/libfred/notifier/test_gather_email_content_nsset_update.cc \
		test/libfred/notifier/test_process_one_notification_request.cc \
		test/libfred/registrar/certification/test_create_registrar_certification.cc \
		test/libfred/registrar/certification/test_update_registrar_certification.cc \
		test/libfred/registrar/certification/test_get_registrar_certifications.cc \
		test/libfred/test_opexception.cc \
		test/libfred/test_check_handle.cc \
		test/libfred/zone/test_create_zone.cc \
		test/libfred/zone/test_info_zone.cc \
		test/libfred/zone/test_update_zone.cc \
		test/libfred/zone/util.cc \
		test/libfred/zone/util.hh \
		test/libfred/zone_soa/test_create_zone_soa.cc \
		test/libfred/zone_soa/test_info_zone_soa.cc \
		test/libfred/zone_soa/test_update_zone_soa.cc \
		test/libfred/zone_soa/util.hh \
		test/libfred/zone_soa/util.cc \
		test/libfred/zone_ns/util.cc \
		test/libfred/zone_ns/util.hh \
		test/libfred/zone_ns/test_create_zone_ns.cc \
		test/libfred/zone_ns/test_info_zone_ns.cc \
		test/libfred/zone_ns/test_update_zone_ns.cc \
		test/util/test_param_query.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc \
		src/util/random.hh \
		src/util/random.cc


test_test_libfred_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_libfred_LDADD = \
		src/libfred/libfred.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		-lmpdec -lm -lrt \
		src/bin/corba/libcorba.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/libfred/public_request/libpublic_request.a \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(POSTGRESQL_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/test_libfred

# test/test_optys
#
test_test_optys_SOURCES = \
		test/test_optys_main.cc \
		test/util/test_optys.cc \
		src/util/optys/download_client.cc \
		src/util/optys/download_client.hh \
		src/bin/cli/optys_get_undelivered.cc \
		src/bin/cli/optys_get_undelivered.hh \
		src/bin/cli/read_config_file.hh \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc



test_test_optys_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_optys_LDADD = \
		src/libfred/libfred.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/types/libtypes.a \
		src/util/tz/libtimezones.a \
		-lmpdec -lm -lrt \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(POSTGRESQL_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/test_optys

# test/test_corba_conversion
#
test_test_corba_conversion_SOURCES = \
		src/backend/buffer.hh \
		src/backend/buffer.cc \
		src/bin/corba/util/corba_conversions_buffer.hh \
		src/bin/corba/util/corba_conversions_buffer.cc \
		src/util/corba_conversion.hh \
		src/util/corba_conversion.cc \
		src/bin/corba/util/corba_conversions_isodatetime.hh \
		src/bin/corba/util/corba_conversions_isodatetime.cc \
		src/bin/corba/mojeid/mojeid_corba_conversion.hh \
		src/bin/corba/mojeid/mojeid_corba_conversion.cc \
		test/util/test_corba_conversion_main.cc \
		test/util/test_corba_conversion.cc \
		src/bin/cli/read_config_file.hh \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_test_corba_conversion_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_corba_conversion_LDADD = \
		src/bin/corba/libcorba_conversion_test.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/bin/corba/libcorbanullableisodatetime.a \
		src/bin/corba/libcorbamojeid.a \
		src/bin/corba/util/libcorbautil.a \
		src/libfred/libfred.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/tz/libtimezones.a \
		-lmpdec -lm -lrt \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(POSTGRESQL_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/test_corba_conversion

# test/backend/test-automatic-keyset-management
#
test_backend_test_automatic_keyset_management_SOURCES = \
		src/bin/corba/nameservice.hh \
		src/bin/corba/nameservice.cc \
		src/bin/corba/logger_client_impl.hh \
		src/bin/corba/logger_client_impl.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc \
		src/backend/epp/keyset/dns_key.cc \
		src/backend/epp/keyset/dns_key.hh \
		src/backend/automatic_keyset_management/dns_key.cc \
		src/backend/automatic_keyset_management/dns_key.hh \
		src/backend/automatic_keyset_management/automatic_keyset_management.cc \
		src/backend/automatic_keyset_management/automatic_keyset_management.hh \
		src/backend/automatic_keyset_management/impl/limits.hh \
		src/backend/automatic_keyset_management/impl/logger_request.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_rollover.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_turn_off.hh \
		src/backend/automatic_keyset_management/impl/logger_request_akm_turn_on.hh \
		src/backend/automatic_keyset_management/impl/logger_request_data.cc \
		src/backend/automatic_keyset_management/impl/logger_request_data.hh \
		src/backend/automatic_keyset_management/impl/logger_request_object_type.cc \
		src/backend/automatic_keyset_management/impl/logger_request_object_type.hh \
		src/backend/automatic_keyset_management/impl/logger_request_property.cc \
		src/backend/automatic_keyset_management/impl/logger_request_property.hh \
		src/backend/automatic_keyset_management/impl/logger_request_result.hh \
		src/backend/automatic_keyset_management/impl/logger_request_type.cc \
		src/backend/automatic_keyset_management/impl/logger_request_type.hh \
		src/backend/automatic_keyset_management/impl/logger_service_type.hh \
		src/backend/automatic_keyset_management/impl/util.cc \
		src/backend/automatic_keyset_management/impl/util.hh \
		test/backend/automatic_keyset_management/util.hh \
		test/backend/automatic_keyset_management/fixture.hh \
		test/backend/automatic_keyset_management/main.cc \
		test/backend/automatic_keyset_management/test_automatic_keyset_management.cc

test_backend_test_automatic_keyset_management_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_automatic_keyset_management_LDADD = \
		src/bin/corba/libcorbaautomatickeysetmanagement.a \
		src/libfred/libfred.a \
		src/bin/corba/util/libcorbautil.a \
		src/bin/corba/libcorba.a \
		src/bin/corba/libcorbalogger.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbaadmin.a \
		src/bin/corba/libcorbaregistry.a \
		src/bin/corba/libcorbafilters.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test_backend_test_automatic_keyset_management

# test/backend/test-record-statement
#
test_backend_test_record_statement_SOURCES = \
		src/backend/buffer.cc \
		src/backend/buffer.hh \
		src/backend/record_statement/record_statement.cc \
		src/backend/record_statement/record_statement.hh \
		src/backend/record_statement/purpose.hh \
		src/backend/record_statement/exceptions.cc \
		src/backend/record_statement/exceptions.hh \
		src/backend/record_statement/impl/record_statement_xml.cc \
		src/backend/record_statement/impl/record_statement_xml.hh \
		src/backend/record_statement/impl/templates_impl.hh \
		src/backend/record_statement/impl/factory.cc \
		src/backend/record_statement/impl/factory.hh \
		src/backend/record_statement/impl/utc_impl.cc \
		src/backend/record_statement/impl/europe_prague_impl.cc \
		src/backend/record_statement/impl/util.cc \
		src/backend/record_statement/impl/util.hh \
		test/backend/record_statement/util.hh \
		test/backend/record_statement/fixture.hh \
		test/backend/record_statement/main.cc \
		test/backend/record_statement/test_record_statement.cc \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_backend_test_record_statement_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_backend_test_record_statement_LDADD = \
		src/libfred/libfred.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		src/util/db/libdb.a \
		src/util/tz/libtimezones.a \
		-lmpdec -lm -lrt \
		src/bin/corba/libcorba.a \
		src/bin/corba/libmailer.a \
		src/bin/corba/libcorbadatatypes.a \
		src/bin/corba/libcorbabuffer.a \
		src/bin/corba/libcorbaisodate.a \
		src/bin/corba/libcorbaisodatetime.a \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(POSTGRESQL_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test_backend_test_record_statement

# test/test-password-storage
#
test_test_password_storage_SOURCES = \
		test/util/test_password_storage.cc \
		test/util/test_password_storage_main.cc \
		src/bin/cli/read_config_file.hh \
		test/setup/fixtures_utils.hh \
		test/setup/fixtures_utils.cc \
		test/setup/fixtures.hh \
		test/setup/fixtures.cc

test_test_password_storage_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_password_storage_LDADD = \
		src/libfred/libfred.a \
		src/bin/corba/libcorbaisodatetime.a \
		src/bin/corba/libcorbanullableisodate.a \
		src/util/libpassword_storage.a \
		src/util/libutil.a \
		src/util/log/liblog.a \
		-lmpdec -lm -lrt \
		$(LIBCRYPTO_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_FILESYSTEM_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(BOOST_SYSTEM_LIB) \
		$(POSTGRESQL_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#
# test/test-password-storage


# non-corba tests with overloaded global operator new
#
test_test_exception_SOURCES = \
		test/util/test_exception.cc

test_test_exception_CPPFLAGS = \
		$(AM_CPPFLAGS)

test_test_exception_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

test_test_exception_LDADD = \
		-lmpdec -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_RANDOM_LIB) \
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
		test/deprecated/example_payments.xml \
		test/deprecated/test_file.pdf
