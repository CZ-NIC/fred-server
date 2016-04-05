#include "src/fredlib/notifier/gather_email_data/gather_email_content.h"

#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/contact.h"
#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/domain.h"
#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/keyset.h"
#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/nsset.h"

#include "src/fredlib/notifier/exception.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"

#include "src/fredlib/registrar/info_registrar.h"

namespace Notification {

/** @throws ExceptionUnknownObjectType */
static std::string to_email_template_id(Fred::object_type _input) {
    switch(_input) {
        case Fred::contact:   return "1";
        case Fred::domain:    return "3";
        case Fred::keyset:    return "4";
        case Fred::nsset:     return "2";
    };

    throw Fred::ExceptionUnknownObjectType();
}



std::map<std::string, std::string> gather_common_email_content(
    Fred::OperationContext& _ctx,
    const notification_request& _request
) {
    std::map<std::string, std::string> data;

    data["ticket"] = _request.svtrid;

    struct registrar_data_format{
        static std::string get(const Fred::InfoRegistrarData& _reg) {
            return _reg.name.get_value_or("") + " (" + _reg.url.get_value_or("") +")";
        }
    };

    data["registrar"] = registrar_data_format::get( Fred::InfoRegistrarById(_request.done_by_registrar).exec(_ctx).info_registrar_data );

    switch(_request.event.get_type()) {
        case Fred::contact :
            data["handle"] = Fred::InfoContactHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_contact_data.handle;
            break;
        case Fred::domain :
            data["handle"] = Fred::InfoDomainHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_domain_data.fqdn;
            break;
        case Fred::keyset :
            data["handle"] = Fred::InfoKeysetHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_keyset_data.handle;
            break;
        case Fred::nsset :
            data["handle"] = Fred::InfoNssetHistoryByHistoryid(_request.history_id_post_change).exec(_ctx).info_nsset_data.handle;
            break;
        default:
            throw Fred::ExceptionUnknownObjectType();
    }

    data["type"] = to_email_template_id( _request.event.get_type() );

    return data;
}



std::map<std::string, std::string> gather_email_content(
    Fred::OperationContext& _ctx,
    const notification_request& _request
) {

    std::map<std::string, std::string> data = gather_common_email_content(_ctx, _request);

    std::map<std::string, std::string> object_type_specific_data;

    if( _request.event.get_type()        == Fred::contact   ) {
        object_type_specific_data = gather_contact_data_change( _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == Fred::domain    ) {
        object_type_specific_data = gather_domain_data_change(  _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == Fred::keyset    ) {
        object_type_specific_data = gather_keyset_data_change(  _ctx, _request.event.get_event(), _request.history_id_post_change );

    } else if( _request.event.get_type() == Fred::nsset     ) {
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
