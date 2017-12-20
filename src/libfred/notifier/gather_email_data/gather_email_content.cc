#include "src/libfred/notifier/gather_email_data/gather_email_content.hh"

#include "src/libfred/notifier/gather_email_data/objecttype_specific_impl/contact.hh"
#include "src/libfred/notifier/gather_email_data/objecttype_specific_impl/domain.hh"
#include "src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.hh"
#include "src/libfred/notifier/gather_email_data/objecttype_specific_impl/nsset.hh"

#include "src/libfred/notifier/exception.hh"

#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"

#include "src/libfred/registrar/info_registrar.hh"

namespace Notification {

/** @throws ExceptionUnknownObjectType */
static std::string to_email_template_id(LibFred::object_type _input) {
    switch(_input) {
        case LibFred::contact:   return "1";
        case LibFred::domain:    return "3";
        case LibFred::keyset:    return "4";
        case LibFred::nsset:     return "2";
    };

    throw LibFred::ExceptionUnknownObjectType();
}



std::map<std::string, std::string> gather_common_email_content(
    LibFred::OperationContext& _ctx,
    const notification_request& _request
) {
    std::map<std::string, std::string> data;

    data["ticket"] = _request.svtrid;

    struct registrar_data_format{
        static std::string get(const LibFred::InfoRegistrarData& _reg) {
            return _reg.name.get_value_or("") + " (" + _reg.url.get_value_or("") +")";
        }
    };

    data["registrar"] = registrar_data_format::get( LibFred::InfoRegistrarById(_request.done_by_registrar).exec(_ctx).info_registrar_data );

    switch(_request.event.get_type()) {
        case LibFred::contact :
            data["handle"] = LibFred::InfoContactHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_contact_data.handle;
            break;
        case LibFred::domain :
            data["handle"] = LibFred::InfoDomainHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_domain_data.fqdn;
            break;
        case LibFred::keyset :
            data["handle"] = LibFred::InfoKeysetHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_keyset_data.handle;
            break;
        case LibFred::nsset :
            data["handle"] = LibFred::InfoNssetHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_nsset_data.handle;
            break;
        default:
            throw LibFred::ExceptionUnknownObjectType();
    }

    data["type"] = to_email_template_id( _request.event.get_type() );

    return data;
}



std::map<std::string, std::string> gather_email_content(
    LibFred::OperationContext& _ctx,
    const notification_request& _request
) {

    std::map<std::string, std::string> data = gather_common_email_content(_ctx, _request);

    std::map<std::string, std::string> object_type_specific_data;

    if( _request.event.get_type()        == LibFred::contact   ) {
        object_type_specific_data = gather_contact_data_change( _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == LibFred::domain    ) {
        object_type_specific_data = gather_domain_data_change(  _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == LibFred::keyset    ) {
        object_type_specific_data = gather_keyset_data_change(  _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == LibFred::nsset     ) {
        object_type_specific_data = gather_nsset_data_change(   _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else {
        throw ExceptionObjectTypeNotImplemented();
    }

    if( !object_type_specific_data.empty() ) {

        for(std::map<std::string, std::string>::const_iterator it = object_type_specific_data.begin();
            it != object_type_specific_data.end();
            ++it
        ) {
            if(data.find(it->first) != data.end()) {
                /* value for given key is already present in result data */
                throw ExceptionDataLoss();
            }
        }

        data.insert(object_type_specific_data.begin(), object_type_specific_data.end());
    }

    return data;
}

}
