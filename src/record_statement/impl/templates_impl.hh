/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef TEMPLATES_IMPL_HH_1EE16A76AB7E51E0D04BA200F2C8BAFE//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define TEMPLATES_IMPL_HH_1EE16A76AB7E51E0D04BA200F2C8BAFE

#include "src/record_statement/impl/record_statement_xml.hh"
#include "src/record_statement/impl/factory.hh"
#include "src/record_statement/exceptions.hh"

#include "src/fredlib/zone/zone.h"

#include "util/tz/utc.hh"

#include <boost/algorithm/string/join.hpp>

namespace Fred {
namespace RecordStatement {
namespace Impl {

template <typename LOCAL_TIMEZONE>
Tz::LocalTimestamp convert_utc_timestamp_to_local(
        Fred::OperationContext& ctx,
        const boost::posix_time::ptime& utc_timestamp)
{
    const DbDateTimeArithmetic convertor(ctx);
    return Tz::LocalTimestamp::into<LOCAL_TIMEZONE>(convertor, Tz::LocalTimestamp::within_utc(utc_timestamp));
}

template <typename REGISTRY_TIMEZONE>
std::string domain_printout_xml(
        Fred::OperationContext& ctx,
        const Fred::InfoDomainOutput& info,
        const Tz::LocalTimestamp& valid_at,
        const Registry::RecordStatement::Purpose::Enum purpose,
        const Fred::InfoContactOutput& registrant_info,
        const std::vector<Fred::InfoContactOutput>& admin_contact_info,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const boost::optional<NssetPrintoutInputData>& nsset_data,
        const boost::optional<KeysetPrintoutInputData>& keyset_data,
        const std::set<std::string>& external_states)
{
    const Tz::LocalTimestamp local_creation_time =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(ctx, info.info_domain_data.creation_time);
    const boost::optional<Tz::LocalTimestamp> local_update_time = info.info_domain_data.update_time.isnull()
             ? boost::optional<Tz::LocalTimestamp>()
             : convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(
                     ctx, info.info_domain_data.update_time.get_value());

    std::vector<Util::XmlCallback> admin_contact_list;
    admin_contact_list.reserve(admin_contact_info.size());

    for (std::vector<Fred::InfoContactOutput>::const_iterator itr = admin_contact_info.begin();
         itr != admin_contact_info.end(); ++itr)
    {
        admin_contact_list.push_back(
                Util::XmlTagPair("admin_contact", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(itr->info_contact_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(itr->info_contact_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization",
                                      Util::XmlEscapeTag(itr->info_contact_data.organization.get_value_or(""))))));
    }

    std::ostringstream private_printout_attr;
    private_printout_attr << "is_private_printout='" << std::boolalpha
                          << (purpose == Registry::RecordStatement::Purpose::private_printout) << "'";

    std::ostringstream disclose_flags;
    disclose_flags << std::boolalpha
        << "name='" << registrant_info.info_contact_data.disclosename << "' "
           "organization='" << registrant_info.info_contact_data.discloseorganization << "' "
           "address='" << registrant_info.info_contact_data.discloseaddress << "' "
           "telephone='" << registrant_info.info_contact_data.disclosetelephone << "' "
           "fax='" << registrant_info.info_contact_data.disclosefax << "' "
           "email='" << registrant_info.info_contact_data.discloseemail << "' "
           "vat='" << registrant_info.info_contact_data.disclosevat << "' "
           "ident='" << registrant_info.info_contact_data.discloseident << "' "
           "notifyemail='" << registrant_info.info_contact_data.disclosenotifyemail << "'";

    const DbDateTimeArithmetic datetime_convertor(ctx);
    std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    Util::XmlTagPair(
            "record_statement",
            Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair(
                        "current_datetime",
                        Util::XmlEscapeTag(Tz::LocalTimestamp::into<REGISTRY_TIMEZONE>(
                                datetime_convertor,
                                valid_at).get_rfc3339_formated_string())))
                (Util::XmlTagPair(
                        "domain",
                        Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("fqdn", Util::XmlEscapeTag(info.info_domain_data.fqdn)))
                            (Util::XmlTagPair("creation_date", Util::XmlEscapeTag(
                                    boost::gregorian::to_iso_extended_string(local_creation_time.get_local_time().date()))))
                            (Util::XmlTagPair("last_update_date", Util::XmlEscapeTag(
                                    local_update_time
                                        ? boost::gregorian::to_iso_extended_string(local_update_time->get_local_time().date())
                                        : std::string())))
                            (Util::XmlTagPair("expiration_date", Util::XmlEscapeTag(
                                    boost::gregorian::to_iso_extended_string(info.info_domain_data.expiration_date))))
                            (Util::XmlTagPair(
                                    "holder",
                                    Util::vector_of<Util::XmlCallback>
                                        (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.handle)))
                                        (Util::XmlTagPair("name", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.name.get_value_or(""))))
                                        (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.organization.get_value_or(""))))
                                        (Util::XmlTagPair("id_number", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.ssntype.get_value_or("") == "ICO"
                                                    ? registrant_info.info_contact_data.ssn.get_value_or("")
                                                    : std::string())))
                                        (Util::XmlTagPair("street1", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().street1)))
                                        (Util::XmlTagPair("street2", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().street2.get_value_or(""))))
                                        (Util::XmlTagPair("street3", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().street3.get_value_or(""))))
                                        (Util::XmlTagPair("city", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().city)))
                                        (Util::XmlTagPair("stateorprovince", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().stateorprovince.get_value_or(""))))
                                        (Util::XmlTagPair("postal_code", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().postalcode)))
                                        (Util::XmlTagPair("country", Util::XmlEscapeTag(
                                                registrant_info.info_contact_data.place.get_value().country)))
                                        (Util::XmlTagPair("disclose", "", disclose_flags.str())),
                                    private_printout_attr.str()))
                            (Util::XmlTagPair("admin_contact_list", admin_contact_list))
                            (Util::XmlTagPair(
                                    "sponsoring_registrar",
                                    Util::vector_of<Util::XmlCallback>
                                        (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.handle)))
                                        (Util::XmlTagPair("name", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.name.get_value_or(""))))
                                        (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.organization.get_value_or(""))))))
                            (nsset_xml(nsset_data))
                            (keyset_xml(keyset_data))
                            (Util::XmlTagPair("external_states_list", external_states_xml(external_states))))))
    (printout_xml);

    return printout_xml;
}

template <typename REGISTRY_TIMEZONE>
std::string contact_printout_xml(
        Fred::OperationContext& ctx,
        const Fred::InfoContactOutput& info,
        const Tz::LocalTimestamp& valid_at,
        Registry::RecordStatement::Purpose::Enum purpose,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const std::set<std::string>& external_states)
{
    const Tz::LocalTimestamp local_creation_time =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(
                    ctx, info.info_contact_data.creation_time);
    const boost::optional<Tz::LocalTimestamp> local_update_time = info.info_contact_data.update_time.isnull()
            ? boost::optional<Tz::LocalTimestamp>()
            : convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(
                    ctx,
                    info.info_contact_data.update_time.get_value());
    const boost::optional<Tz::LocalTimestamp> local_transfer_time = info.info_contact_data.transfer_time.isnull()
            ? boost::optional<Tz::LocalTimestamp>()
            : convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(
                    ctx,
                    info.info_contact_data.transfer_time.get_value());

    std::ostringstream private_printout_attr;
    private_printout_attr << "is_private_printout='" << std::boolalpha
                          << (purpose == Registry::RecordStatement::Purpose::private_printout) << "'";

    std::ostringstream disclose_flags;
    disclose_flags << std::boolalpha
        << "name='" << info.info_contact_data.disclosename << "' "
           "organization='" << info.info_contact_data.discloseorganization << "' "
           "address='" << info.info_contact_data.discloseaddress << "' "
           "telephone='" << info.info_contact_data.disclosetelephone << "' "
           "fax='" << info.info_contact_data.disclosefax << "' "
           "email='" << info.info_contact_data.discloseemail << "' "
           "vat='" << info.info_contact_data.disclosevat << "' "
           "ident='" << info.info_contact_data.discloseident << "' "
           "notifyemail='" << info.info_contact_data.disclosenotifyemail << "'";

    const DbDateTimeArithmetic datetime_convertor(ctx);
    std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    Util::XmlTagPair(
            "record_statement",
            Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair(
                        "current_datetime",
                        Util::XmlEscapeTag(Tz::LocalTimestamp::into<REGISTRY_TIMEZONE>(
                                datetime_convertor,
                                valid_at).get_rfc3339_formated_string())))
                (Util::XmlTagPair(
                        "contact",
                        Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                                    info.info_contact_data.handle)))
                            (Util::XmlTagPair("name", Util::XmlEscapeTag(
                                    info.info_contact_data.name.get_value_or(""))))
                            (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                                    info.info_contact_data.organization.get_value_or(""))))
                            (Util::XmlTagPair("taxpayer_id_number", Util::XmlEscapeTag(
                                    info.info_contact_data.vat.get_value_or(""))))
                            (Util::XmlTagPair("id_type", Util::XmlEscapeTag(
                                    info.info_contact_data.ssntype.get_value_or(""))))
                            (Util::XmlTagPair("id_value", Util::XmlEscapeTag(
                                    info.info_contact_data.ssn.get_value_or(""))))
                            (Util::XmlTagPair("email", Util::XmlEscapeTag(
                                    info.info_contact_data.email.get_value_or(""))))
                            (Util::XmlTagPair("notification_email", Util::XmlEscapeTag(
                                    info.info_contact_data.notifyemail.get_value_or(""))))
                            (Util::XmlTagPair("phone", Util::XmlEscapeTag(
                                    info.info_contact_data.telephone.get_value_or(""))))
                            (Util::XmlTagPair("fax", Util::XmlEscapeTag(
                                    info.info_contact_data.fax.get_value_or(""))))
                            (Util::XmlTagPair("creation_date", Util::XmlEscapeTag(
                                    boost::gregorian::to_iso_extended_string(local_creation_time.get_local_time().date()))))
                            (Util::XmlTagPair("last_update_date", Util::XmlEscapeTag(
                                    local_update_time
                                        ? boost::gregorian::to_iso_extended_string(local_update_time->get_local_time().date())
                                        : std::string())))
                            (Util::XmlTagPair("last_transfer_date", Util::XmlEscapeTag(
                                    local_transfer_time
                                        ? boost::gregorian::to_iso_extended_string(local_transfer_time->get_local_time().date())
                                        : std::string())))
                            (Util::XmlTagPair(
                                    "address",
                                    Util::vector_of<Util::XmlCallback>
                                        (Util::XmlTagPair("street1", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().street1)))
                                        (Util::XmlTagPair("street2", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().street2.get_value_or(""))))
                                        (Util::XmlTagPair("street3", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().street3.get_value_or(""))))
                                        (Util::XmlTagPair("city", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().city)))
                                        (Util::XmlTagPair("stateorprovince", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().stateorprovince.get_value_or(""))))
                                        (Util::XmlTagPair("postal_code", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().postalcode)))
                                        (Util::XmlTagPair("country", Util::XmlEscapeTag(
                                                info.info_contact_data.place.get_value().country)))))
                            (Util::XmlTagPair(
                                    "sponsoring_registrar",
                                    Util::vector_of<Util::XmlCallback>
                                        (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.handle)))
                                        (Util::XmlTagPair("name", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.name.get_value_or(""))))
                                        (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                                                sponsoring_registrar_info.info_registrar_data.organization.get_value_or(""))))))
                            (Util::XmlTagPair("disclose", "", disclose_flags.str()))
                            (Util::XmlTagPair("external_states_list", external_states_xml(external_states))),
                        private_printout_attr.str())))
    (printout_xml);

    return printout_xml;
}

template <typename REGISTRY_TIMEZONE>
XmlWithData domain_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& fqdn,
        Registry::RecordStatement::Purpose::Enum purpose)
{
    Fred::InfoDomainOutput info_domain_output;
    try
    {
        info_domain_output = Fred::InfoDomainByHandle(Fred::Zone::rem_trailing_dot(fqdn)).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());
    }
    catch (const Fred::InfoDomainByHandle::Exception& e)
    {
        if (e.is_set_unknown_fqdn())
        {
            throw Registry::RecordStatement::ObjectNotFound();
        }
        //other error
        throw;
    }

    const Fred::InfoContactOutput info_registrant_output = Fred::InfoContactByHandle(
            info_domain_output.info_domain_data.registrant.handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    XmlWithData retval;
    if (!info_registrant_output.info_contact_data.email.isnull())
    {
        retval.email_out.insert(info_registrant_output.info_contact_data.email.get_value());
    }

    const Fred::InfoRegistrarOutput info_sponsoring_registrar_output =
            Fred::InfoRegistrarByHandle(
                    info_domain_output.info_domain_data.sponsoring_registrar_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    std::vector<Fred::InfoContactOutput> info_admin_contact_output;
    info_admin_contact_output.reserve(info_domain_output.info_domain_data.admin_contacts.size());

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr =
            info_domain_output.info_domain_data.admin_contacts.begin();
         itr != info_domain_output.info_domain_data.admin_contacts.end(); ++itr)
    {
        info_admin_contact_output.push_back(
                Fred::InfoContactByHandle(itr->handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    const boost::optional<std::string> nsset_handle =
            info_domain_output.info_domain_data.nsset.isnull()
                ? boost::optional<std::string>()
                : boost::optional<std::string>(info_domain_output.info_domain_data.nsset.get_value().handle);

    const boost::optional<NssetPrintoutInputData> nsset_data = make_nsset_data(nsset_handle, ctx);

    const boost::optional<std::string> keyset_handle =
            info_domain_output.info_domain_data.keyset.isnull()
                ? boost::optional<std::string>()
                : boost::optional<std::string>(info_domain_output.info_domain_data.keyset.get_value().handle);

    const boost::optional<KeysetPrintoutInputData> keyset_data = make_keyset_data(keyset_handle, ctx);

    retval.request_local_timestamp =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(ctx, info_domain_output.utc_timestamp);

    retval.xml =
            domain_printout_xml<REGISTRY_TIMEZONE>(
                    ctx,
                    info_domain_output,
                    retval.request_local_timestamp,
                    purpose,
                    info_registrant_output,
                    info_admin_contact_output,
                    info_sponsoring_registrar_output,
                    nsset_data,
                    keyset_data,
                    make_external_states(info_domain_output.info_domain_data.id, ctx));

    ctx.get_log().debug(retval.xml);
    return retval;
}

template <typename REGISTRY_TIMEZONE>
XmlWithData nsset_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle)
{
    NssetPrintoutInputData nsset_data;

    try
    {
        nsset_data = *make_nsset_data(handle, ctx);
    }
    catch (const Fred::InfoNssetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw Registry::RecordStatement::ObjectNotFound();
        }
        //other error
        throw;
    }

    XmlWithData retval;
    for (std::vector<Fred::InfoContactOutput>::const_iterator itr = nsset_data.tech_contact.begin();
         itr != nsset_data.tech_contact.end(); ++itr)
    {
        if (!itr->info_contact_data.email.isnull())
        {
            retval.email_out.insert(itr->info_contact_data.email.get_value());
        }
    }

    retval.request_local_timestamp =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(ctx, nsset_data.info.utc_timestamp);

    retval.xml = nsset_printout_xml(
            nsset_data,
            retval.request_local_timestamp);

    ctx.get_log().debug(retval.xml);
    return retval;
}

template <typename REGISTRY_TIMEZONE>
XmlWithData keyset_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle)
{
    KeysetPrintoutInputData keyset_data;

    try
    {
        keyset_data = *make_keyset_data(handle, ctx);
    }
    catch (const Fred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw Registry::RecordStatement::ObjectNotFound();
        }
        //other error
        throw;
    }

    XmlWithData retval;
    for (std::vector<Fred::InfoContactOutput>::const_iterator itr = keyset_data.tech_contact.begin();
         itr != keyset_data.tech_contact.end(); ++itr)
    {
        if (!itr->info_contact_data.email.isnull())
        {
            retval.email_out.insert(itr->info_contact_data.email.get_value());
        }
    }

    retval.request_local_timestamp =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(ctx, keyset_data.info.utc_timestamp);

    retval.xml = keyset_printout_xml(
            keyset_data,
            retval.request_local_timestamp);

    ctx.get_log().debug(retval.xml);
    return retval;
}

template <typename REGISTRY_TIMEZONE>
XmlWithData contact_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle,
        Registry::RecordStatement::Purpose::Enum purpose)
{
    Fred::InfoContactOutput info_contact_output;

    try
    {
        info_contact_output = Fred::InfoContactByHandle(handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());
    }
    catch (const Fred::InfoContactByHandle::Exception& e)
    {
        if (e.is_set_unknown_contact_handle())
        {
            throw Registry::RecordStatement::ObjectNotFound();
        }
        //other error
        throw;
    }

    XmlWithData retval;
    if (!info_contact_output.info_contact_data.email.isnull())
    {
        retval.email_out.insert(info_contact_output.info_contact_data.email.get_value());
    }

    const Fred::InfoRegistrarOutput info_sponsoring_registrar_output =
            Fred::InfoRegistrarByHandle(
                    info_contact_output.info_contact_data.sponsoring_registrar_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    retval.request_local_timestamp =
            convert_utc_timestamp_to_local<REGISTRY_TIMEZONE>(ctx, info_contact_output.utc_timestamp);

    retval.xml =
            contact_printout_xml<REGISTRY_TIMEZONE>(
                    ctx,
                    info_contact_output,
                    retval.request_local_timestamp,
                    purpose,
                    info_sponsoring_registrar_output,
                    make_external_states(info_contact_output.info_contact_data.id, ctx));

    ctx.get_log().debug(retval.xml);
    return retval;
}

template <typename T>
ImplementationWithin<T>::ImplementationWithin(
        const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
        const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager)
    : doc_manager_(_doc_manager),
      mailer_manager_(_mailer_manager)
{
}

template <typename T>
ImplementationWithin<T>::~ImplementationWithin()
{}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::domain_printout(
        Fred::OperationContext& _ctx,
        const std::string& _fqdn,
        Registry::RecordStatement::Purpose::Enum _purpose)const
{
    std::stringstream xml_document;
    xml_document << domain_printout_xml_with_data<RegistryTimeZone>(
            _ctx,
            _fqdn,
            _purpose).xml;

    std::ostringstream pdf_document;
    doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_DOMAIN, xml_document, pdf_document, "");

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::nsset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle)const
{
    std::stringstream xml_document;
    xml_document << nsset_printout_xml_with_data<RegistryTimeZone>(_ctx, _handle).xml;

    std::ostringstream pdf_document;
    doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_NSSET, xml_document, pdf_document, "");

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::keyset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle)const
{
    std::stringstream xml_document;
    xml_document << keyset_printout_xml_with_data<RegistryTimeZone>(_ctx, _handle).xml;

    std::ostringstream pdf_document;
    doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_KEYSET, xml_document, pdf_document, "");

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::contact_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        Registry::RecordStatement::Purpose::Enum _purpose)const
{
    std::stringstream xml_document;
    xml_document << contact_printout_xml_with_data<RegistryTimeZone>(
            _ctx,
            _handle,
            _purpose).xml;

    std::ostringstream pdf_document;
    doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_CONTACT, xml_document, pdf_document, "");

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::historic_domain_printout(
        Fred::OperationContext& _ctx,
        const std::string& _fqdn,
        const Tz::LocalTimestamp& _valid_at)const
{
    const Fred::InfoDomainOutput info_domain_output =
            Fred::InfoDomainHistoryByHistoryid(
                    get_history_id_of<Fred::Object_Type::domain>(_fqdn, _valid_at, _ctx))
            .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>());

    const Fred::InfoContactOutput info_registrant_output =
            Fred::InfoContactHistoryByHistoryid(
                    get_history_id_internal_of<Fred::Object_Type::contact>(
                            info_domain_output.info_domain_data.registrant.handle, _valid_at, _ctx))
            .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>());

    const Fred::InfoRegistrarOutput info_sponsoring_registrar_output =
            Fred::InfoRegistrarByHandle(info_domain_output.info_domain_data.sponsoring_registrar_handle)
            .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>());

    std::vector<Fred::InfoContactOutput> info_admin_contact_output;

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr =
                 info_domain_output.info_domain_data.admin_contacts.begin();
         itr != info_domain_output.info_domain_data.admin_contacts.end(); ++itr)
    {
        info_admin_contact_output.push_back(
                Fred::InfoContactHistoryByHistoryid(get_history_id_internal_of<Fred::Object_Type::contact>(
                        itr->handle, _valid_at, _ctx))
                .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    const boost::optional<unsigned long long> nsset_historyid = info_domain_output.info_domain_data.nsset.isnull()
        ? boost::optional<unsigned long long>()
        : get_history_id_internal_of<Fred::Object_Type::nsset>(
                info_domain_output.info_domain_data.nsset.get_value().handle, _valid_at, _ctx);

    const boost::optional<NssetPrintoutInputData> nsset_data =
            make_historic_nsset_data(nsset_historyid, _valid_at, _ctx);

    const boost::optional<unsigned long long> keyset_historyid = info_domain_output.info_domain_data.keyset.isnull()
        ? boost::optional<unsigned long long>()
        : get_history_id_internal_of<Fred::Object_Type::keyset>(
                info_domain_output.info_domain_data.keyset.get_value().handle, _valid_at, _ctx);

    const boost::optional<KeysetPrintoutInputData> keyset_data =
            make_historic_keyset_data(keyset_historyid, _valid_at, _ctx);

    const std::string xml_document = domain_printout_xml<RegistryTimeZone>(
            _ctx,
            info_domain_output,
            _valid_at,
            Registry::RecordStatement::Purpose::private_printout,
            info_registrant_output,
            info_admin_contact_output,
            info_sponsoring_registrar_output,
            nsset_data,
            keyset_data,
            make_historic_external_states(
                    info_domain_output.info_domain_data.id,
                    _valid_at,
                    _ctx));

    _ctx.get_log().debug(xml_document);

    std::ostringstream pdf_document;
    boost::shared_ptr<Fred::Document::Generator> doc_gen(
        doc_manager_->createOutputGenerator(Fred::Document::GT_RECORD_STATEMENT_DOMAIN, pdf_document, "").release());

    doc_gen->getInput() << xml_document;
    doc_gen->closeInput();

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::historic_nsset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    const NssetPrintoutInputData nsset_data =
            *make_historic_nsset_data(
                    get_history_id_of<Fred::Object_Type::nsset>(_handle, _valid_at, _ctx),
                    _valid_at,
                    _ctx);

    const DbDateTimeArithmetic datetime_convertor(_ctx);
    const std::string xml_document =
            nsset_printout_xml(
                    nsset_data,
                    Tz::LocalTimestamp::into<RegistryTimeZone>(
                            datetime_convertor,
                            _valid_at));

    _ctx.get_log().debug(xml_document);

    std::ostringstream pdf_document;
    boost::shared_ptr<Fred::Document::Generator> doc_gen(
            doc_manager_->createOutputGenerator(
                    Fred::Document::GT_RECORD_STATEMENT_NSSET, pdf_document, "").release());

    doc_gen->getInput() << xml_document;
    doc_gen->closeInput();

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::historic_keyset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    const KeysetPrintoutInputData keyset_data =
            *make_historic_keyset_data(
                    get_history_id_of<Fred::Object_Type::keyset>(_handle, _valid_at, _ctx),
                    _valid_at,
                    _ctx);

    const DbDateTimeArithmetic datetime_convertor(_ctx);
    const std::string xml_document =
            keyset_printout_xml(
                    keyset_data,
                    Tz::LocalTimestamp::into<RegistryTimeZone>(
                            datetime_convertor,
                            _valid_at));

    _ctx.get_log().debug(xml_document);

    std::ostringstream pdf_document;
    boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                    Fred::Document::GT_RECORD_STATEMENT_KEYSET, pdf_document, "").release());

    doc_gen->getInput() << xml_document;
    doc_gen->closeInput();

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
Registry::RecordStatement::PdfBufferImpl ImplementationWithin<T>::historic_contact_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    const Fred::InfoContactOutput info_contact_output =
            Fred::InfoContactHistoryByHistoryid(
                    get_history_id_of<Fred::Object_Type::contact>(_handle, _valid_at, _ctx))
            .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>());

    const Fred::InfoRegistrarOutput info_sponsoring_registrar_output =
            Fred::InfoRegistrarByHandle(info_contact_output.info_contact_data.sponsoring_registrar_handle)
            .exec(_ctx, Tz::get_psql_handle_of<Tz::UTC>());

    const std::string xml_document = contact_printout_xml<RegistryTimeZone>(
            _ctx,
            info_contact_output,
            _valid_at,
            Registry::RecordStatement::Purpose::private_printout,
            info_sponsoring_registrar_output,
            make_historic_external_states(
                    info_contact_output.info_contact_data.id, _valid_at, _ctx));

    _ctx.get_log().debug(xml_document);

    std::ostringstream pdf_document;
    boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                    Fred::Document::GT_RECORD_STATEMENT_CONTACT, pdf_document, "").release());

    doc_gen->getInput() << xml_document;
    doc_gen->closeInput();

    return Registry::RecordStatement::PdfBufferImpl(pdf_document.str());
}

template <typename T>
void ImplementationWithin<T>::send_domain_printout(
        Fred::OperationContext& _ctx,
        const std::string& _fqdn,
        Registry::RecordStatement::Purpose::Enum _purpose)const
{
    const XmlWithData printout_data =
            domain_printout_xml_with_data<RegistryTimeZone>(
                    _ctx,
                    _fqdn,
                    _purpose);
    std::stringstream xml_document;
    xml_document << printout_data.xml;

    static const int record_statement_filetype = 11;
    const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_DOMAIN,
            xml_document,
            "registry_record_statement.pdf",
            record_statement_filetype,
            "");
    if (file_id == 0)
    {
        throw std::runtime_error("missing file_id");
    }

    Fred::Mailer::Parameters params;
    params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().day()));
    params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().month()));
    params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().year()));

    Fred::Mailer::Handles handles;
    Fred::Mailer::Attachments attach;

    attach.push_back(file_id);

    const std::string registrant_email_out = printout_data.email_out.empty()
            ? std::string()
            : *printout_data.email_out.begin();
    const unsigned long long mail_id = mailer_manager_->sendEmail(
            "", // default sender according to template
            registrant_email_out,
            "", // default subject according to template
            "record_statement",
            params,
            handles,
            attach);

    if (mail_id == 0)
    {
        throw std::runtime_error("sendEmail failed");
    }
}

template <typename T>
void ImplementationWithin<T>::send_nsset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle)const
{
    const XmlWithData printout_data =
            nsset_printout_xml_with_data<RegistryTimeZone>(
                    _ctx,
                    _handle);
    std::stringstream xml_document;
    xml_document << printout_data.xml;

    static const int record_statement_filetype = 11;
    const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_NSSET,
            xml_document,
            "registry_record_statement.pdf",
            record_statement_filetype,
            "");
    if (file_id == 0)
    {
        throw std::runtime_error("missing file_id");
    }

    Fred::Mailer::Parameters params;
    params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().day()));
    params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().month()));
    params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().year()));

    Fred::Mailer::Handles handles;
    Fred::Mailer::Attachments attach;

    attach.push_back(file_id);

    std::string emails = boost::join(printout_data.email_out, ",");

    if (mailer_manager_->checkEmailList(emails))
    {
        const unsigned long long mail_id = mailer_manager_->sendEmail(
                "", // default sender according to template
                emails,
                "", // default subject according to template
                "record_statement",
                params,
                handles,
                attach);

        if (mail_id == 0)
        {
            throw std::runtime_error("sendEmail failed");
        }
    }
}

template <typename T>
void ImplementationWithin<T>::send_keyset_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle)const
{
    const XmlWithData printout_data =
            keyset_printout_xml_with_data<RegistryTimeZone>(
                    _ctx,
                    _handle);
    std::stringstream xml_document;
    xml_document << printout_data.xml;

    static const int record_statement_filetype = 11;
    const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_KEYSET,
            xml_document,
            "registry_record_statement.pdf",
            record_statement_filetype,
            "");
    if (file_id == 0)
    {
        throw std::runtime_error("missing file_id");
    }

    Fred::Mailer::Parameters params;
    params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().day()));
    params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().month()));
    params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().year()));

    Fred::Mailer::Handles handles;
    Fred::Mailer::Attachments attach;

    attach.push_back(file_id);

    std::string emails = boost::join(printout_data.email_out, ",");

    if (mailer_manager_->checkEmailList(emails))
    {
        const unsigned long long mail_id = mailer_manager_->sendEmail(
                "", // default sender according to template
                emails,
                "", // default subject according to template
                "record_statement",
                params,
                handles,
                attach);

        if (mail_id == 0)
        {
            throw std::runtime_error("sendEmail failed");
        }
    }
}

template <typename T>
void ImplementationWithin<T>::send_contact_printout(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        Registry::RecordStatement::Purpose::Enum _purpose)const
{
    const XmlWithData printout_data =
            contact_printout_xml_with_data<RegistryTimeZone>(
                    _ctx,
                    _handle,
                    _purpose);
    std::stringstream xml_document;
    xml_document << printout_data.xml;

    static const int record_statement_filetype = 11;
    const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_CONTACT,
            xml_document,
            "registry_record_statement.pdf",
            record_statement_filetype,
            "");
    if (file_id == 0)
    {
        throw std::runtime_error("missing file_id");
    }

    Fred::Mailer::Parameters params;
    params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().day()));
    params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().month()));
    params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(printout_data.request_local_timestamp.get_local_time().date().year()));

    Fred::Mailer::Handles handles;
    Fred::Mailer::Attachments attach;
    attach.push_back(file_id);
    const std::string email = printout_data.email_out.empty()
            ? std::string()
            : *printout_data.email_out.begin();

    const unsigned long long mail_id = mailer_manager_->sendEmail(
            "", // default sender according to template
            email,
            "", // default subject according to template
            "record_statement",
            params,
            handles,
            attach);

    if (mail_id == 0)
    {
        throw std::runtime_error("sendEmail failed");
    }
}

template <typename REGISTRY_TIMEZONE>
class InstanceOfNecessaryImpl
{
private:
    static std::string fake_domain_printout_xml(
            Fred::OperationContext& ctx,
            const Fred::InfoDomainOutput& info,
            const Tz::LocalTimestamp& valid_at,
            const Registry::RecordStatement::Purpose::Enum purpose,
            const Fred::InfoContactOutput& registrant_info,
            const std::vector<Fred::InfoContactOutput>& admin_contact_info,
            const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
            const boost::optional<NssetPrintoutInputData>& nsset_data,
            const boost::optional<KeysetPrintoutInputData>& keyset_data,
            const std::set<std::string>& external_states)
    {
        return domain_printout_xml<REGISTRY_TIMEZONE>(
                ctx,
                info,
                valid_at,
                purpose,
                registrant_info,
                admin_contact_info,
                sponsoring_registrar_info,
                nsset_data,
                keyset_data,
                external_states);
    }
    static std::string fake_contact_printout_xml(
            Fred::OperationContext& ctx,
            const Fred::InfoContactOutput& info,
            const Tz::LocalTimestamp& valid_at,
            Registry::RecordStatement::Purpose::Enum purpose,
            const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
            const std::set<std::string>& external_states)
    {
        return contact_printout_xml<REGISTRY_TIMEZONE>(
                ctx,
                info,
                valid_at,
                purpose,
                sponsoring_registrar_info,
                external_states);
    }
    static XmlWithData fake_domain_printout_xml_with_data(
            Fred::OperationContext& ctx,
            const std::string& fqdn,
            Registry::RecordStatement::Purpose::Enum purpose)
    {
        return domain_printout_xml_with_data<REGISTRY_TIMEZONE>(
                ctx,
                fqdn,
                purpose);
    }
    static XmlWithData fake_nsset_printout_xml_with_data(
            Fred::OperationContext& ctx,
            const std::string& handle)
    {
        return nsset_printout_xml_with_data<REGISTRY_TIMEZONE>(
                ctx,
                handle);
    }
    static XmlWithData fake_keyset_printout_xml_with_data(
            Fred::OperationContext& ctx,
            const std::string& handle)
    {
        return keyset_printout_xml_with_data<REGISTRY_TIMEZONE>(
                ctx,
                handle);
    }
    static XmlWithData fake_contact_printout_xml_with_data(
            Fred::OperationContext& ctx,
            const std::string& handle,
            Registry::RecordStatement::Purpose::Enum purpose)
    {
        return contact_printout_xml_with_data<REGISTRY_TIMEZONE>(
                ctx,
                handle,
                purpose);
    }

    static Factory::Product producer(
            const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
            const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager)
    {
        return Factory::Product(
                new ImplementationWithin<REGISTRY_TIMEZONE>(_doc_manager, _mailer_manager));
    }

    struct RegistryProducer
    {
        RegistryProducer()
        {
            Factory::register_producer(Tz::get_psql_handle_of<REGISTRY_TIMEZONE>(), producer);
        }
    };
    static const RegistryProducer auto_registry_producer;
};

template <typename T>
const typename InstanceOfNecessaryImpl<T>::RegistryProducer InstanceOfNecessaryImpl<T>::auto_registry_producer;

}//namespace Fred::RecordStatement::Impl
}//namespace Fred::RecordStatement
}//namespace Fred

#endif//TEMPLATES_IMPL_HH_1EE16A76AB7E51E0D04BA200F2C8BAFE
