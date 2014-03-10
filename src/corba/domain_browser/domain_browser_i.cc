/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @domain_browser_i.cc
 *  implementation of domain browser interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/DomainBrowser.idl
 */

#include "domain_browser_i.h"
#include "src/domain_browser/domain_browser.h"
#include "src/corba/DomainBrowser.hh"
#include <string>


namespace Registry
{
    namespace DomainBrowser
    {

        Server_i::Server_i(const std::string &_server_name)
        : pimpl_(new Registry::DomainBrowserImpl::DomainBrowser(_server_name))
        {}

        Server_i::~Server_i()
        {}

        //   Methods corresponding to IDL attributes and operations
        Registry::DomainBrowser::TID Server_i::getObjectRegistryId(
            const char* objtype,
            const char* handle)
        {
            try
            {
                unsigned long long id = pimpl_->getObjectRegistryId(objtype,handle);
                return id;
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSet* Server_i::getDomainList(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* lang,
            ::CORBA::ULong offset,
             ::CORBA::Boolean& limit_exceeded)
        {
            try
            {
                RecordSet_var rs = new RecordSet;
                rs->length(1);

                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                rs[0] = rseq;

                return rs._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSet* Server_i::getNssetList(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* lang,
            ::CORBA::ULong offset,
             ::CORBA::Boolean& limit_exceeded)
        {
            try
            {
                RecordSet_var rs = new RecordSet;
                rs->length(1);

                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                rs[0] = rseq;

                return rs._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSet* Server_i::getKeysetList(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* lang,
            ::CORBA::ULong offset,
             ::CORBA::Boolean& limit_exceeded)
        {
            try
            {
                RecordSet_var rs = new RecordSet;
                rs->length(1);

                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                rs[0] = rseq;

                return rs._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSet* Server_i::getDomainsForKeyset(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& keyset,
            const char* lang,
            ::CORBA::ULong offset,
            ::CORBA::Boolean& limit_exceeded)
        {
            try
            {
                RecordSet_var rs = new RecordSet;
                rs->length(1);

                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                rs[0] = rseq;

                return rs._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::ACCESS_DENIED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSet* Server_i::getDomainsForNsset(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& nsset,
            const char* lang,
            ::CORBA::ULong offset,
            ::CORBA::Boolean& limit_exceeded)
        {
            try
            {
                RecordSet_var rs = new RecordSet;
                rs->length(1);

                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                rs[0] = rseq;

                return rs._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::ACCESS_DENIED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::ContactDetail* Server_i::getContactDetail(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& detail,
            const char* lang,
            Registry::DomainBrowser::DataAccessLevel& auth_result)
        {
            try
            {
                Registry::DomainBrowserImpl::ContactDetail detail_impl
                    = pimpl_->getContactDetail(contact.id, detail.id, lang);

                ContactDetail_var contact_detail = new ContactDetail;
                contact_detail->id = detail_impl.id;
                contact_detail->handle = CORBA::string_dup(detail_impl.handle.c_str());
                contact_detail->roid = CORBA::string_dup(detail_impl.roid.c_str());
                contact_detail->registrar.id = detail_impl.sponsoring_registrar.id;
                contact_detail->registrar.handle = CORBA::string_dup(detail_impl.sponsoring_registrar.handle.c_str());
                contact_detail->registrar.name = CORBA::string_dup(detail_impl.sponsoring_registrar.name.c_str());
                contact_detail->create_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(detail_impl.creation_time.date()).c_str());
                contact_detail->transfer_date = CORBA::string_dup(detail_impl.transfer_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.transfer_time.get_value().date()).c_str());
                contact_detail->update_date = CORBA::string_dup(detail_impl.update_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.update_time.get_value().date()).c_str());
                contact_detail->auth_info = CORBA::string_dup(detail_impl.authinfopw.c_str());
                contact_detail->name = CORBA::string_dup(detail_impl.name.get_value_or_default().c_str());
                contact_detail->organization = CORBA::string_dup(detail_impl.organization.get_value_or_default().c_str());
                contact_detail->street1 = CORBA::string_dup(detail_impl.street1.get_value_or_default().c_str());
                contact_detail->street2 = CORBA::string_dup(detail_impl.street2.get_value_or_default().c_str());
                contact_detail->street3 = CORBA::string_dup(detail_impl.street3.get_value_or_default().c_str());
                contact_detail->province = CORBA::string_dup(detail_impl.stateorprovince.get_value_or_default().c_str());
                contact_detail->postalcode = CORBA::string_dup(detail_impl.postalcode.get_value_or_default().c_str());
                contact_detail->city = CORBA::string_dup(detail_impl.city.get_value_or_default().c_str());
                contact_detail->country = CORBA::string_dup(detail_impl.country.get_value_or_default().c_str());
                contact_detail->telephone = CORBA::string_dup(detail_impl.telephone.get_value_or_default().c_str());
                contact_detail->fax = CORBA::string_dup(detail_impl.fax.get_value_or_default().c_str());
                contact_detail->email = CORBA::string_dup(detail_impl.email.get_value_or_default().c_str());
                contact_detail->notify_email = CORBA::string_dup(detail_impl.notifyemail.get_value_or_default().c_str());
                contact_detail->ssn = CORBA::string_dup(detail_impl.ssn.get_value_or_default().c_str());
                contact_detail->ssn_type = CORBA::string_dup(detail_impl.ssntype.get_value_or_default().c_str());
                contact_detail->vat = CORBA::string_dup(detail_impl.vat.get_value_or_default().c_str());
                contact_detail->disclose_flags.address = detail_impl.disclose_flags.address;
                contact_detail->disclose_flags.email = detail_impl.disclose_flags.email;
                contact_detail->disclose_flags.fax = detail_impl.disclose_flags.fax;
                contact_detail->disclose_flags.ident = detail_impl.disclose_flags.ident;
                contact_detail->disclose_flags.name = detail_impl.disclose_flags.name;
                contact_detail->disclose_flags.notify_email = detail_impl.disclose_flags.notify_email;
                contact_detail->disclose_flags.organization = detail_impl.disclose_flags.organization;
                contact_detail->disclose_flags.telephone = detail_impl.disclose_flags.telephone;
                contact_detail->disclose_flags.vat = detail_impl.disclose_flags.vat;
                contact_detail->states = CORBA::string_dup(detail_impl.states.c_str());
                contact_detail->state_codes = CORBA::string_dup(detail_impl.state_codes.c_str());

                if(detail_impl.is_owner)
                {
                    auth_result = PRIVATE_DATA;
                }
                else
                {
                    auth_result = PUBLIC_DATA;
                }

                return contact_detail._retn();
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::NSSetDetail* Server_i::getNssetDetail(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& nsset,
            const char* lang,
            Registry::DomainBrowser::DataAccessLevel& auth_result)
        {
            try
            {
                Registry::DomainBrowserImpl::NssetDetail detail_impl
                    = pimpl_->getNssetDetail(contact.id, nsset.id, lang);

                NSSetDetail_var nsset_detail = new NSSetDetail;

                nsset_detail->id = detail_impl.id;
                nsset_detail->handle = CORBA::string_dup(detail_impl.handle.c_str());
                nsset_detail->roid = CORBA::string_dup(detail_impl.roid.c_str());
                nsset_detail->registrar.id = detail_impl.sponsoring_registrar.id;
                nsset_detail->registrar.handle = CORBA::string_dup(detail_impl.sponsoring_registrar.handle.c_str());
                nsset_detail->registrar.name = CORBA::string_dup(detail_impl.sponsoring_registrar.name.c_str());
                nsset_detail->create_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(detail_impl.creation_time.date()).c_str());
                nsset_detail->transfer_date = CORBA::string_dup(detail_impl.transfer_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.transfer_time.get_value().date()).c_str());
                nsset_detail->update_date = CORBA::string_dup(detail_impl.update_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.update_time.get_value().date()).c_str());

                nsset_detail->create_registrar.id = detail_impl.create_registrar.id;
                nsset_detail->create_registrar.handle = CORBA::string_dup(detail_impl.create_registrar.handle.c_str());
                nsset_detail->create_registrar.name = CORBA::string_dup(detail_impl.create_registrar.name.c_str());

                nsset_detail->update_registrar.id = detail_impl.update_registrar.id;
                nsset_detail->update_registrar.handle = CORBA::string_dup(detail_impl.update_registrar.handle.c_str());
                nsset_detail->update_registrar.name = CORBA::string_dup(detail_impl.update_registrar.name.c_str());

                nsset_detail->auth_info = CORBA::string_dup(detail_impl.authinfopw.c_str());

                nsset_detail->admins.length(detail_impl.admins.size());
                for(std::size_t i = 0; i < detail_impl.admins.size(); ++i)
                {
                    nsset_detail->admins[i].id = detail_impl.admins[i].id;
                    nsset_detail->admins[i].handle = CORBA::string_dup(detail_impl.admins[i].handle.c_str());
                    nsset_detail->admins[i].name = CORBA::string_dup(detail_impl.admins[i].name.c_str());
                }

                nsset_detail->hosts.length(detail_impl.hosts.size());
                for(std::size_t i = 0; i < detail_impl.hosts.size(); ++i)
                {
                    nsset_detail->hosts[i].fqdn = CORBA::string_dup(detail_impl.hosts[i].fqdn.c_str());
                    nsset_detail->hosts[i].inet = CORBA::string_dup(detail_impl.hosts[i].inet_addr.c_str());
                }


                nsset_detail->states = CORBA::string_dup(detail_impl.states.c_str());
                nsset_detail->state_codes = CORBA::string_dup(detail_impl.state_codes.c_str());

                nsset_detail->report_level = detail_impl.report_level;

                if(detail_impl.is_owner)
                {
                    auth_result = PRIVATE_DATA;
                }
                else
                {
                    auth_result = PUBLIC_DATA;
                }

                return nsset_detail._retn();
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::DomainDetail* Server_i::getDomainDetail(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& domain,
            const char* lang,
            Registry::DomainBrowser::DataAccessLevel& auth_result)
        {
            try
            {
                Registry::DomainBrowserImpl::DomainDetail detail_impl
                    = pimpl_->getDomainDetail(contact.id, domain.id, lang);

                DomainDetail_var domain_detail = new DomainDetail;
                domain_detail->id = detail_impl.id;
                domain_detail->fqdn = CORBA::string_dup(detail_impl.fqdn.c_str());
                domain_detail->roid = CORBA::string_dup(detail_impl.roid.c_str());
                domain_detail->registrar.id = detail_impl.sponsoring_registrar.id;
                domain_detail->registrar.handle = CORBA::string_dup(detail_impl.sponsoring_registrar.handle.c_str());
                domain_detail->registrar.name = CORBA::string_dup(detail_impl.sponsoring_registrar.name.c_str());
                domain_detail->create_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(detail_impl.creation_time.date()).c_str());
                domain_detail->update_date = CORBA::string_dup(detail_impl.update_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.update_time.get_value().date()).c_str());
                domain_detail->auth_info = CORBA::string_dup(detail_impl.authinfopw.c_str());
                domain_detail->registrant.id = detail_impl.registrant.id;
                domain_detail->registrant.handle = CORBA::string_dup(detail_impl.registrant.handle.c_str());
                domain_detail->registrant.name = CORBA::string_dup(detail_impl.registrant.name.c_str());
                domain_detail->expiration_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(detail_impl.expiration_date).c_str());

                domain_detail->is_enum = !detail_impl.enum_domain_validation.isnull();
                if(domain_detail->is_enum)
                {
                    domain_detail->publish = detail_impl.enum_domain_validation.get_value().publish;
                    domain_detail->val_ex_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(
                        detail_impl.enum_domain_validation.get_value().validation_expiration).c_str());
                }
                else
                {
                    domain_detail->publish = false;
                    domain_detail->val_ex_date = CORBA::string_dup("");
                }

                domain_detail->nsset.id = detail_impl.nsset.id;
                domain_detail->nsset.handle = CORBA::string_dup(detail_impl.nsset.handle.c_str());
                domain_detail->nsset.name = CORBA::string_dup(detail_impl.nsset.name.c_str());

                domain_detail->keyset.id = detail_impl.keyset.id;
                domain_detail->keyset.handle = CORBA::string_dup(detail_impl.keyset.handle.c_str());
                domain_detail->keyset.name = CORBA::string_dup(detail_impl.keyset.name.c_str());

                domain_detail->admins.length(detail_impl.admins.size());

                for(std::size_t i = 0; i < detail_impl.admins.size(); ++i)
                {
                    domain_detail->admins[i].id = detail_impl.admins[i].id;
                    domain_detail->admins[i].handle = CORBA::string_dup(detail_impl.admins[i].handle.c_str());
                    domain_detail->admins[i].name = CORBA::string_dup(detail_impl.admins[i].name.c_str());
                }

                domain_detail->states = CORBA::string_dup(detail_impl.states.c_str());
                domain_detail->state_codes = CORBA::string_dup(detail_impl.state_codes.c_str());

                if(detail_impl.is_owner)
                {
                    auth_result = PRIVATE_DATA;
                }
                else
                {
                    auth_result = PUBLIC_DATA;
                }

                return domain_detail._retn();
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::KeysetDetail* Server_i::getKeysetDetail(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::RegistryReference& keyset,
            const char* lang,
            Registry::DomainBrowser::DataAccessLevel& auth_result)
        {
            try
            {
                Registry::DomainBrowserImpl::KeysetDetail detail_impl
                    = pimpl_->getKeysetDetail(contact.id, keyset.id, lang);

                KeysetDetail_var keyset_detail = new KeysetDetail;

                keyset_detail->id = detail_impl.id;
                keyset_detail->handle = CORBA::string_dup(detail_impl.handle.c_str());
                keyset_detail->roid = CORBA::string_dup(detail_impl.roid.c_str());
                keyset_detail->registrar.id = detail_impl.sponsoring_registrar.id;
                keyset_detail->registrar.handle = CORBA::string_dup(detail_impl.sponsoring_registrar.handle.c_str());
                keyset_detail->registrar.name = CORBA::string_dup(detail_impl.sponsoring_registrar.name.c_str());
                keyset_detail->create_date = CORBA::string_dup(boost::gregorian::to_iso_extended_string(detail_impl.creation_time.date()).c_str());
                keyset_detail->transfer_date = CORBA::string_dup(detail_impl.transfer_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.transfer_time.get_value().date()).c_str());
                keyset_detail->update_date = CORBA::string_dup(detail_impl.update_time.isnull()
                    ? "" : boost::gregorian::to_iso_extended_string(detail_impl.update_time.get_value().date()).c_str());

                keyset_detail->create_registrar.id = detail_impl.create_registrar.id;
                keyset_detail->create_registrar.handle = CORBA::string_dup(detail_impl.create_registrar.handle.c_str());
                keyset_detail->create_registrar.name = CORBA::string_dup(detail_impl.create_registrar.name.c_str());

                keyset_detail->update_registrar.id = detail_impl.update_registrar.id;
                keyset_detail->update_registrar.handle = CORBA::string_dup(detail_impl.update_registrar.handle.c_str());
                keyset_detail->update_registrar.name = CORBA::string_dup(detail_impl.update_registrar.name.c_str());

                keyset_detail->auth_info = CORBA::string_dup(detail_impl.authinfopw.c_str());

                keyset_detail->admins.length(detail_impl.admins.size());
                for(std::size_t i = 0; i < detail_impl.admins.size(); ++i)
                {
                    keyset_detail->admins[i].id = detail_impl.admins[i].id;
                    keyset_detail->admins[i].handle = CORBA::string_dup(detail_impl.admins[i].handle.c_str());
                    keyset_detail->admins[i].name = CORBA::string_dup(detail_impl.admins[i].name.c_str());
                }

                keyset_detail->dnskeys.length(detail_impl.dnskeys.size());
                for(std::size_t i = 0; i < detail_impl.dnskeys.size(); ++i)
                {
                    keyset_detail->dnskeys[i].flags = detail_impl.dnskeys[i].flags;
                    keyset_detail->dnskeys[i].protocol = detail_impl.dnskeys[i].protocol;
                    keyset_detail->dnskeys[i].alg = detail_impl.dnskeys[i].alg;
                    keyset_detail->dnskeys[i].key = CORBA::string_dup(detail_impl.dnskeys[i].key.c_str());
                }

                keyset_detail->states = CORBA::string_dup(detail_impl.states.c_str());
                keyset_detail->state_codes = CORBA::string_dup(detail_impl.state_codes.c_str());

                if(detail_impl.is_owner)
                {
                    auth_result = PRIVATE_DATA;
                }
                else
                {
                    auth_result = PUBLIC_DATA;
                }

                return keyset_detail._retn();
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RegistrarDetail* Server_i::getRegistrarDetail(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* handle)
        {
            try
            {
                Registry::DomainBrowserImpl::RegistrarDetail detail_impl
                    = pimpl_->getRegistrarDetail(contact.id, handle);

                RegistrarDetail_var registrar_detail = new RegistrarDetail;
                registrar_detail->id = detail_impl.id;
                registrar_detail->handle = CORBA::string_dup(detail_impl.handle.c_str());
                registrar_detail->name = CORBA::string_dup(detail_impl.name.c_str());
                registrar_detail->phone = CORBA::string_dup(detail_impl.phone.c_str());
                registrar_detail->fax = CORBA::string_dup(detail_impl.fax.c_str());
                registrar_detail->url = CORBA::string_dup(detail_impl.url.c_str());
                registrar_detail->address = CORBA::string_dup(detail_impl.address.c_str());

                return registrar_detail._retn();
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        ::CORBA::Boolean Server_i::setContactDiscloseFlags(
            const Registry::DomainBrowser::RegistryReference& contact,
            const Registry::DomainBrowser::UpdateContactDiscloseFlags& flags,
            Registry::DomainBrowser::TID request_id)
        {
            try
            {
                Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet flags_;
                flags_.email = flags.email;
                flags_.address = flags.address;
                flags_.telephone = flags.telephone;
                flags_.fax = flags.fax;
                flags_.ident = flags.ident;
                flags_.vat = flags.vat;
                flags_.notify_email = flags.notify_email;
                return pimpl_->setContactDiscloseFlags(contact.id, flags_, request_id);
            }//try
            catch (const Registry::DomainBrowserImpl::ObjectNotExists& )
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }
            catch (const Registry::DomainBrowserImpl::UserNotExists& )
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (const boost::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (const std::exception&)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        ::CORBA::Boolean Server_i::setAuthInfo(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* objtype,
            const Registry::DomainBrowser::RegistryReference& objref,
            const char* auth_info,
            Registry::DomainBrowser::TID request_id)
        {
            try
            {
                return false;
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_BLOCKED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::ACCESS_DENIED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }

            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        ::CORBA::Boolean Server_i::setObjectBlockStatus(
            const Registry::DomainBrowser::RegistryReference& contact,
            const char* objtype,
            const Registry::DomainBrowser::RegistryReferenceSeq& objects,
            Registry::DomainBrowser::ObjectBlockType block,
            Registry::DomainBrowser::RecordSequence_out blocked)
        {
            try
            {
                return false;
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::ACCESS_DENIED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::OBJECT_NOT_EXISTS();
            }

            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::USER_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

        Registry::DomainBrowser::RecordSequence* Server_i::getPublicStatusDesc(const char* lang)
        {
            try
            {
                RecordSequence_var rseq = new RecordSequence;
                rseq->length(1);

                RegistryObject_var robject = CORBA::string_dup("test_object");

                rseq[0] = robject._retn();

                return rseq._retn();
            }//try
            catch (std::exception &_ex)
            {
                throw Registry::DomainBrowser::INCORRECT_USAGE();
            }
            catch (...)
            {
                throw Registry::DomainBrowser::INTERNAL_SERVER_ERROR();
            }
        }

    }//namespace DomainBrowser
}//namespace Registry
