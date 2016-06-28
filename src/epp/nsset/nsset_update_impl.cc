#include "src/epp/nsset/nsset_update_impl.h"

#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"

#include "util/optional_value.h"
#include "util/map_at.h"
#include "util/util.h"

#include <vector>
#include <map>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>


namespace Epp {

unsigned long long nsset_update_impl(
    Fred::OperationContext& _ctx,
    const NssetUpdateInputData& _data,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Nsset::get_handle_registrability(_ctx, _data.handle) != Fred::NssetHandleState::Registrability::registered ) {
        throw NonexistentHandle();
    }

    struct translate_info_nsset_exception {
        static Fred::InfoNssetData exec(Fred::OperationContext& _ctx, const std::string _handle) {
            try {
                return Fred::InfoNssetByHandle(_handle).set_lock().exec(_ctx).info_nsset_data;
            } catch(const Fred::InfoNssetByHandle::Exception& e) {
                if( e.is_set_unknown_handle() ) {
                    throw NonexistentHandle();
                }
                throw;
            }
        }
    };

    const Fred::InfoNssetData nsset_data_before_update = translate_info_nsset_exception::exec(_ctx, _data.handle);

    const Fred::InfoRegistrarData sponsoring_registrar_before_update =
        Fred::InfoRegistrarByHandle(nsset_data_before_update.sponsoring_registrar_handle)
            .set_lock(/* TODO lock registrar for share */ )
            .exec(_ctx)
            .info_registrar_data;

    const Fred::InfoRegistrarData logged_in_registrar = Fred::InfoRegistrarById(_registrar_id)
            .set_lock(/* TODO lock registrar for share */ )
            .exec(_ctx)
            .info_registrar_data;

    if( sponsoring_registrar_before_update.id != _registrar_id
        && !logged_in_registrar.system.get_value_or_default() ) {
        throw AutorError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data_before_update.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data_before_update.id).exec(_ctx);

    if( !logged_in_registrar.system.get_value_or_default()
            && (Fred::ObjectHasState(nsset_data_before_update.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(_ctx)
                ||
                Fred::ObjectHasState(nsset_data_before_update.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx))
    ) {
        throw ObjectStatusProhibitingOperation();
    }

    //check technical contacts to add and remove
    //check dns hosts to add and remove
    {
        ParameterValuePolicyError ex;
        std::map<std::string, std::size_t> tech_contact_to_add_duplicity_map;
        for(std::size_t i = 0; i < _data.tech_contacts_add.size(); ++i)
        {   //check technical contact exists
            if(Fred::Contact::get_handle_registrability(_ctx, _data.tech_contacts_add.at(i))
                != Fred::ContactHandleState::Registrability::registered)
            {
                ex.add(Error(Param::nsset_tech_add,
                    boost::numeric_cast<unsigned short>(i+1),//position in list
                    Reason::tech_notexist));
            }
            else
            {//check technical contact duplicity
                const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                    _data.tech_contacts_add.at(i));
                Optional<std::size_t> duplicity = optional_map_at<Optional>(
                    tech_contact_to_add_duplicity_map, upper_tech_contact_handle);

                if(duplicity.isset())
                {
                    ex.add(Error(Param::nsset_tech_add,
                        boost::numeric_cast<unsigned short>(i+1),//position in list
                        Reason::duplicity_contact));
                }
                else
                {
                    tech_contact_to_add_duplicity_map[upper_tech_contact_handle] = i;
                }
            }
        }

        std::map<std::string, std::size_t> tech_contact_to_remove_duplicity_map;


        std::set<std::string> nsset_tech_c_handles;
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_c_element, nsset_data_before_update.tech_contacts)
        {
            nsset_tech_c_handles.insert(tech_c_element.handle);
        }

        for(std::size_t i = 0; i < _data.tech_contacts_rem.size(); ++i)
        {
            //check if given tech contact to remove is NOT admin of nsset
            if(nsset_tech_c_handles.find(_data.tech_contacts_rem.at(i)) == nsset_tech_c_handles.end())
            {
                ex.add(Error(Param::nsset_tech_rem,
                    boost::numeric_cast<unsigned short>(i+1),//position in list
                    Reason::can_not_remove_tech));
            }
            else
            {//check technical contact duplicity
                const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                    _data.tech_contacts_rem.at(i));
                Optional<std::size_t> duplicity = optional_map_at<Optional>(
                    tech_contact_to_remove_duplicity_map, upper_tech_contact_handle);

                if(duplicity.isset())
                {
                    ex.add(Error(Param::nsset_tech_rem,
                        boost::numeric_cast<unsigned short>(i+1),//position in list
                        Reason::duplicity_contact));
                }
                else
                {
                    tech_contact_to_remove_duplicity_map[upper_tech_contact_handle] = i;
                }
            }
        }

        if(!ex.is_empty()) throw ex;
    }


    // update itself
    {
        std::vector<Fred::DnsHost> dns_hosts_add;
        dns_hosts_add.reserve(_data.dns_hosts_add.size());
        BOOST_FOREACH(const Epp::DNShostData& host, _data.dns_hosts_add)
        {
            dns_hosts_add.push_back(Fred::DnsHost(host.fqdn, host.inet_addr));
        }

        std::vector<std::string> dns_hosts_rem;
        dns_hosts_rem.reserve(_data.dns_hosts_rem.size());
        BOOST_FOREACH(const Epp::DNShostData& host, _data.dns_hosts_rem)
        {
            dns_hosts_rem.push_back(host.fqdn);
        }

        Fred::UpdateNsset update(_data.handle,
            sponsoring_registrar_before_update.handle,
            Optional<std::string>(),
            _data.authinfo,
            dns_hosts_add,
            dns_hosts_rem,
            _data.tech_contacts_add,
            _data.tech_contacts_rem,
            _data.tech_check_level,
            _logd_request_id
        );

        try {
            const unsigned long long new_history_id = update.exec(_ctx);

            return new_history_id;

        } catch(const Fred::UpdateNsset::Exception& e) {

            /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
            if(
                e.is_set_unknown_registrar_handle()
                || e.is_set_unknown_sponsoring_registrar_handle()
            ) {
                throw;
            }

            if( e.is_set_unknown_nsset_handle() ) {
                throw NonexistentHandle();
            }

            /* in the improbable case that exception is incorrectly set */
            throw;
        }
    }
}

}
