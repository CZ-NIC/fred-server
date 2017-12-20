#include "src/backend/admin/contact/merge_contact_logger.hh"


void logger_merge_contact_transform_output_data(
        const LibFred::MergeContactOutput &_merge_data,
        LibFred::Logger::RequestProperties &_properties,
        LibFred::Logger::ObjectReferences &_references)
{
    for (std::vector<LibFred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        _properties.push_back(LibFred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(LibFred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(LibFred::Logger::RequestProperty("registrant", i->set_registrant, true));
        _references.push_back(LibFred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<LibFred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        _properties.push_back(LibFred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(LibFred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(LibFred::Logger::RequestProperty("remAdmin", i->rem_admin_contact, true));
        _properties.push_back(LibFred::Logger::RequestProperty("addAdmin", i->add_admin_contact, true));
        _references.push_back(LibFred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<LibFred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        _properties.push_back(LibFred::Logger::RequestProperty("command", "update_nsset", false));
        _properties.push_back(LibFred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(LibFred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(LibFred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(LibFred::Logger::ObjectReference("nsset", i->nsset_id));
    }
    for (std::vector<LibFred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        _properties.push_back(LibFred::Logger::RequestProperty("command", "update_keyset", false));
        _properties.push_back(LibFred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(LibFred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(LibFred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(LibFred::Logger::ObjectReference("keyset", i->keyset_id));
    }
}


unsigned long long logger_merge_contact_create_request(
        LibFred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact)
{
    unsigned long long req_id = _logger_client.createRequest("", "Admin", "",
            boost::assign::list_of
                (LibFred::Logger::RequestProperty("src_contact", _src_contact, false))
                (LibFred::Logger::RequestProperty("dst_contact", _dst_contact, false)),
            LibFred::Logger::ObjectReferences(),
            "ContactMerge", 0);
    if (req_id == 0) {
        throw std::runtime_error("unable to log merge contact request");
    }
    return req_id;
}


void logger_merge_contact_close(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        LibFred::Logger::RequestProperties &_properties,
        LibFred::Logger::ObjectReferences &_references,
        const std::string &_result)
{
    if (_req_id) {
        _properties.push_back(LibFred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
        _logger_client.closeRequest(_req_id, "Admin", "",
                _properties,
                _references,
                _result, 0);
    }
}


void logger_merge_contact_close_request_success(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const LibFred::MergeContactOutput &_merge_data)
{
    LibFred::Logger::RequestProperties props;
    LibFred::Logger::ObjectReferences refs;
    logger_merge_contact_transform_output_data(_merge_data, props, refs),
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Success");
}


void logger_merge_contact_close_request_fail(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id)
{
    LibFred::Logger::RequestProperties props;
    LibFred::Logger::ObjectReferences refs;
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Fail");
}

