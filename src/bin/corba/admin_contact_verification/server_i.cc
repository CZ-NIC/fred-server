/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  implementation of admin contact verification wrapper over corba
 */

#include "src/bin/corba/admin_contact_verification/server_i.hh"

#include "src/backend/admin/contact/verification/create_test_impl_prototypes.hh"
#include "src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh"
#include "src/backend/admin/contact/verification/enqueue_check.hh"
#include "src/backend/admin/contact/verification/exceptions.hh"
#include "src/backend/admin/contact/verification/fill_check_queue.hh"
#include "src/backend/admin/contact/verification/list_active_checks.hh"
#include "src/backend/admin/contact/verification/related_records.hh"
#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "src/backend/admin/contact/verification/run_all_enqueued_checks.hh"
#include "src/backend/admin/contact/verification/update_tests.hh"
#include "src/bin/corba/util/corba_conversions_datetime.hh"
#include "src/bin/corba/util/corba_conversions_nullable_types.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/libfred/registrable_object/contact/check_contact.hh"
#include "src/libfred/registrable_object/contact/copy_contact.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/registrable_object/contact/delete_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact_diff.hh"
#include "src/libfred/registrable_object/contact/merge_contact.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"
#include "src/libfred/registrable_object/contact/verification/create_check.hh"
#include "src/libfred/registrable_object/contact/verification/create_test.hh"
#include "src/libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "src/libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "src/libfred/registrable_object/contact/verification/exceptions.hh"
#include "src/libfred/registrable_object/contact/verification/info_check.hh"
#include "src/libfred/registrable_object/contact/verification/list_checks.hh"
#include "src/libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "src/libfred/registrable_object/contact/verification/update_check.hh"
#include "src/libfred/registrable_object/contact/verification/update_test.hh"
#include "src/util/log/context.hh"
#include "src/util/random_data_generator.hh"
#include "src/util/uuid.hh"

#include <string>
#include <utility>
#include <vector>

static inline std::string create_log_server_id(const std::string& _server_name) {
    return _server_name + "-" + RandomDataGenerator().xnumstring(5);
}

namespace Corba {
    static void wrap_check_detail(const LibFred::InfoContactCheckOutput& in, Registry::AdminContactVerification::ContactCheckDetail_var& out) {
        typedef LibFred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
        typedef LibFred::InfoContactCheckOutput::ContactTestResultData ContactTestData;
        typedef LibFred::InfoContactCheckOutput::ContactTestResultState ContactTestState;

        LibFred::InfoContactOutput contact_info_historical;
        LibFred::InfoContactOutput contact_info_current;
        {
            LibFred::OperationContextCreator ctx;
            contact_info_historical = LibFred::InfoContactHistoryByHistoryid(in.contact_history_id).exec(ctx);
            contact_info_current =
                // looks strange but in case contact was deleted it's current data are not accessible via LibFred::InfoContact anymore
                // so the most recent history is used instead
                LibFred::InfoContactHistoryById(contact_info_historical.info_contact_data.id)
                    .exec(ctx)
                    .at(0);
        }

        out->check_handle =         LibFred::Corba::wrap_string(in.handle);
        out->test_suite_handle =    LibFred::Corba::wrap_string(in.testsuite_handle);
        out->contact_handle =       LibFred::Corba::wrap_string(contact_info_historical.info_contact_data.handle);
        out->contact_id =           contact_info_historical.info_contact_data.id;
        out->checked_contact_hid =  in.contact_history_id;
        out->created =              LibFred::Corba::wrap_time(in.local_create_time);

        out->status_history.length(in.check_state_history.size());
        unsigned long check_seq_i(0);

        for(std::vector<ContactCheckState>::const_iterator check_it = in.check_state_history.begin();
            check_it != in.check_state_history.end();
            ++check_it, ++check_seq_i
        ) {
            out->status_history[check_seq_i].status =           LibFred::Corba::wrap_string(check_it->status_handle);
            out->status_history[check_seq_i].update =           LibFred::Corba::wrap_time(check_it->local_update_time);
            out->status_history[check_seq_i].logd_request_id =  LibFred::Corba::wrap_nullable_ulonglong(check_it->logd_request_id);
        }

        out->test_list.length(in.tests.size());
        unsigned long test_seq_i(0);
        unsigned long testhistory_seq_i= 0;

        for(std::vector<ContactTestData>::const_iterator test_it = in.tests.begin();
            test_it != in.tests.end();
            ++test_it, ++test_seq_i
        ) {
            std::vector<std::string> tested_values =
                Admin::ContactVerification::test_data_provider_factory::instance_ref()
                    .create_sh_ptr(test_it->test_handle)
                    ->init_data(in.contact_history_id)
                        .get_string_data();

            std::vector<std::string> current_values =
                Admin::ContactVerification::test_data_provider_factory::instance_ref()
                    .create_sh_ptr(test_it->test_handle)
                    ->init_data(contact_info_current.info_contact_data.historyid)
                        .get_string_data();

            unsigned out_tested_index = 0;
            out->test_list[test_seq_i].tested_contact_data.length(tested_values.size());
            for(std::vector<std::string>::const_iterator it = tested_values.begin();
                it != tested_values.end();
                ++it, ++out_tested_index
            ) {
                out->test_list[test_seq_i].tested_contact_data[out_tested_index]
                    = LibFred::Corba::wrap_string(*it);
            }

            unsigned out_current_index = 0;
            out->test_list[test_seq_i].current_contact_data.length(current_values.size());
            for(std::vector<std::string>::const_iterator it = current_values.begin();
                it != current_values.end();
                ++it, ++out_current_index
            ) {
                out->test_list[test_seq_i].current_contact_data[out_current_index]
                    = LibFred::Corba::wrap_string(*it);
            }

            out->test_list[test_seq_i].test_handle = LibFred::Corba::wrap_string(test_it->test_handle);
            out->test_list[test_seq_i].created = LibFred::Corba::wrap_time(test_it->local_create_time);

            out->test_list[test_seq_i].status_history.length(test_it->state_history.size());
            testhistory_seq_i= 0;

            for(std::vector<ContactTestState>::const_iterator testhistory_it = test_it->state_history.begin();
                testhistory_it != test_it->state_history.end();
                ++testhistory_it, ++testhistory_seq_i
            ) {
                out->test_list[test_seq_i].status_history[testhistory_seq_i].status
                    = LibFred::Corba::wrap_string(testhistory_it->status_handle);

                out->test_list[test_seq_i].status_history[testhistory_seq_i].err_msg
                    = (testhistory_it->error_msg.isnull())
                        ? LibFred::Corba::wrap_string(std::string())
                        : LibFred::Corba::wrap_string(testhistory_it->error_msg.get_value_or_default());

                out->test_list[test_seq_i].status_history[testhistory_seq_i].update
                    = LibFred::Corba::wrap_time(testhistory_it->local_update_time);

                out->test_list[test_seq_i].status_history[testhistory_seq_i].logd_request_id
                    = LibFred::Corba::wrap_nullable_ulonglong(testhistory_it->logd_request_id);
            }
        }
    }

    static void wrap_check_list(const std::vector<LibFred::ListChecksItem>& in, Registry::AdminContactVerification::ContactCheckList_var& out) {

        out->length(in.size());

        long list_index = 0;
        for(std::vector<LibFred::ListChecksItem>::const_iterator it = in.begin(); it != in.end(); ++it, ++list_index) {
            out[list_index].check_handle =          LibFred::Corba::wrap_string(it->check_handle);
            out[list_index].test_suite_handle =     LibFred::Corba::wrap_string(it->testsuite_handle);
            out[list_index].contact_handle =        LibFred::Corba::wrap_string(it->contact_handle);
            out[list_index].contact_id =            it->contact_id;
            out[list_index].checked_contact_hid =   it->contact_history_id;
            out[list_index].created =               LibFred::Corba::wrap_time(it->local_create_time);
            out[list_index].updated =               LibFred::Corba::wrap_time(it->local_update_time);
            out[list_index].last_contact_update =   LibFred::Corba::wrap_time(it->local_last_contact_update);
            out[list_index].last_test_finished =    LibFred::Corba::wrap_nullable_datetime(it->last_test_finished_local_time);
            out[list_index].current_status =        LibFred::Corba::wrap_string(it->status_handle);
        }
    }

    template<typename Tin, typename Tout>
        static void wrap_enum(const Tin& in, Tout& out) {
            out->length(in.size());

            long out_index = 0;
            for(typename Tin::const_iterator in_it = in.begin();
                in_it != in.end();
                ++in_it, ++out_index
            ) {
                out[out_index].handle =        LibFred::Corba::wrap_string(in_it->handle);
                out[out_index].name =          LibFred::Corba::wrap_string(in_it->name);
                out[out_index].description =   LibFred::Corba::wrap_string(in_it->description);
            }
        }

    static void wrap_test_statuses(
        const std::vector<LibFred::test_result_status>& in,
        Registry::AdminContactVerification::ContactTestStatusDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_check_statuses(
        const std::vector<LibFred::check_status>& in,
        Registry::AdminContactVerification::ContactCheckStatusDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_test_definitions(
        const std::vector<LibFred::test_definition>& in,
        Registry::AdminContactVerification::ContactTestDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_testsuite_definitions(
        const std::vector<LibFred::testsuite_definition>& in,
        Registry::AdminContactVerification::ContactTestSuiteDefSeq_var& out
    ) {
        wrap_enum(in, out);

        long out_index = 0;
        for(typename std::vector<LibFred::testsuite_definition>::const_iterator in_it = in.begin();
            in_it != in.end();
            ++in_it, ++out_index
        ) {
            Registry::AdminContactVerification::ContactTestDefSeq_var temp_tests(new Registry::AdminContactVerification::ContactTestDefSeq);
            wrap_test_definitions(in_it->tests, temp_tests );
            out[out_index].tests = temp_tests;
        }
    }

    static std::vector<std::pair<std::string, std::string> > unwrap_test_change_sequence(
        const Registry::AdminContactVerification::TestUpdateSeq& in
    ) {
        std::vector<std::pair<std::string, std::string> > result;

        for(unsigned long long i=0; i<in.length(); ++i) {
            result.push_back(
                std::make_pair(
                    LibFred::Corba::unwrap_string(in[i].test_handle),
                    LibFred::Corba::unwrap_string(in[i].status)
                )
            );
        }

        return result;
    }

    static void wrap_messages(
        const std::vector<Admin::related_message>& in,
        Registry::AdminContactVerification::MessageSeq_var& out
    ) {
        out->length(in.size());

        long list_index = 0;
        for(std::vector<Admin::related_message>::const_iterator it = in.begin();
            it != in.end();
            ++it, ++list_index
        ) {
            out[list_index].id              = it->id;
            out[list_index].type_handle     = LibFred::Corba::wrap_string(it->comm_type);
            out[list_index].content_handle  = LibFred::Corba::wrap_string(it->content_type);
            out[list_index].created         = LibFred::Corba::wrap_time(it->created);
            out[list_index].updated         = LibFred::Corba::wrap_nullable_datetime(it->update);
            out[list_index].status          = LibFred::Corba::wrap_string(it->status_name);
        }
    }
}

namespace Registry
{
    namespace AdminContactVerification
    {
        ContactCheckDetail* Server_i::getContactCheckDetail(const char* check_handle) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("getContactCheckDetail");

            try {
                ContactCheckDetail_var result(new ContactCheckDetail);
                LibFred::OperationContextCreator ctx;

                Corba::wrap_check_detail(
                    LibFred::InfoContactCheck(
                        uuid::from_string( LibFred::Corba::unwrap_string(check_handle) )
                    ).exec(ctx),
                    result
                );

                return result._retn();
            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch (const Admin::ExceptionUnknownCheckHandle&) {
                throw UNKNOWN_CHECK_HANDLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactCheckList* Server_i::getChecksOfContact(::CORBA::ULongLong contact_id, NullableString* testsuite, ::CORBA::ULong max_item_count) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("getChecksOfContact");

            try {
                ContactCheckList_var result(new ContactCheckList);
                LibFred::OperationContextCreator ctx;
                LibFred::ListContactChecks list_checks;

                list_checks
                    .set_max_item_count(static_cast<unsigned long>(max_item_count))
                    .set_contact_id( static_cast<unsigned long long>(contact_id) );

                Optional<std::string> testsuite_unwrapped = LibFred::Corba::unwrap_nullable_string_to_optional(testsuite);
                if(testsuite_unwrapped.isset()) {
                    list_checks.set_testsuite_handle(testsuite_unwrapped.get_value());
                }

                Corba::wrap_check_list(
                    list_checks.exec(ctx),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactCheckList* Server_i::getActiveChecks(NullableString* testsuite) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("getActiveChecks");

            try {
                ContactCheckList_var result(new ContactCheckList);

                Corba::wrap_check_list(
                    Admin::list_active_checks( LibFred::Corba::unwrap_nullable_string_to_optional(testsuite) ),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        void Server_i::updateContactCheckTests(const char* check_handle, const TestUpdateSeq& changes, ::CORBA::ULongLong logd_request_id){
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("updateContactCheckTests");

            try {
                LibFred::OperationContextCreator ctx;

                Admin::update_tests(
                    ctx,
                    uuid::from_string( LibFred::Corba::unwrap_string(check_handle) ),
                    Corba::unwrap_test_change_sequence(changes),
                    logd_request_id
                );

                ctx.commit_transaction();
            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch(const Admin::ExceptionUnknownCheckHandle&) {
                throw UNKNOWN_CHECK_HANDLE();
            } catch(const Admin::ExceptionUnknownTestHandle&) {
                throw UNKNOWN_TEST_HANDLE();
            } catch(const Admin::ExceptionUnknownCheckTestPair&) {
                throw UNKNOWN_CHECK_TEST_PAIR();
            } catch(const Admin::ExceptionUnknownTestStatusHandle&) {
                throw UNKNOWN_TEST_STATUS_HANDLE();
            } catch(const Admin::ExceptionCheckNotUpdateable&) {
                throw CHECK_NOT_UPDATEABLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        void Server_i::resolveContactCheckStatus(const char* check_handle, const char* status, ::CORBA::ULongLong logd_request_id){
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("resolveContactCheckStatus");

            try {
                LibFred::OperationContextCreator ctx;

                Admin::resolve_check(
                    uuid::from_string( LibFred::Corba::unwrap_string(check_handle) ),
                    LibFred::Corba::unwrap_string(status),
                    logd_request_id
                ).exec(ctx);

                ctx.commit_transaction();
            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch(const Admin::ExceptionUnknownCheckHandle&) {
                throw UNKNOWN_CHECK_HANDLE();
            } catch(const Admin::ExceptionUnknownCheckStatusHandle&) {
                throw UNKNOWN_CHECK_STATUS_HANDLE();
            } catch(const Admin::ExceptionCheckNotUpdateable&) {
                throw CHECK_NOT_UPDATEABLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        void Server_i::deleteDomainsAfterFailedManualCheck(const char* check_handle) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("deleteDomainsAfterFailedManualCheck");

            try {
                LibFred::OperationContextCreator ctx;

                Admin::delete_domains_of_invalid_contact(
                    ctx,
                    uuid::from_string( LibFred::Corba::unwrap_string(check_handle) )
                );

                ctx.commit_transaction();
            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch (const Admin::ExceptionUnknownCheckHandle&) {
                throw UNKNOWN_CHECK_HANDLE();
            } catch (const Admin::ExceptionIncorrectTestsuite&) {
                throw INCORRECT_TESTSUITE();
            } catch (const Admin::ExceptionIncorrectCheckStatus&) {
                throw INCORRECT_CHECK_STATUS();
            } catch (const Admin::ExceptionIncorrectContactStatus&) {
                throw INCORRECT_CONTACT_STATUS();
            } catch (const Admin::ExceptionDomainsAlreadyDeleted&) {
                throw DOMAINS_ALREADY_DELETED();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        char* Server_i::requestEnqueueingContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle, ::CORBA::ULongLong logd_request_id) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("requestEnqueueingContactCheck");

            try {
                LibFred::OperationContextCreator ctx;

                std::string created_handle;

                created_handle = Admin::request_check_enqueueing(
                    ctx,
                    contact_id,
                    LibFred::Corba::unwrap_string(testsuite_handle),
                    logd_request_id
                );

                ctx.commit_transaction();

                return CORBA::string_dup(created_handle.c_str());
            } catch(const LibFred::ExceptionUnknownContactId&) {
                throw UNKNOWN_CONTACT_ID();
            } catch(const LibFred::ExceptionUnknownTestsuiteHandle&) {
                throw UNKNOWN_TESTSUITE_HANDLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        void Server_i::confirmEnqueueingContactCheck(const char* check_handle, ::CORBA::ULongLong logd_request_id) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("confirmEnqueueingContactCheck");

            try {
                LibFred::OperationContextCreator ctx;

                Admin::confirm_check_enqueueing(
                    ctx,
                    uuid::from_string( LibFred::Corba::unwrap_string(check_handle) ),
                    logd_request_id
                );

                ctx.commit_transaction();

            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch (const LibFred::ExceptionUnknownCheckHandle&) {
                throw UNKNOWN_CHECK_HANDLE();
            } catch(const Admin::ExceptionCheckNotUpdateable&) {
                throw CHECK_NOT_UPDATEABLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        char* Server_i::enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle, ::CORBA::ULongLong logd_request_id){
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("enqueueContactCheck");

            try {
                LibFred::OperationContextCreator ctx;

                std::string created_handle;

                created_handle = Admin::enqueue_check(
                    ctx,
                    contact_id,
                    LibFred::Corba::unwrap_string(testsuite_handle),
                    logd_request_id
                );

                ctx.commit_transaction();

                return CORBA::string_dup(created_handle.c_str());
            } catch(const Admin::ExceptionUnknownContactId&) {
                throw UNKNOWN_CONTACT_ID();
            } catch(const Admin::ExceptionUnknownTestsuiteHandle&) {
                throw UNKNOWN_TESTSUITE_HANDLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        MessageSeq* Server_i::getContactCheckMessages(::CORBA::ULongLong contact_id) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("getContactCheckMessages");

            try {
                MessageSeq_var result(new MessageSeq);

                LibFred::OperationContextCreator ctx;

                Corba::wrap_messages(
                    Admin::get_related_messages(
                        ctx,
                        contact_id
                    ),
                    result
                );

                return result._retn();

            } catch (const uuid::ExceptionInvalidUuid&) {
                throw INVALID_CHECK_HANDLE();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactTestStatusDefSeq* Server_i::listTestStatusDefs(const char* lang) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("listTestStatusDefs");

            try {
                ContactTestStatusDefSeq_var result (new ContactTestStatusDefSeq);

                Corba::wrap_test_statuses(
                    LibFred::list_test_result_statuses(
                        LibFred::Corba::unwrap_string(lang)),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactCheckStatusDefSeq* Server_i::listCheckStatusDefs(const char* lang) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("listCheckStatusDefs");

            try {
                ContactCheckStatusDefSeq_var result (new ContactCheckStatusDefSeq);

                Corba::wrap_check_statuses(
                    LibFred::list_check_statuses(
                        LibFred::Corba::unwrap_string(lang)),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactTestDefSeq* Server_i::listTestDefs(
            const char* lang,
            Registry::NullableString* testsuite_handle
        ) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("listTestDefs");

            try {
                ContactTestDefSeq_var result (new ContactTestDefSeq);

                Corba::wrap_test_definitions(
                    LibFred::list_test_definitions(
                        LibFred::Corba::unwrap_string(lang),
                        LibFred::Corba::unwrap_nullable_string(testsuite_handle).get_value_or_default()
                    ),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }

        ContactTestSuiteDefSeq* Server_i::listTestSuiteDefs(const char* lang) {
            Logging::Context log_server(create_log_server_id(server_name_));
            Logging::Context log_method("listTestSuiteDefs");

            try {
                ContactTestSuiteDefSeq_var result (new ContactTestSuiteDefSeq);

                Corba::wrap_testsuite_definitions(
                    LibFred::list_testsuite_definitions(
                        LibFred::Corba::unwrap_string(lang)),
                    result
                );

                return result._retn();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }
        }
    }
}