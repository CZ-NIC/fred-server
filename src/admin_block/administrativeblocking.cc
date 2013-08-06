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
 *  @administrativeblocking.cc
 *  administrativeblocking implementation
 */

#include "administrativeblocking.h"
#include "corba/connection_releaser.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/create_administrative_object_block_request.h"
#include "fredlib/domain/create_administrative_object_block_request_id.h"
#include "fredlib/domain/create_administrative_object_state_restore_request.h"
#include "fredlib/domain/create_administrative_object_state_restore_request_id.h"
#include "fredlib/domain/create_domain_name_blacklist.h"
#include "fredlib/domain/create_domain_name_blacklist_id.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/create_contact.h"
#include <memory>

namespace
{

enum { OBJECT_TYPE_DOMAIN = 3 };

}

namespace Registry
{
    namespace Administrative
    {

        StatusDescList* BlockingImpl::getBlockingStatusDescList(const std::string &_lang)
        {
            try {
                std::auto_ptr< StatusDescList > result(new StatusDescList);
                Fred::OperationContext ctx;
                Fred::GetBlockingStatusDescList blocking_status_desc_list;
                blocking_status_desc_list.set_lang(_lang);
                Fred::GetBlockingStatusDescList::StatusDescList &desc_list = blocking_status_desc_list.exec(ctx);
                result->length(desc_list.size());
                int n = 0;
                for (Fred::GetBlockingStatusDescList::StatusDescList::const_iterator pItem = desc_list.begin();
                     pItem != desc_list.end(); ++n, ++pItem) {
                    StatusDesc &item = (*result)[n];
                    item.id = pItem->state_id;
                    item.shortName = ::CORBA::string_dup(pItem->status.c_str());
                    item.name = ::CORBA::string_dup(pItem->desc.c_str());
                }
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        DomainOwnerChangeList* BlockingImpl::blockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
            const std::string &_reason)
        {
            try {
                std::auto_ptr< DomainOwnerChangeList > result(new DomainOwnerChangeList);
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateAdministrativeObjectBlockRequest create_object_state_request(domain, OBJECT_TYPE_DOMAIN, status_list);
                    const Fred::ObjectId object_id = create_object_state_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        DomainIdHandleOwnerChangeList* BlockingImpl::blockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            try {
                std::auto_ptr< DomainIdHandleOwnerChangeList > result(new DomainIdHandleOwnerChangeList);
                if (_owner_block_mode == BLOCK_OWNER_COPY) {
                    result->length(_domain_list.length());
                }
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectBlockRequestId create_object_block_request(object_id, status_list);
                    create_object_block_request.set_reason(_reason);
                    if (_owner_block_mode == BLOCK_OWNER) {
                        Database::query_param_list param(object_id);
                        Database::Result registrant_result = ctx.get_conn().exec_params(
                            "SELECT rc.name,rc.type "
                            "FROM domain d "
                            "JOIN object_registry rc ON rc.id=d.registrant "
                            "WHERE d.id=$1::bigint", param);
                        if (registrant_result.size() <= 0) {
                            std::string errmsg("|| not found:object_id: ");
                            errmsg += boost::lexical_cast< std::string >(object_id);
                            errmsg += " |";
                            throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                        }
                        const Database::Row &row = registrant_result[0];
                        const std::string registrant = static_cast< std::string >(row[0]);
                        const Fred::ObjectType type = static_cast< Fred::ObjectType >(row[1]);
                        Fred::StatusList status_list;
                        status_list.push_back("serverUpdateProhibited");
                        Fred::CreateAdministrativeObjectBlockRequest block_owner_request(registrant, type, status_list);
                        block_owner_request.set_reason(_reason);
                        const Fred::ObjectId registrant_id = block_owner_request.exec(ctx);
                        Fred::PerformObjectStateRequest(registrant_id).exec(ctx);
                        create_object_block_request.exec(ctx);
                    }
                    else if (_owner_block_mode == BLOCK_OWNER_COPY) {
                        std::string owner_copy_name;
                        Fred::ObjectId contact_type;
                        std::string registrar;
                        const std::string domain = create_object_block_request.exec(ctx);
                        ::Registry::Administrative::DomainIdHandleOwnerChange &result_item = (*result)[idx];
                        result_item.domainId = object_id;
                        result_item.domainHandle = ::CORBA::string_dup(domain.c_str());
                        {
                            Database::query_param_list param(object_id);
                            Database::Result owner_result = ctx.get_conn().exec_params(
                                "SELECT o.id,o.name,o.type,rar.handle "
                                "FROM domain d "
                                "JOIN object_registry o ON o.id=d.registrant "
                                "JOIN registrar rar ON rar.id=o.crid "
                                "WHERE d.id=$1::bigint", param);
                            if (owner_result.size() <= 0) {
                                std::string errmsg("|| not found:object_id: ");
                                errmsg += boost::lexical_cast< std::string >(object_id);
                                errmsg += " |";
                                throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                            }
                            const Database::Row &row = owner_result[0];
                            result_item.oldOwnerId = static_cast< Fred::ObjectId >(row[0]);
                            const std::string owner_name = static_cast< std::string >(row[1]);
                            contact_type = static_cast< Fred::ObjectId >(row[2]);
                            registrar = static_cast< std::string >(row[3]);
                            result_item.oldOwnerHandle = ::CORBA::string_dup(owner_name.c_str());
                            static const std::string owner_copy_suffix = "-ABC_"; //AdministrativeBlockingCopy
                            owner_copy_name = owner_name + owner_copy_suffix;
                            Database::Result last_owner_result = ctx.get_conn().exec_params(
                                "SELECT name "
                                "FROM object_registry "
                                "WHERE (type=$1::bigint) AND "
                                      "($2<name) AND "
                                      "(name LIKE $3) AND "
                                      "((erdate IS NULL) OR (NOW()<erdate)) "
                                "ORDER BY name DESC LIMIT 1", Database::query_param_list(contact_type)
                                                                                        (owner_copy_name)
                                                                                        (owner_copy_name + "%"));
                            static const std::string owner_copy_zero_idx = "00000";
                            if (last_owner_result.size() == 0) {
                                owner_copy_name += owner_copy_zero_idx;
                            }
                            else {
                                const std::string last_owner_name = static_cast< std::string >(last_owner_result[0][0]);
                                const std::string str_copy_idx = last_owner_name.substr(owner_copy_name.length());
                                ::size_t copy_idx = 0;
                                bool ilegal_copy_idx = false;
                                for (const char *pC = str_copy_idx.c_str(); *pC != '\0'; ++pC) {
                                    if (('0' <= *pC) && (*pC <= '9')) {
                                        copy_idx = 10 * copy_idx + int(*pC) - int('0');
                                    }
                                    else {
                                        ilegal_copy_idx = true;
                                        break;
                                    }
                                }
                                if (ilegal_copy_idx) {
                                    owner_copy_name = last_owner_name + owner_copy_suffix + owner_copy_zero_idx;
                                }
                                else {
                                    ++copy_idx;
                                    std::ostringstream idx;
                                    idx << std::setw(owner_copy_zero_idx.length()) << std::setfill('0') << copy_idx;
                                    owner_copy_name += idx.str();
                                }
                            }
                            Fred::InfoContact info_contact(owner_name, registrar);
                            Fred::InfoContactOutput old_contact = info_contact.exec(ctx);
                            Fred::CreateContact create_contact(owner_copy_name, registrar);
                            if (!old_contact.info_contact_data.authinfopw.empty()) {
                                create_contact.set_authinfo(old_contact.info_contact_data.authinfopw);
                            }
                            if (!old_contact.info_contact_data.name.isnull()) {
                                create_contact.set_name(old_contact.info_contact_data.name);
                            }
                            if (!old_contact.info_contact_data.organization.isnull()) {
                                create_contact.set_organization(old_contact.info_contact_data.organization);
                            }
                            if (!old_contact.info_contact_data.street1.isnull()) {
                                create_contact.set_street1(old_contact.info_contact_data.street1);
                            }
                            if (!old_contact.info_contact_data.street2.isnull()) {
                                create_contact.set_street2(old_contact.info_contact_data.street2);
                            }
                            if (!old_contact.info_contact_data.street3.isnull()) {
                                create_contact.set_street3(old_contact.info_contact_data.street3);
                            }
                            if (!old_contact.info_contact_data.city.isnull()) {
                                create_contact.set_city(old_contact.info_contact_data.city);
                            }
                            if (!old_contact.info_contact_data.stateorprovince.isnull()) {
                                create_contact.set_stateorprovince(old_contact.info_contact_data.stateorprovince);
                            }
                            if (!old_contact.info_contact_data.postalcode.isnull()) {
                                create_contact.set_postalcode(old_contact.info_contact_data.postalcode);
                            }
                            if (!old_contact.info_contact_data.country.isnull()) {
                                create_contact.set_country(old_contact.info_contact_data.country);
                            }
                            if (!old_contact.info_contact_data.telephone.isnull()) {
                                create_contact.set_telephone(old_contact.info_contact_data.telephone);
                            }
                            if (!old_contact.info_contact_data.fax.isnull()) {
                                create_contact.set_fax(old_contact.info_contact_data.fax);
                            }
                            if (!old_contact.info_contact_data.email.isnull()) {
                                create_contact.set_email(old_contact.info_contact_data.email);
                            }
                            if (!old_contact.info_contact_data.notifyemail.isnull()) {
                                create_contact.set_notifyemail(old_contact.info_contact_data.notifyemail);
                            }
                            if (!old_contact.info_contact_data.vat.isnull()) {
                                create_contact.set_vat(old_contact.info_contact_data.vat);
                            }
                            if (!old_contact.info_contact_data.ssntype.isnull()) {
                                create_contact.set_ssntype(old_contact.info_contact_data.ssntype);
                            }
                            if (!old_contact.info_contact_data.ssn.isnull()) {
                                create_contact.set_ssn(old_contact.info_contact_data.ssn);
                            }
                            if (!old_contact.info_contact_data.disclosename.isnull()) {
                                create_contact.set_disclosename(old_contact.info_contact_data.disclosename);
                            }
                            if (!old_contact.info_contact_data.discloseorganization.isnull()) {
                                create_contact.set_discloseorganization(old_contact.info_contact_data.discloseorganization);
                            }
                            if (!old_contact.info_contact_data.discloseaddress.isnull()) {
                                create_contact.set_discloseaddress(old_contact.info_contact_data.discloseaddress);
                            }
                            if (!old_contact.info_contact_data.disclosetelephone.isnull()) {
                                create_contact.set_disclosetelephone(old_contact.info_contact_data.disclosetelephone);
                            }
                            if (!old_contact.info_contact_data.disclosefax.isnull()) {
                                create_contact.set_disclosefax(old_contact.info_contact_data.disclosefax);
                            }
                            if (!old_contact.info_contact_data.discloseemail.isnull()) {
                                create_contact.set_discloseemail(old_contact.info_contact_data.discloseemail);
                            }
                            if (!old_contact.info_contact_data.disclosevat.isnull()) {
                                create_contact.set_disclosevat(old_contact.info_contact_data.disclosevat);
                            }
                            if (!old_contact.info_contact_data.discloseident.isnull()) {
                                create_contact.set_discloseident(old_contact.info_contact_data.discloseident);
                            }
                            if (!old_contact.info_contact_data.disclosenotifyemail.isnull()) {
                                create_contact.set_disclosenotifyemail(old_contact.info_contact_data.disclosenotifyemail);
                            }
                            create_contact.exec(ctx);
                            Database::Result new_owner_result = ctx.get_conn().exec_params(
                                "SELECT id "
                                "FROM object_registry "
                                "WHERE (type=$1::bigint) AND "
                                      "(name=$2)", Database::query_param_list(contact_type)
                                                                             (owner_copy_name));
                            if (new_owner_result.size() <= 0) {
                                std::string errmsg("|| not found:object_id: ");
                                errmsg += boost::lexical_cast< std::string >(object_id);
                                errmsg += " |";
                                throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                            }
                            result_item.newOwnerHandle = ::CORBA::string_dup(owner_copy_name.c_str());
                            result_item.newOwnerId = static_cast< Fred::ObjectId >(new_owner_result[0][0]);
                        }
                        Fred::StatusList owner_block_list;
                        owner_block_list.push_back("serverUpdateProhibited");
                        Fred::CreateAdministrativeObjectBlockRequest block_owner_request(owner_copy_name, contact_type, owner_block_list);
                        block_owner_request.set_reason(_reason);
                        const Fred::ObjectId registrant_id = block_owner_request.exec(ctx);
                        Fred::PerformObjectStateRequest(registrant_id).exec(ctx);
                        Fred::UpdateDomain update_domain(domain, registrar);
                        update_domain.set_registrant(owner_copy_name);
                        update_domain.exec(ctx);
                    }
                    else { // KEEP_OWNER 
                        create_object_block_request.exec(ctx);
                    }
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::restorePreAdministrativeBlockStates(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateAdministrativeObjectStateRestoreRequest create_object_state_restore_request(domain, OBJECT_TYPE_DOMAIN);
                    const Fred::ObjectId object_id = create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::restorePreAdministrativeBlockStatesId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason);
                    create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    if ((_new_owner != NULL) && (_new_owner->_value() != NULL) && (_new_owner->_value()[0] != '\0')) {
                        Database::query_param_list param(object_id);
                        Database::Result registrar_fqdn_result = ctx.get_conn().exec_params(
                            "SELECT reg.handle,oreg.name "
                            "FROM object_registry oreg "
                            "JOIN registrar reg ON oreg.crid=reg.id "
                            "WHERE oreg.id=$1::bigint", param);
                        if (registrar_fqdn_result.size() <= 0) {
                            std::string errmsg("|| not found:object_id: ");
                            errmsg += boost::lexical_cast< std::string >(object_id);
                            errmsg += " |";
                            throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                        }
                        const Database::Row &row = registrar_fqdn_result[0];
                        const std::string registrar = static_cast< std::string >(row[0]);
                        const std::string domain = static_cast< std::string >(row[1]);
                        Fred::UpdateDomain update_domain(domain, registrar);
                        update_domain.set_registrant(_new_owner->_value());
                        update_domain.exec(ctx);
                    }
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::updateBlockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
//                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
//                    const std::string domain = _domain_list[idx].in();
//                    enum { OBJECT_TYPE_DOMAIN = 3 };
//                    Fred::CreateAdministrativeObjectBlockRequest create_object_state_request(domain, OBJECT_TYPE_DOMAIN, status_list);
//                    const Fred::ObjectId object_id = create_object_state_request.exec(ctx);
//                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
//                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::updateBlockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            try {
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason);
                    create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    Fred::CreateAdministrativeObjectBlockRequestId create_object_state_request(object_id, status_list);
                    create_object_state_request.set_reason(_reason);
                    create_object_state_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::unblockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            bool _remove_admin_c,
            const std::string &_reason)
        {
        }

        void BlockingImpl::unblockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            bool _remove_admin_c,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            if (!_remove_admin_c) {
                this->restorePreAdministrativeBlockStatesId(_domain_list, _new_owner, _reason, _log_req_id);
                return;
            }
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason);
                    create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    Database::query_param_list param(object_id);
                    Database::Result registrar_fqdn_result = ctx.get_conn().exec_params(
                        "SELECT reg.handle,oreg.name "
                        "FROM object_registry oreg "
                        "JOIN registrar reg ON oreg.crid=reg.id "
                        "WHERE oreg.id=$1::bigint", param);
                    if (registrar_fqdn_result.size() <= 0) {
                        std::string errmsg("|| not found:object_id: ");
                        errmsg += boost::lexical_cast< std::string >(object_id);
                        errmsg += " |";
                        throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                    }
                    const Database::Row &row = registrar_fqdn_result[0];
                    const std::string registrar = static_cast< std::string >(row[0]);
                    const std::string domain = static_cast< std::string >(row[1]);
                    Fred::UpdateDomain update_domain(domain, registrar);
                    if ((_new_owner != NULL) && (_new_owner->_value() != NULL) && (_new_owner->_value()[0] != '\0')) {
                        update_domain.set_registrant(_new_owner->_value());
                    }
                    Database::Result admin_name_result = ctx.get_conn().exec_params(
                        "SELECT rc.name "
                        "FROM domain_contact_map dcm "
                        "JOIN object_registry rc ON rc.id=dcm.contactid "
                        "WHERE dcm.domainid=$1::bigint", param);
                    for (::size_t idx = 0; idx < admin_name_result.size(); ++idx) {
                        const Database::Row &row = admin_name_result[idx];
                        const std::string admin_name = static_cast< std::string >(row[0]);
                        update_domain.rem_admin_contact(admin_name);
                    }
                    update_domain.exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::blacklistAndDeleteDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date)
        {
        }

        void BlockingImpl::blacklistAndDeleteDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            unsigned long long _log_req_id)
        {
        }

        void BlockingImpl::blacklistDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            bool _with_delete)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateDomainNameBlacklist(domain, "").exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::blacklistDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            bool _with_delete,
            unsigned long long _log_req_id)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, "blacklistDomainsId() call");
                    create_domain_name_blacklist.exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::unblacklistAndCreateDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const std::string &_owner)
        {
        }


//        void BlockingImpl::unblacklistAndCreateDomainsId(
//            const ::Registry::Administrative::DomainIdList &_domain_list,
//            const std::string &_owner)
//        {
//        }

    }//namespace Administrative
}//namespace Registry



