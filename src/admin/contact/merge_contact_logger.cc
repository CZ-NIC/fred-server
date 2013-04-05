#include "merge_contact_logger.h"


void logger_merge_contact_transform_output_data(
        const Fred::MergeContactOutput &_merge_data,
        Fred::Logger::RequestProperties &_properties,
        Fred::Logger::ObjectReferences &_references)
{
    for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("registrant", i->set_registrant, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("remAdmin", i->rem_admin_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addAdmin", i->add_admin_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_nsset", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(Fred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("nsset", i->nsset_id));
    }
    for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_keyset", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(Fred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("keyset", i->keyset_id));
    }
}


unsigned long long logger_merge_contact_create_request(
        Fred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact)
{
    unsigned long long req_id = _logger_client.createRequest("", "Admin", "",
            boost::assign::list_of
                (Fred::Logger::RequestProperty("src_contact", _src_contact, false))
                (Fred::Logger::RequestProperty("dst_contact", _dst_contact, false)),
            Fred::Logger::ObjectReferences(),
            "ContactMerge", 0);
    if (req_id == 0) {
        throw std::runtime_error("unable to log merge contact request");
    }
    return req_id;
}


void logger_merge_contact_close(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        Fred::Logger::RequestProperties &_properties,
        Fred::Logger::ObjectReferences &_references,
        const std::string &_result)
{
    if (_req_id) {
        _properties.push_back(Fred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
        _logger_client.closeRequest(_req_id, "Admin", "",
                _properties,
                _references,
                _result, 0);
    }
}


void logger_merge_contact_close_request_success(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const Fred::MergeContactOutput &_merge_data)
{
    Fred::Logger::RequestProperties props;
    Fred::Logger::ObjectReferences refs;
    logger_merge_contact_transform_output_data(_merge_data, props, refs),
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Success");
}


void logger_merge_contact_close_request_fail(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id)
{
    Fred::Logger::RequestProperties props;
    Fred::Logger::ObjectReferences refs;
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Fail");
}

