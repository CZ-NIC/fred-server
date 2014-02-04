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

#include "src/corba/admin_contact_verification/server_i.h"

#include <vector>
#include <utility>

#include "src/fredlib/contact/util.h"
#include "src/admin/contact/verification/enqueue_check.h"
#include "src/fredlib/contact/verification/update_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/list_checks.h"
#include "src/fredlib/contact/verification/list_enum_objects.h"
#include "src/fredlib/contact/verification/update_test.h"
#include "src/admin/contact/verification/resolve_check.h"
#include "src/corba/util/corba_conversions_datetime.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_nullable_types.h"

namespace Corba {
    static void wrap_check_detail(const Fred::InfoContactCheckOutput& in, Registry::AdminContactVerification::ContactCheckDetail_var& out) {
        typedef Fred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
        typedef Fred::InfoContactCheckOutput::ContactTestResultData ContactTestData;
        typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestState;

        // TODO prasarna - casem pouzit novou verzi InfoContactByHistoryId

        std::pair<std::string, unsigned long long> contact_data = Fred::ContactUtil::contact_hid_to_handle_id_pair(in.contact_history_id);

        out->check_handle =         Corba::wrap_string(in.handle);
        out->test_suite_handle =    Corba::wrap_string(in.testsuite_handle);
        out->contact_handle =       Corba::wrap_string(contact_data.first);
        out->contact_id =           contact_data.second;
        out->checked_contact_hid =  in.contact_history_id;
        out->created =              Corba::wrap_time(in.local_create_time);

        out->status_history.length(in.check_state_history.size());
        unsigned long check_seq_i(0);

        for(std::vector<ContactCheckState>::const_iterator check_it = in.check_state_history.begin();
            check_it != in.check_state_history.end();
            ++check_it, ++check_seq_i
        ) {
            out->status_history[check_seq_i].status =           Corba::wrap_string(check_it->status_handle);
            out->status_history[check_seq_i].update =           Corba::wrap_time(check_it->local_update_time);
            out->status_history[check_seq_i].logd_request_id =  Corba::wrap_nullable_ulonglong(check_it->logd_request_id);
        }

        out->test_list.length(in.tests.size());
        unsigned long test_seq_i(0);
        unsigned long testhistory_seq_i= 0;

        for(std::vector<ContactTestData>::const_iterator test_it = in.tests.begin();
            test_it != in.tests.end();
            ++test_it, ++test_seq_i
        ) {

            // TODO pridelat diffovatko na data kontaktu
            // TODO vyresit jaka data kontaktu ktery test kontroluje
            // TODO vyresit jake casy jdou ven

            out->test_list[test_seq_i].test_handle = Corba::wrap_string(test_it->test_handle);
            out->test_list[test_seq_i].created = Corba::wrap_time(test_it->local_create_time);

            out->test_list[test_seq_i].status_history.length(test_it->state_history.size());
            testhistory_seq_i= 0;

            for(std::vector<ContactTestState>::const_iterator testhistory_it = test_it->state_history.begin();
                testhistory_it != test_it->state_history.end();
                ++testhistory_it, ++testhistory_seq_i
            ) {
                out->test_list[test_seq_i].status_history[testhistory_seq_i].status
                    = Corba::wrap_string(testhistory_it->status_handle);

                out->test_list[test_seq_i].status_history[testhistory_seq_i].err_msg
                    = (testhistory_it->error_msg.isnull())
                        ? Corba::wrap_string(std::string())
                        : Corba::wrap_string(static_cast<std::string>(testhistory_it->error_msg));

                out->test_list[test_seq_i].status_history[testhistory_seq_i].update
                    = Corba::wrap_time(testhistory_it->local_update_time);

                out->test_list[test_seq_i].status_history[testhistory_seq_i].logd_request_id
                    = Corba::wrap_nullable_ulonglong(testhistory_it->logd_request_id);
            }
        }
    }

    static void wrap_check_list(const std::vector<Fred::ListChecksItem>& in, Registry::AdminContactVerification::ContactCheckList_var& out) {

        out->length(in.size());

        std::pair<std::string, unsigned long long> contact_data;
        long list_index = 0;
        for(std::vector<Fred::ListChecksItem>::const_iterator it = in.begin(); it != in.end(); ++it, ++list_index) {
            out->operator[](list_index).check_handle =          Corba::wrap_string(it->check_handle);
            out->operator[](list_index).test_suite_handle =     Corba::wrap_string(it->testsuite_handle);

            contact_data = Fred::ContactUtil::contact_hid_to_handle_id_pair(it->contact_history_id);

            out->operator[](list_index).contact_handle =        Corba::wrap_string(contact_data.first);
            out->operator[](list_index).contact_id =            contact_data.second;
            out->operator[](list_index).checked_contact_hid =   it->contact_history_id;
            out->operator[](list_index).created =               Corba::wrap_time(it->local_create_time);
            out->operator[](list_index).current_status =        Corba::wrap_string(it->status_handle);

            out->operator[](list_index).created =               Corba::wrap_time(it->local_create_time);

            out->operator[](list_index).tests_finished =
                (it->local_tests_finished_time.isset())
                ? Corba::wrap_nullable_datetime(it->local_tests_finished_time.get_value())
                : Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());

            out->operator[](list_index).last_relevant_contact_update =
                (it->local_relevant_contact_update_time.isset())
                ? Corba::wrap_nullable_datetime(it->local_relevant_contact_update_time.get_value())
                : Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());
        }
    }

    template<typename Tin, typename Tout>
        static void wrap_enum(const Tin& in, Tout& out) {
            (out.operator->())->length(in.size());

            long out_index = 0;
            for(typename Tin::const_iterator in_it = in.begin();
                in_it != in.end();
                ++in_it, ++out_index
            ) {
                out->operator [](out_index).handle =        Corba::wrap_string(in_it->handle);
                out->operator [](out_index).name =          Corba::wrap_string(in_it->name);
                out->operator [](out_index).description =   Corba::wrap_string(in_it->description);
            }
        }

    static void wrap_test_statuses(
        const std::vector<Fred::test_result_status>& in,
        Registry::AdminContactVerification::ContactTestStatusDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_check_statuses(
        const std::vector<Fred::check_status>& in,
        Registry::AdminContactVerification::ContactCheckStatusDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_test_definitions(
        const std::vector<Fred::test_definition>& in,
        Registry::AdminContactVerification::ContactTestDefSeq_var& out
    ) {
        wrap_enum(in, out);
    }

    static void wrap_testsuite_definitions(
        const std::vector<Fred::testsuite_definition>& in,
        Registry::AdminContactVerification::ContactTestSuiteDefSeq_var& out
    ) {
        wrap_enum(in, out);

        long out_index = 0;
        for(typename std::vector<Fred::testsuite_definition>::const_iterator in_it = in.begin();
            in_it != in.end();
            ++in_it, ++out_index
        ) {
            Registry::AdminContactVerification::ContactTestDefSeq_var temp_tests(new Registry::AdminContactVerification::ContactTestDefSeq);
            Corba::wrap_test_definitions(in_it->tests, temp_tests );
            out->operator [](out_index).tests = temp_tests;
        }
    }
}

namespace Registry
{
    namespace AdminContactVerification
    {
        ContactCheckDetail* Server_i::getContactCheckDetail(const char* check_handle) {
            ContactCheckDetail_var result(new ContactCheckDetail);
            Fred::OperationContext ctx;

            Corba::wrap_check_detail(
                Fred::InfoContactCheck(Corba::unwrap_string(check_handle))
                    .exec(ctx),
                result
            );

            return result._retn();
        }

        ContactCheckList* Server_i::getContactCheckList(NullableString* testsuite, NullableULongLong* contact_id, ::CORBA::ULong max_item_count) {
            ContactCheckList_var result(new ContactCheckList);
            Fred::OperationContext ctx;

            Corba::wrap_check_list(
                Fred::ListContactChecks(
                    static_cast<unsigned long>(max_item_count),
                    Corba::unwrap_nullable_string_to_optional(testsuite),
                    Corba::unwrap_nullable_ulonglong_to_optional(contact_id)
                ).exec(ctx),
                result
            );

            return result._retn();
        }

        void Server_i::updateContactCheckTests(const char* check_handle, const TestUpdateSeq& changes, ::CORBA::ULongLong logd_request_id){
            std::string ch_handle(Corba::unwrap_string(check_handle));
            std::string testname;
            std::string status;

            Fred::OperationContext ctx;

            for(unsigned long long i=0; i<changes.length(); ++i) {
                status = Corba::unwrap_string(changes[i].status);
                testname = Corba::unwrap_string(changes[i].test_handle);
                Fred::UpdateContactTest(ch_handle, testname, status)
                    .set_logd_request_id(logd_request_id)
                    .exec(ctx);
            }

            ctx.commit_transaction();
        }

        void Server_i::resolveContactCheckStatus(const char* check_handle, const char* status, ::CORBA::ULongLong logd_request_id){
            Fred::OperationContext ctx;

            Admin::resolve_check(
                Corba::unwrap_string(check_handle),
                Corba::unwrap_string(status),
                logd_request_id
            ).exec(ctx);

            ctx.commit_transaction();
        }

        char* Server_i::enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle, ::CORBA::ULongLong logd_request_id){
            Fred::OperationContext ctx;

            std::string created_handle;
            created_handle = Admin::enqueue_check(
                ctx,
                contact_id,
                Corba::unwrap_string(testsuite_handle),
                logd_request_id
            );

            ctx.commit_transaction();

            return CORBA::string_dup(created_handle.c_str());
        }

        ContactTestStatusDefSeq* Server_i::listTestStatusDefs(const char* lang) {
            ContactTestStatusDefSeq_var result (new ContactTestStatusDefSeq);

            Corba::wrap_test_statuses(
                Fred::list_test_result_statuses(
                    Corba::unwrap_string(lang)),
                result
            );

            return result._retn();
        }

        ContactCheckStatusDefSeq* Server_i::listCheckStatusDefs(const char* lang) {
            ContactCheckStatusDefSeq_var result (new ContactCheckStatusDefSeq);

            Corba::wrap_check_statuses(
                Fred::list_check_statuses(
                    Corba::unwrap_string(lang)),
                result
            );

            return result._retn();
        }

        ContactTestDefSeq* Server_i::listTestDefs(
            const char* lang,
            Registry::NullableString* testsuite_handle
        ) {
            ContactTestDefSeq_var result (new ContactTestDefSeq);

            Corba::wrap_test_definitions(
                Fred::list_test_definitions(
                    Corba::unwrap_string(lang),
                    Corba::unwrap_nullable_string(testsuite_handle)),
                result
            );

            return result._retn();
        }

        ContactTestSuiteDefSeq* Server_i::listTestSuiteDefs(const char* lang) {
            ContactTestSuiteDefSeq_var result (new ContactTestSuiteDefSeq);

            Corba::wrap_testsuite_definitions(
                Fred::list_testsuite_definitions(
                    Corba::unwrap_string(lang)),
                result
            );

            return result._retn();
        }
    }
}
