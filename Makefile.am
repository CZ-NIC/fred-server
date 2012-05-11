ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
		$(POSTGRESQL_CFLAGS) \
		${BOOST_CPPFLAGS} \
		$(AM_CFLAGS) \
		$(LIBIDN_CFLAGS) \
		$(OMNIORB4_CFLAGS) \
		$(XML2_CFLAGS) \
		-I $(top_srcdir)/util \
		-I $(top_srcdir)/src \
		-I . \
		-I $(top_srcdir)/util/db/model \
		-I $(top_srcdir)/src/fredlib \
		-I $(top_srcdir)/src/fredlib/banking \
		-I $(top_srcdir)/src/corba \
		-I $(top_srcdir)/src/corba/admin \
		-I $(top_srcdir)/util/db \
		-I $(top_srcdir)/util \
		-I $(top_srcdir)/util/decimal \
		-I $(top_builddir)/src/corba \
		-I $(top_builddir)/src

AM_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS)

include $(top_srcdir)/am_doxygen.am


CLEANFILES = \
		config/server.conf \
		config/hpmail.conf \
		init-scripts/fred-server \
		util/decimal/mpdecimal-2.2/mpdecimal.h \
		src/corba/MessagesSK.cc \
		src/corba/MessagesDynSK.cc \
		src/corba/MessagesSK.h \
		src/corba/Messages.hh \
		src/corba/MojeIDSK.cc \
		src/corba/MojeIDDynSK.cc \
		src/corba/MojeIDSK.h \
		src/corba/MojeID.hh \
		src/corba/FileManagerSK.cc \
		src/corba/FileManagerDynSK.cc \
		src/corba/FileManagerSK.h \
		src/corba/FileManager.hh \
		src/corba/MailerSK.cc \
		src/corba/MailerDynSK.cc \
		src/corba/MailerSK.h \
		src/corba/Mailer.hh \
		src/corba/ZoneSK.cc \
		src/corba/ZoneDynSK.cc \
		src/corba/ZoneSK.h \
		src/corba/Zone.hh \
		src/corba/_dataTypesSK.cc \
		src/corba/_dataTypesDynSK.cc \
		src/corba/_dataTypesSK.h \
		src/corba/_dataTypes.hh \
		src/corba/FiltersSK.cc \
		src/corba/FiltersDynSK.cc \
		src/corba/FiltersSK.h \
		src/corba/Filters.hh \
		src/corba/RegistrySK.cc \
		src/corba/RegistryDynSK.cc \
		src/corba/RegistrySK.h \
		src/corba/Registry.hh \
		src/corba/WhoisSK.cc \
		src/corba/WhoisDynSK.cc \
		src/corba/WhoisSK.h \
		src/corba/Whois.hh \
		src/corba/AdminSK.cc \
		src/corba/AdminDynSK.cc \
		src/corba/AdminSK.h \
		src/corba/Admin.hh \
		src/corba/LoggerSK.cc \
		src/corba/LoggerDynSK.cc \
		src/corba/LoggerSK.h \
		src/corba/Logger.hh \
		src/corba/TechCheckSK.cc \
		src/corba/TechCheckDynSK.cc \
		src/corba/TechCheckSK.h \
		src/corba/TechCheck.hh \
		src/corba/EPPSK.cc \
		src/corba/EPPDynSK.cc \
		src/corba/EPPSK.h \
		src/corba/EPP.hh \
		src/corba/ZoneGeneratorSK.cc \
		src/corba/ZoneGeneratorDynSK.cc \
		src/corba/ZoneGeneratorSK.h \
		src/corba/ZoneGenerator.hh


fredconfdir = $(sysconfdir)/fred
rcconfdir = $(sysconfdir)/init.d


EXTRA_DIST = \
		fred-server.spec \
		util/sms_send.bash \
		tests/test-file.pdf \
		util/decimal/mpdecimal-2.2/README.txt \
		util/decimal/mpdecimal-2.2/LICENSE.txt

dist_fredconf_DATA = \
		config/server.conf \
		config/hpmail.conf

dist_rcconf_SCRIPTS = \
		init-scripts/fred-server


if DX_COND_html
doc_DATA = @DX_DOCDIR@/html/*
$(doc_DATA): doxygen-doc
MOSTLYCLEANFILES = ${DX_CLEANFILES}
endif


noinst_HEADERS = \
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
		util/cfg/handle_adifd_args.h \
		util/cfg/handle_logd_args.h \
		util/cfg/handle_rifd_args.h \
		util/cfg/command_selection_args.h \
		util/cfg/check_args.h \
		util/cfg/validate_args.h \
		util/cfg/handle_sms_args.h \
		util/types/optional.h \
		util/types/optional_from_program_options.h \
		util/types/money.h \
		tests/exception-test.h


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
		src/fredlib/mojeid/libmojeid.a \
		src/fredlib/banking/libbanking.a \
		src/corba/libcorba.a \
		src/corba/libcorbamessages.a \
		src/corba/libcorbamojeid.a \
		src/corba/libcorbazone.a \
		src/corba/libfilemanager.a \
		src/corba/libmailer.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/whois/libwhois_impl.a \
		util/decimal/libmpdec.a \
		src/corba/epp/libepp_impl.a \
		src/corba/log/liblog_impl.a \
		util/hp/libhp.a \
		src/fredlib/notifier/libnotifier.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libcorbaepp.a \
		src/corba/libcorbazonegenerator.a \
		src/fredlib/public_request/libpublic_request.a


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
		src/cli_admin/fred-admin \
		tools/logd_migration/migrate_log


# util/libutil.a
#
util_libutil_a_SOURCES = \
		util/random.cc \
		util/util.cc
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
		$(BOOST_THREAD_LIB)
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
		$(POSTGRESQL_LIB)
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
		$(BOOST_THREAD_LIB)

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


# util/decimal/libmpdec.a
#
MPD_HEADER = @MPD_HEADER@
MPD_WARN = @MPD_WARN@
MPD_CONFIG = @MPD_CONFIG@

util_decimal_libmpdec_a_CFLAGS = \
		-I $(top_srcdir)/util/decimal/mpdecimal-2.2 \
		-I $(top_builddir)/util/decimal/mpdecimal-2.2 \
		$(CONFIGURE_CFLAGS)

util_decimal_libmpdec_a_SOURCES = \
		util/decimal/mpdecimal-2.2/basearith.c \
		util/decimal/mpdecimal-2.2/context.c \
		util/decimal/mpdecimal-2.2/constants.c \
		util/decimal/mpdecimal-2.2/convolute.c \
		util/decimal/mpdecimal-2.2/crt.c \
		util/decimal/mpdecimal-2.2/mpdecimal.c \
		util/decimal/mpdecimal-2.2/mpsignal.c \
		util/decimal/mpdecimal-2.2/difradix2.c \
		util/decimal/mpdecimal-2.2/fnt.c \
		util/decimal/mpdecimal-2.2/fourstep.c \
		util/decimal/mpdecimal-2.2/io.c \
		util/decimal/mpdecimal-2.2/memory.c \
		util/decimal/mpdecimal-2.2/numbertheory.c \
		util/decimal/mpdecimal-2.2/sixstep.c \
		util/decimal/mpdecimal-2.2/transpose.c \
		util/decimal/mpdecimal-2.2/mptypes.h \
		util/decimal/mpdecimal-2.2/umodarith.h \
		util/decimal/mpdecimal-2.2/basearith.h \
		util/decimal/mpdecimal-2.2/fnt.h \
		util/decimal/mpdecimal-2.2/memory.h \
		util/decimal/mpdecimal-2.2/mpdecimal32vc.h \
		util/decimal/mpdecimal-2.2/convolute.h \
		util/decimal/mpdecimal-2.2/difradix2.h \
		util/decimal/mpdecimal-2.2/io.h \
		util/decimal/mpdecimal-2.2/mpdecimal64vc.h \
		util/decimal/mpdecimal-2.2/numbertheory.h \
		util/decimal/mpdecimal-2.2/vccompat.h \
		util/decimal/mpdecimal-2.2/constants.h \
		util/decimal/mpdecimal-2.2/transpose.h \
		util/decimal/mpdecimal-2.2/bits.h \
		util/decimal/mpdecimal-2.2/fourstep.h \
		util/decimal/mpdecimal-2.2/sixstep.h \
		util/decimal/mpdecimal-2.2/vcstdint.h \
		util/decimal/mpdecimal-2.2/crt.h \
		util/decimal/mpdecimal-2.2/docstrings.h \
		util/decimal/mpdecimal-2.2/mptest.h \
		util/decimal/mpdecimal-2.2/typearith.h \
		util/decimal/mpdecimal-2.2/vcdiv64.asm
#
# util/decimal/libmpdec.a


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
		$(POSTGRESQL_LIB)
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
		src/fredlib/reminder.h
#
# src/fredlib/libfred.a


# src/fredlib/public_request/libpublic_request.a
#
src_fredlib_public_request_libpublic_request_a_SOURCES = \
		src/fredlib/public_request/public_request.cc \
		src/fredlib/public_request/public_request.h \
		src/fredlib/public_request/public_request_impl.cc \
		src/fredlib/public_request/public_request_impl.h \
		src/fredlib/public_request/public_request_authinfo_impl.h \
		src/fredlib/public_request/public_request_block_impl.h \
		src/fredlib/public_request/public_request_verification_impl.h
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


# src/fredlib/mojeid/libmojeid.a
#
src_fredlib_mojeid_libmojeid_a_SOURCES = \
		src/fredlib/mojeid/contact.h \
		src/fredlib/mojeid/contact.cc \
		src/fredlib/mojeid/nullable.h \
		src/fredlib/mojeid/request.h \
		src/fredlib/mojeid/mojeid_data_validation.h \
		src/fredlib/mojeid/mojeid_data_validation.cc \
		src/fredlib/mojeid/mojeid_contact_states.h \
		src/fredlib/mojeid/mojeid_disclose_policy.h
#
# src/fredlib/mojeid/libmojeid.a


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


BUILT_SOURCES = \
		src/corba/MessagesSK.cc \
		src/corba/MojeIDSK.cc \
		src/corba/FileManagerSK.cc \
		src/corba/MailerSK.cc \
		src/corba/ZoneSK.cc \
		src/corba/_dataTypesSK.cc \
		src/corba/FiltersSK.cc src/corba/RegistrySK.cc \
		src/corba/WhoisSK.cc \
		src/corba/AdminSK.cc \
		src/corba/LoggerSK.cc \
		src/corba/TechCheckSK.cc \
		src/corba/EPPSK.cc \
		src/corba/ZoneGeneratorSK.cc


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
		src/corba/_dataTypes.hh

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
#
# nodist corba libs


# idl build
#
src/corba/MessagesSK.cc: $(IDLDIR)/Messages.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Messages.idl

src/corba/MojeIDSK.cc: $(IDLDIR)/MojeID.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/MojeID.idl

src/corba/FileManagerSK.cc: $(IDLDIR)/FileManager.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/FileManager.idl

src/corba/MailerSK.cc: $(IDLDIR)/Mailer.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Mailer.idl

src/corba/ZoneSK.cc: $(IDLDIR)/Zone.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Zone.idl

src/corba/_dataTypesSK.cc: $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/_dataTypes.idl

src/corba/FiltersSK.cc: $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Filters.idl

src/corba/RegistrySK.cc: $(IDLDIR)/Registry.idl $(IDLDIR)/Filters.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Registry.idl

src/corba/WhoisSK.cc: $(IDLDIR)/Whois.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Whois.idl

src/corba/AdminSK.cc: $(IDLDIR)/Admin.idl $(IDLDIR)/Whois.idl $(IDLDIR)/Filters.idl $(IDLDIR)/Registry.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Admin.idl

src/corba/LoggerSK.cc: $(IDLDIR)/Logger.idl $(IDLDIR)/Admin.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/Logger.idl

src/corba/TechCheckSK.cc: $(IDLDIR)/TechCheck.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/TechCheck.idl

src/corba/EPPSK.cc: $(IDLDIR)/EPP.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/EPP.idl

src/corba/ZoneGeneratorSK.cc: $(IDLDIR)/ZoneGenerator.idl $(IDLDIR)/_dataTypes.idl
		omniidl -bcxx -Wba -C$(top_builddir)/src/corba $(IDLDIR)/ZoneGenerator.idl
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


# src/corba/epp/libepp_impl.a
#
src_corba_epp_libepp_impl_a_SOURCES = \
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
		src/corba/libcorbaepp.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/mojeid/libmojeid.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		util/log/liblog.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		util/decimal/libmpdec.a -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB)
#
# src/corba/fred-rifd


# src/corba/fred-adifd
#
src_corba_fred_adifd_SOURCES = \
		src/corba/fred-adifd.cc

src_corba_fred_adifd_LDADD = \
		src/corba/admin/libadmin_impl.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbazone.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaepp.a \
		src/corba/libfilemanager.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/mojeid/libmojeid.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/banking/libbanking.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		util/log/liblog.a \
		src/old_utils/liboldutils.a \
		util/libutil.a \
		util/decimal/libmpdec.a -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB)
#
# src/corba/fred-adifd


# src/corba/fred-pifd
#
src_corba_fred_pifd_SOURCES = \
		src/corba/fred-pifd.cc

src_corba_fred_pifd_LDADD = \
		src/corba/whois/libwhois_impl.a \
		src/corba/admin/libadmin_impl.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbazone.a \
		src/corba/libmailer.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libcorba.a \
		src/corba/libcorbaepp.a \
		src/corba/libfilemanager.a \
		src/corba/libcorbalogger.a \
		src/fredlib/requests/librequests.a \
		src/fredlib/banking/libbanking.a \
		src/fredlib/messages/libmessages.a \
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/mojeid/libmojeid.a \
		src/fredlib/libfred.a \
		src/model/libmodel.a \
		util/db/libdb.a \
		util/types/libtypes.a \
		src/old_utils/liboldutils.a \
		util/log/liblog.a \
		util/libutil.a \
		util/decimal/libmpdec.a -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB)
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
		util/decimal/libmpdec.a -lm \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB)
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
		$(POSTGRESQL_LIB)
#
# src/corba/messages/fred-msgd


# src/corba/mojeid/fred-mifd
#
src_corba_mojeid_fred_mifd_SOURCES = \
		src/corba/nameservice.cc \
		src/corba/mailer_manager.cc \
		src/corba/mojeid/mojeid_impl.h \
		src/corba/mojeid/mojeid_impl.cc \
		src/corba/mojeid/mojeid_identification.h \
		src/corba/mojeid/mojeid_notifier.cc \
		src/corba/mojeid/mojeid_notifier.h \
		src/corba/mojeid/corba_conversion.h \
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
		src/fredlib/public_request/libpublic_request.a \
		src/fredlib/mojeid/libmojeid.a \
		src/fredlib/messages/libmessages.a \
		util/libutil.a \
		util/log/liblog.a \
		util/types/libtypes.a \
		util/db/libdb.a \
		src/old_utils/liboldutils.a \
		util/decimal/libmpdec.a -lm \
		$(LIBIDN_LIBS) \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(OMNIORB4_LIBS) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB)
#
# src/corba/mojeid/fred-mifd


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
		src/cli_admin/charge_client_impl.h

src_cli_admin_fred_admin_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS)

src_cli_admin_fred_admin_LDADD = \
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
		util/decimal/libmpdec.a -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(OMNIORB4_LIBS) \
		$(XML2_LIBS) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(POSTGRESQL_LIB)
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
		$(POSTGRESQL_LIB)
#
# tools/logd_migration


# tests
#
SERVER_TESTS = \
		tests/test-server \
		tests/test-exception


# tests/test_server
#
tests_test_server_SOURCES = \
		tests/test-server.cc \
		tests/test-server.h \
		tests/test-conv.cc \
		tests/test-conv.h \
		tests/test-file-manager-client.cc \
		tests/test-file-manager-client.h \
		tests/test-db.cc \
		tests/test-db.h \
		tests/test-model.cc \
		tests/test-model.h \
		tests/test-registrar-group.cc \
		tests/test-registrar-group.h \
		tests/test-registrar-blocking.cc \
		src/corba/admin/filter_impl.cc \
		src/corba/admin/filter_impl.h \
		tests/test-registrar-certification.cc \
		tests/test-registrar-certification.h \
		tests/test_logd.cc \
		tests/test-msgd.cc \
		tests/test-msgd.h \
		tests/test-admin.cc \
		tests/test-corbans.cc \
		tests/test-whois.cc \
		tests/test-cache.cc \
		tests/tests-common.cc \
		tests/tests-common.h \
		tests/test-filters-date-time.cc \
		tests/test-invoice-common.h \
		tests/test-invoice-common.cc \
		tests/test-invoice.cc \
		tests/test-invoice-account.cc \
		tests/test-common-threaded.h \
		tests/test-common-registry.h \
		tests/test-common-registry.cc \
		tests/test-invoice2.cc \
		tests/test-decimal.cc \
		tests/test-shellcmd.cc \
		src/corba/epp_corba_client_impl.cc \
		src/corba/epp_corba_client_impl.h

tests_test_server_LDFLAGS = \
		$(POSTGRESQL_LDFLAGS) \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_server_LDADD = \
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
		src/fredlib/mojeid/libmojeid.a \
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
		src/corba/libcorbaepp.a \
		src/corba/libcorbatechcheck.a \
		src/corba/libcorbalogger.a \
		src/corba/libcorbaadmin.a \
		src/corba/libcorbawhois.a \
		src/corba/libcorbaregistry.a \
		src/corba/libcorbafilters.a \
		src/corba/libcorbadatatypes.a \
		src/corba/libmailer.a \
		util/decimal/libmpdec.a -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(OMNIORB4_LIBS) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS) \
		$(POSTGRESQL_LIB)
#
# tests/test_server


# non-corba tests with overloaded global operator new
#
tests_test_exception_SOURCES = \
		tests/test-exception.cc

tests_test_exception_LDFLAGS = \
		$(BOOST_LDFLAGS) \
		$(AM_LDFLAGS)

tests_test_exception_LDADD = \
		util/decimal/libmpdec.a -lm \
		$(BOOST_PROGRAM_OPTIONS_LIB) \
		$(BOOST_DATE_TIME_LIB) \
		$(BOOST_REGEX_LIB) \
		$(LIBIDN_CFLAGS) \
		$(LIBIDN_LIBS) \
		$(BOOST_THREAD_LIB) \
		$(BOOST_SERIALIZATION_LIB) \
		$(XML2_LIBS)
#
# non-corba tests with overloaded global operator new




install-tests: $(SERVER_TESTS)
		test -z "$(bindir)" || $(MKDIR_P) "$(bindir)"
		$(INSTALL_PROGRAM) $(SERVER_TESTS) $(bindir)

testdatadir = $(docdir)

dist_testdata_DATA = \
		tests/example_payments.xml \
		tests/test-file.pdf

