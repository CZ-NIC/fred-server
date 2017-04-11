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

/**
 *  @file
 *  registry record statement implementation
 */

#include "record_statement.hh"

#include "src/record_statement/impl/record_statement_xml.hh"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/log/context.h"
#include "util/util.h"
#include "util/subprocess.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>



namespace Registry
{
namespace RecordStatement
{

    PdfBufferImpl::PdfBufferImpl(const std::string& s)
    : value(s)
    {
    }

    RecordStatementImpl::RecordStatementImpl(
        const std::string &server_name,
        boost::shared_ptr<Fred::Document::Manager> doc_manager,
        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager,
        const std::string& registry_timezone)
    : server_name_(server_name),
      doc_manager_(doc_manager),
      mailer_manager_(mailer_manager),
      registry_timezone_(registry_timezone)
    {
    }
    RecordStatementImpl::~RecordStatementImpl()
    {}

    std::string RecordStatementImpl::get_server_name() const
    {
        return server_name_;
    }

    PdfBufferImpl RecordStatementImpl::domain_printout(
        const std::string& fqdn,
        bool is_private_printout,
        Fred::OperationContext* ex_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ex_ctx);

        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::domain_printout_xml_with_data(
            fqdn, registry_timezone_, is_private_printout, ctx);

        std::ostringstream pdf_document;
        doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_DOMAIN, xml_document, pdf_document, "");

        return PdfBufferImpl(pdf_document.str());
    }

    PdfBufferImpl RecordStatementImpl::nsset_printout(
        const std::string& handle,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::nsset_printout_xml_with_data(handle, registry_timezone_, ctx);

        std::ostringstream pdf_document;
        doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_NSSET, xml_document, pdf_document, "");

        return PdfBufferImpl(pdf_document.str());
    }


    PdfBufferImpl RecordStatementImpl::keyset_printout(
        const std::string& handle,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::keyset_printout_xml_with_data(handle, registry_timezone_, ctx);

        std::ostringstream pdf_document;
        doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_KEYSET, xml_document, pdf_document, "");

        return PdfBufferImpl(pdf_document.str());
    }

    PdfBufferImpl RecordStatementImpl::contact_printout(
        const std::string& handle,
        bool is_private_printout,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::contact_printout_xml_with_data(
            handle, registry_timezone_, is_private_printout, ctx);

        std::ostringstream pdf_document;
        doc_manager_->generateDocument(Fred::Document::GT_RECORD_STATEMENT_CONTACT, xml_document, pdf_document, "");

        return PdfBufferImpl(pdf_document.str());
    }

    unsigned long long get_history_id(
        const Fred::Object_Type::Enum objtype,
        const std::string& object_name, //if not found, throws Registry::RecordStatement::ObjectNotFound
        const std::string& timestamp,//if not valid, throws Registry::RecordStatement::InvalidTimestamp
        Fred::OperationContext& ctx)
    {
        Database::ParamQuery historic_object_timestamp = Fred::RecordStatement::make_utc_timestamp_query(timestamp);
        try
        {
            ctx.get_log().debug(
            static_cast<std::string>(ctx.get_conn().exec_params(
                Database::ParamQuery("SELECT ")(historic_object_timestamp))[0][0]));
        }
        catch(const Database::ResultFailed&)
        {
            throw Registry::RecordStatement::InvalidTimestamp();
        }


        Conversion::Enums::to_db_handle(Fred::Object_Type::domain);

        Database::ParamQuery history_id_query;
        history_id_query("SELECT h.id"
            " FROM object_registry obr"
            " JOIN history h ON obr.historyid = h.id"
                " AND obr.type = get_object_type_id(").param_text(Conversion::Enums::to_db_handle(objtype))(")"
                " AND obr.name = CASE WHEN ").param_text(Conversion::Enums::to_db_handle(objtype))(" = 'domain'::text"
                    " THEN LOWER(").param_text(Fred::Zone::rem_trailing_dot(object_name))(") "
                    " ELSE UPPER(").param_text(object_name)(") END"
                " AND (date_trunc('second', h.valid_from) <= ")(historic_object_timestamp)(
                " AND (date_trunc('second', h.valid_to) > ")(historic_object_timestamp)(
                    " OR h.valid_to IS NULL))"
            " ORDER BY h.id DESC LIMIT 1"
            );

        Database::Result history_id_res = ctx.get_conn().exec_params(history_id_query);

        if(history_id_res.size() == 0)
        {
            throw Registry::RecordStatement::ObjectNotFound();
        }

        return static_cast<unsigned long long> (history_id_res[0][0]);
    }

    unsigned long long get_history_id_internal( //throws std::runtime_error in case of failure
        const Fred::Object_Type::Enum objtype,
        const std::string& object_name,
        const std::string& timestamp,
        Fred::OperationContext& ctx)
    {
        try
        {
            return get_history_id(objtype, object_name, timestamp, ctx);
        }
        catch(const std::exception& ex)
        {
            throw std::runtime_error(ex.what());
        }

    }

    boost::optional<Fred::RecordStatement::NssetPrintoutInputData> make_hystoric_nsset_data(
        boost::optional<unsigned long long> nsset_historyid,
        const std::string& timestamp, //for nsset tech contact and nsset states history
        Fred::OperationContext& ctx)
    {
        boost::optional<Fred::RecordStatement::NssetPrintoutInputData> ret;

        if(nsset_historyid.is_initialized())
        {
            Fred::RecordStatement::NssetPrintoutInputData nd;
            nd.info = Fred::InfoNssetHistoryByHistoryid(
                    *nsset_historyid).exec(ctx, "UTC");

            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
                it = nd.info.info_nsset_data.tech_contacts.begin();
                it != nd.info.info_nsset_data.tech_contacts.end(); ++it)
            {
                nd.tech_contact.push_back(Fred::InfoContactHistoryByHistoryid(
                    get_history_id_internal(Fred::Object_Type::contact,
                    it->handle, timestamp, ctx)).exec(ctx, "UTC"));
            }

            nd.sponsoring_registrar = Fred::InfoRegistrarByHandle(
                nd.info.info_nsset_data.sponsoring_registrar_handle).exec(ctx, "UTC");

            nd.external_states = Fred::RecordStatement::make_historic_external_states(
                nd.info.info_nsset_data.id, timestamp, ctx);

            ret = nd;
        }

        return ret;
    }

    boost::optional<Fred::RecordStatement::KeysetPrintoutInputData> make_hystoric_keyset_data(
        boost::optional<unsigned long long> keyset_historyid,
        const std::string& timestamp, //for keyset tech contact and keyset states history
        Fred::OperationContext& ctx)
    {
        boost::optional<Fred::RecordStatement::KeysetPrintoutInputData> ret;

        if(keyset_historyid.is_initialized())
        {
            Fred::RecordStatement::KeysetPrintoutInputData kd;
            kd.info = Fred::InfoKeysetHistoryByHistoryid(
                    *keyset_historyid).exec(ctx, "UTC");

            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
                it = kd.info.info_keyset_data.tech_contacts.begin();
                it != kd.info.info_keyset_data.tech_contacts.end(); ++it)
            {
                kd.tech_contact.push_back(Fred::InfoContactHistoryByHistoryid(
                    get_history_id_internal(Fred::Object_Type::contact,
                    it->handle, timestamp, ctx)).exec(ctx, "UTC"));
            }

            kd.sponsoring_registrar = Fred::InfoRegistrarByHandle(
                kd.info.info_keyset_data.sponsoring_registrar_handle).exec(ctx, "UTC");

            kd.external_states = Fred::RecordStatement::make_historic_external_states(
                kd.info.info_keyset_data.id, timestamp, ctx);

            ret = kd;
        }

        return ret;
    }

    PdfBufferImpl RecordStatementImpl::historic_domain_printout(
        const std::string& fqdn,
        const std::string& timestamp,
        Fred::OperationContext* ex_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ex_ctx);

        Fred::InfoDomainOutput info_domain_output = Fred::InfoDomainHistoryByHistoryid(
            get_history_id(Fred::Object_Type::domain, fqdn, timestamp, ctx)).exec(ctx, "UTC");

        Fred::InfoContactOutput info_registrant_output = Fred::InfoContactHistoryByHistoryid(
            get_history_id_internal(Fred::Object_Type::contact,
                info_domain_output.info_domain_data.registrant.handle, timestamp, ctx))
        .exec(ctx, "UTC");

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
                info_domain_output.info_domain_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        std::vector<Fred::InfoContactOutput> info_admin_contact_output;

        for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
            i = info_domain_output.info_domain_data.admin_contacts.begin();
            i != info_domain_output.info_domain_data.admin_contacts.end(); ++i)
        {
            info_admin_contact_output.push_back(
                Fred::InfoContactHistoryByHistoryid(get_history_id_internal(Fred::Object_Type::contact,
                    i->handle, timestamp, ctx)).exec(ctx, "UTC"));
        }

        boost::optional<unsigned long long> nsset_historyid = info_domain_output.info_domain_data.nsset.isnull()
            ? boost::optional<unsigned long long>()
            : boost::optional<unsigned long long>(get_history_id_internal(Fred::Object_Type::nsset,
                info_domain_output.info_domain_data.nsset.get_value().handle, timestamp, ctx));

        boost::optional<Fred::RecordStatement::NssetPrintoutInputData> nsset_data
            = Registry::RecordStatement::make_hystoric_nsset_data(nsset_historyid, timestamp, ctx);

        boost::optional<unsigned long long> keyset_historyid = info_domain_output.info_domain_data.keyset.isnull()
            ? boost::optional<unsigned long long>()
            : boost::optional<unsigned long long>(get_history_id_internal(Fred::Object_Type::keyset,
                info_domain_output.info_domain_data.keyset.get_value().handle, timestamp, ctx));

        boost::optional<Fred::RecordStatement::KeysetPrintoutInputData> keyset_data
            = Registry::RecordStatement::make_hystoric_keyset_data(keyset_historyid, timestamp, ctx);


        const std::string xml_document = domain_printout_xml(
            info_domain_output,
            Fred::RecordStatement::convert_utc_timestamp_to_local(
                ctx, info_domain_output.utc_timestamp, registry_timezone_),
            Fred::RecordStatement::convert_utc_timestamp_to_local(
                ctx, info_domain_output.info_domain_data.creation_time, registry_timezone_),
            (info_domain_output.info_domain_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_domain_output.info_domain_data.update_time.get_value(), registry_timezone_))),
            true, //is_private_printout
            info_registrant_output,
            info_admin_contact_output,
            info_sponsoring_registrar_output,
            nsset_data,
            keyset_data,
            Fred::RecordStatement::make_historic_external_states(
                info_domain_output.info_domain_data.id, timestamp, ctx)
        );

        ctx.get_log().debug(xml_document);

        std::ostringstream pdf_document;
        boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                Fred::Document::GT_RECORD_STATEMENT_DOMAIN, pdf_document,"").release());

        doc_gen->getInput() << xml_document;
        doc_gen->closeInput();

        return PdfBufferImpl(pdf_document.str());
    }

    PdfBufferImpl RecordStatementImpl::historic_nsset_printout(
        const std::string& handle,
        const std::string& timestamp,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        Fred::RecordStatement::NssetPrintoutInputData nsset_data
            = *Registry::RecordStatement::make_hystoric_nsset_data(get_history_id(
                Fred::Object_Type::nsset, handle, timestamp, ctx), timestamp, ctx);

        const std::string xml_document = Fred::RecordStatement::nsset_printout_xml(nsset_data,
                Fred::RecordStatement::convert_utc_timestamp_to_local(
                    ctx, nsset_data.info.utc_timestamp, registry_timezone_));

        ctx.get_log().debug(xml_document);

        std::ostringstream pdf_document;
        boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                Fred::Document::GT_RECORD_STATEMENT_NSSET, pdf_document,"").release());

        doc_gen->getInput() << xml_document;
        doc_gen->closeInput();

        return PdfBufferImpl(pdf_document.str());
    }

    PdfBufferImpl RecordStatementImpl::historic_keyset_printout(
        const std::string& handle,
        const std::string& timestamp,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        Fred::RecordStatement::KeysetPrintoutInputData keyset_data
            = *Registry::RecordStatement::make_hystoric_keyset_data(get_history_id(
                Fred::Object_Type::keyset, handle, timestamp, ctx), timestamp, ctx);

        const std::string xml_document =  Fred::RecordStatement::keyset_printout_xml(keyset_data,
                Fred::RecordStatement::convert_utc_timestamp_to_local(
                    ctx, keyset_data.info.utc_timestamp, registry_timezone_));

        ctx.get_log().debug(xml_document);

        std::ostringstream pdf_document;
        boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                Fred::Document::GT_RECORD_STATEMENT_KEYSET, pdf_document,"").release());

        doc_gen->getInput() << xml_document;
        doc_gen->closeInput();

        return PdfBufferImpl(pdf_document.str());
    }

    PdfBufferImpl RecordStatementImpl::historic_contact_printout(
        const std::string& handle,
        const std::string& timestamp,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        Fred::InfoContactOutput info_contact_output = Fred::InfoContactHistoryByHistoryid(
                get_history_id(Fred::Object_Type::contact, handle, timestamp, ctx))
            .exec(ctx, "UTC");

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
            info_contact_output.info_contact_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        const std::string xml_document = Fred::RecordStatement::contact_printout_xml(true, info_contact_output,
                Fred::RecordStatement::convert_utc_timestamp_to_local(
                    ctx, info_contact_output.utc_timestamp, registry_timezone_),
                Fred::RecordStatement::convert_utc_timestamp_to_local(
                    ctx, info_contact_output.info_contact_data.creation_time, registry_timezone_),
            (info_contact_output.info_contact_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.update_time.get_value(), registry_timezone_))),
            (info_contact_output.info_contact_data.transfer_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.transfer_time.get_value(), registry_timezone_))),
            info_sponsoring_registrar_output,
                Fred::RecordStatement::make_historic_external_states(
                    info_contact_output.info_contact_data.id, timestamp, ctx)
        );

        ctx.get_log().debug(xml_document);

        std::ostringstream pdf_document;
        boost::shared_ptr< Fred::Document::Generator > doc_gen(
            doc_manager_->createOutputGenerator(
                Fred::Document::GT_RECORD_STATEMENT_CONTACT, pdf_document,"").release());

        doc_gen->getInput() << xml_document;
        doc_gen->closeInput();

        return PdfBufferImpl(pdf_document.str());
    }

    void RecordStatementImpl::send_domain_printout(
        const std::string& fqdn,
        bool is_private_printout,
        Fred::OperationContext* ex_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ex_ctx);

        std::string registrant_email_out;
        boost::posix_time::ptime request_local_timestamp;
        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::domain_printout_xml_with_data(
            fqdn,
            registry_timezone_,
            is_private_printout,
            ctx,
            &registrant_email_out,
            &request_local_timestamp);

        enum { RECORD_STATEMENT_FILETYPE = 11 };
        const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_DOMAIN,
            xml_document,
            "registry_record_statement.pdf",
            RECORD_STATEMENT_FILETYPE, "");

        Fred::Mailer::Parameters params;
        params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().day()));
        params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().month()));
        params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().year()));

        Fred::Mailer::Handles handles;
        Fred::Mailer::Attachments attach;

        if(file_id != 0)
        {
            attach.push_back(file_id);
        }
        else
        {
            throw std::runtime_error("missing file_id");
        }

        const unsigned long long mail_id = mailer_manager_->sendEmail(
            "", // default sender according to template
            registrant_email_out,
            "", // default subject according to template
            "record_statement",
            params, handles, attach
        );

        if(mail_id == 0)
        {
            throw std::runtime_error("sendEmail failed");
        }
    }

    void RecordStatementImpl::send_nsset_printout(
        const std::string& handle,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        std::vector<std::string> email_out;
        boost::posix_time::ptime request_local_timestamp;
        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::nsset_printout_xml_with_data(
            handle, registry_timezone_, ctx, &email_out, &request_local_timestamp);

        enum { RECORD_STATEMENT_FILETYPE = 11 };
        const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_NSSET,
            xml_document,
            "registry_record_statement.pdf",
            RECORD_STATEMENT_FILETYPE, "");


        Fred::Mailer::Parameters params;
        params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().day()));
        params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().month()));
        params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().year()));

        Fred::Mailer::Handles handles;
        Fred::Mailer::Attachments attach;

        if(file_id != 0)
        {
            attach.push_back(file_id);
        }
        else
        {
            throw std::runtime_error("missing file_id");
        }

        std::string emails = boost::join(email_out, "', '");

        if(mailer_manager_->checkEmailList(emails))
        {
            const unsigned long long mail_id = mailer_manager_->sendEmail(
                "", // default sender according to template
                emails,
                "", // default subject according to template
                "record_statement",
                params, handles, attach
            );

            if(mail_id == 0)
            {
                throw std::runtime_error("sendEmail failed");
            }
        }
    }

    void RecordStatementImpl::send_keyset_printout(
        const std::string& handle,
        Fred::OperationContext* ext_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ext_ctx);

        std::vector<std::string> email_out;
        boost::posix_time::ptime request_local_timestamp;
        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::keyset_printout_xml_with_data(
            handle, registry_timezone_, ctx, &email_out, &request_local_timestamp);

        enum { RECORD_STATEMENT_FILETYPE = 11 };
        const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_KEYSET,
            xml_document,
            "registry_record_statement.pdf",
            RECORD_STATEMENT_FILETYPE, "");

        Fred::Mailer::Parameters params;
        params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().day()));
        params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().month()));
        params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().year()));

        Fred::Mailer::Handles handles;
        Fred::Mailer::Attachments attach;

        if(file_id != 0)
        {
            attach.push_back(file_id);
        }
        else
        {
            throw std::runtime_error("missing file_id");
        }

        std::string emails = boost::join(email_out, "', '");

        if(mailer_manager_->checkEmailList(emails))
        {
            const unsigned long long mail_id = mailer_manager_->sendEmail(
                "", // default sender according to template
                emails,
                "", // default subject according to template
                "record_statement",
                params, handles, attach
            );

            if(mail_id == 0)
            {
                throw std::runtime_error("sendEmail failed");
            }
        }
    }

    void RecordStatementImpl::send_contact_printout(
                const std::string& handle,
                bool is_private_printout,
                Fred::OperationContext* ex_ctx) const
    {
        Fred::OperationContextCreator in_ctx;
        Fred::OperationContext& ctx = Fred::select_operation_context(in_ctx, ex_ctx);

        std::string email_out;
        boost::posix_time::ptime request_local_timestamp;
        std::stringstream xml_document;
        xml_document << Fred::RecordStatement::contact_printout_xml_with_data(
            handle,
            registry_timezone_,
            is_private_printout,
            ctx,
            &email_out,
            &request_local_timestamp);

        enum { RECORD_STATEMENT_FILETYPE = 11 };
        const unsigned long long file_id = doc_manager_->generateDocumentAndSave(
            Fred::Document::GT_RECORD_STATEMENT_CONTACT,
            xml_document,
            "registry_record_statement.pdf",
            RECORD_STATEMENT_FILETYPE, "");

        Fred::Mailer::Parameters params;
        params["request_day"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().day()));
        params["request_month"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().month()));
        params["request_year"] = boost::lexical_cast<std::string>(
            static_cast<int>(request_local_timestamp.date().year()));

        Fred::Mailer::Handles handles;
        Fred::Mailer::Attachments attach;

        if(file_id != 0)
        {
            attach.push_back(file_id);
        }
        else
        {
            throw std::runtime_error("missing file_id");
        }

        const unsigned long long mail_id = mailer_manager_->sendEmail(
            "", // default sender according to template
            email_out,
            "", // default subject according to template
            "record_statement",
            params, handles, attach
        );

        if(mail_id == 0)
        {
            throw std::runtime_error("sendEmail failed");
        }
    }


}//namespace RecordStatement
}//namespace Registry

