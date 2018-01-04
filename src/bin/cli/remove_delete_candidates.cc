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

#include "src/bin/cli/remove_delete_candidates.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/object/object_state.hh"
#include "src/libfred/registrable_object/contact/delete_contact.hh"
#include "src/libfred/registrable_object/domain/delete_domain.hh"
#include "src/libfred/registrable_object/keyset/delete_keyset.hh"
#include "src/libfred/registrable_object/nsset/delete_nsset.hh"

#include "src/util/enum_conversion.hh"
#include "src/util/random.hh"
#include "src/util/log/context.hh"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/format.hpp>

namespace Admin {

namespace {

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const std::string &_op_name)
        : ctx_operation_(_op_name)
    { }
private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))

} // namespace Admin::{anonymous}

template <>
DbHandle<ObjectType> to_db_handle(ObjectType enum_value)
{
    switch (enum_value)
    {
        case ObjectType::contact:
            return DbHandle<ObjectType>::construct_from(std::string("contact"));
        case ObjectType::domain:
            return DbHandle<ObjectType>::construct_from(std::string("domain"));
        case ObjectType::keyset:
            return DbHandle<ObjectType>::construct_from(std::string("keyset"));
        case ObjectType::nsset:
            return DbHandle<ObjectType>::construct_from(std::string("nsset"));
    }
    throw std::runtime_error("unable convert to db handle");
}

template <>
ObjectType from_db_handle(const DbHandle<ObjectType>& handle)
{
    static const ObjectType set_of_values[] =
            {
                ObjectType::contact,
                ObjectType::domain,
                ObjectType::keyset,
                ObjectType::nsset
            };
    return Conversion::Enums::inverse_transformation(handle, set_of_values, to_db_handle<ObjectType>);
}

ObjectType object_type_from_string(const std::string& src)
{
    try
    {
        return from_db_handle(DbHandle<ObjectType>::construct_from(src));
    }
    catch (const std::invalid_argument&)
    {
        if (src == "1")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::contact;
        }
        if (src == "2")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::nsset;
        }
        if (src == "3")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::domain;
        }
        if (src == "4")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::keyset;
        }
    }
    throw std::runtime_error("unable convert to object type");
}

boost::optional<SetOfObjectTypes> construct_set_of_object_types_from_string(const std::string& src)
{
    if (src.empty())
    {
        return boost::none;
    }

    SetOfObjectTypes set_of_object_types;

    static const std::regex delimiter(",");
    for (auto object_types_itr = std::sregex_token_iterator(src.begin(), src.end(), delimiter, -1);
         object_types_itr != std::sregex_token_iterator(); ++object_types_itr)
    {
        const ObjectType object_type = object_type_from_string(*object_types_itr);
        set_of_object_types.insert(object_type);
    }
    return set_of_object_types;
}

namespace {

std::string object_types_into_sql(
        const SetOfObjectTypes& types,
        Database::query_param_list& params)
{
    std::string sql;
    for (const auto& object_type : types)
    {
        if (!sql.empty())
        {
            sql += ",";
        }
        sql += "$" + params.add(to_db_handle(object_type).get_value()) + "::TEXT";
    }
    return sql;
}

struct Domain_;
typedef DbId<Domain_> DomainId;
typedef DbHandle<Domain_> DomainFqdn;

struct Contact_;
typedef DbId<Contact_> ContactId;
typedef DbHandle<Contact_> ContactHandle;

struct Keyset_;
typedef DbId<Keyset_> KeysetId;
typedef DbHandle<Keyset_> KeysetHandle;

struct Nsset_;
typedef DbId<Nsset_> NssetId;
typedef DbHandle<Nsset_> NssetHandle;

struct Zone_;
typedef DbId<Zone_> ZoneId;

struct DomainToDelete
{
    DomainToDelete(
            DomainId _domain_id,
            const DomainFqdn& _domain_fqdn,
            ZoneId _zone_id)
        : domain_id(_domain_id),
          domain_fqdn(_domain_fqdn),
          zone_id(_zone_id)
    { }
    DomainId domain_id;
    DomainFqdn domain_fqdn;
    ZoneId zone_id;
};

struct ContactToDelete
{
    ContactToDelete(
            ContactId _contact_id,
            const ContactHandle& _contact_handle)
        : contact_id(_contact_id),
          contact_handle(_contact_handle)
    { }
    ContactId contact_id;
    ContactHandle contact_handle;
};

struct KeysetToDelete
{
    KeysetToDelete(
            KeysetId _keyset_id,
            const KeysetHandle& _keyset_handle)
        : keyset_id(_keyset_id),
          keyset_handle(_keyset_handle)
    { }
    KeysetId keyset_id;
    KeysetHandle keyset_handle;
};

struct NssetToDelete
{
    NssetToDelete(
            NssetId _nsset_id,
            const NssetHandle& _nsset_handle)
        : nsset_id(_nsset_id),
          nsset_handle(_nsset_handle)
    { }
    NssetId nsset_id;
    NssetHandle nsset_handle;
};

unsigned init_objects_to_delete(
        const boost::optional<unsigned>& max_number_of_selected_candidates,
        const boost::optional<SetOfObjectTypes>& types,
        std::vector<ContactToDelete>& contacts,
        std::vector<DomainToDelete>& domains,
        std::vector<KeysetToDelete>& keysets,
        std::vector<NssetToDelete>& nssets)
{
    Database::query_param_list params(Conversion::Enums::to_db_handle(LibFred::Object_State::delete_candidate));//$1
    params(to_db_handle(ObjectType::domain).get_value());//$2
    const std::string sql =
        "SELECT eot.name,o.id,o.name,"
               "CASE WHEN eot.name=$2::TEXT THEN (SELECT zone FROM domain WHERE id=o.id) ELSE NULL END "
        "FROM object_registry o "
        "JOIN enum_object_type eot ON eot.id=o.type "
        "JOIN object_state s ON s.object_id=o.id "
        "JOIN enum_object_states eos ON eos.id=s.state_id "
        "WHERE o.erdate IS NULL AND " + (types != boost::none
            ? "eot.name IN (" + object_types_into_sql(*types, params) + ") AND "
            : std::string()) +
              "s.valid_to IS NULL AND "
              "eos.name=$1::TEXT";
    LibFred::OperationContextCreator ctx;
    const Database::Result dbres = ctx.get_conn().exec_params(sql, params);
    contacts.clear();
    domains.clear();
    keysets.clear();
    nssets.clear();
    if (dbres.size() == 0)
    {
        return 0;
    }
    if ((types == boost::none) || (types->find(ObjectType::contact) != types->end()))
    {
        contacts.reserve(dbres.size());
    }
    if ((types == boost::none) || (types->find(ObjectType::domain) != types->end()))
    {
        domains.reserve(dbres.size());
    }
    if ((types == boost::none) || (types->find(ObjectType::keyset) != types->end()))
    {
        keysets.reserve(dbres.size());
    }
    if ((types == boost::none) || (types->find(ObjectType::nsset) != types->end()))
    {
        nssets.reserve(dbres.size());
    }
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        const ObjectType object_type = from_db_handle(
                DbHandle<ObjectType>::construct_from(static_cast<std::string>(dbres[idx][0])));
        switch (object_type)
        {
            case ObjectType::contact:
                contacts.push_back(ContactToDelete(
                        ContactId::construct_from(static_cast<ContactId::BaseType>(dbres[idx][1])),
                        ContactHandle::construct_from(static_cast<ContactHandle::BaseType>(dbres[idx][2]))));
                break;
            case ObjectType::domain:
                domains.push_back(DomainToDelete(
                        DomainId::construct_from(static_cast<DomainId::BaseType>(dbres[idx][1])),
                        DomainFqdn::construct_from(static_cast<DomainFqdn::BaseType>(dbres[idx][2])),
                        ZoneId::construct_from(static_cast<ZoneId::BaseType>(dbres[idx][3]))));
                break;
            case ObjectType::keyset:
                keysets.push_back(KeysetToDelete(
                        KeysetId::construct_from(static_cast<KeysetId::BaseType>(dbres[idx][1])),
                        KeysetHandle::construct_from(static_cast<KeysetHandle::BaseType>(dbres[idx][2]))));
                break;
            case ObjectType::nsset:
                nssets.push_back(NssetToDelete(
                        NssetId::construct_from(static_cast<NssetId::BaseType>(dbres[idx][1])),
                        NssetHandle::construct_from(static_cast<NssetHandle::BaseType>(dbres[idx][2]))));
                break;
        }
    }
    return (max_number_of_selected_candidates == boost::none) ||
           (dbres.size() <= *max_number_of_selected_candidates)
               ? dbres.size()
               : *max_number_of_selected_candidates;
}

} // namespace Admin::{anonymous}

template <Debug d>
void delete_objects_marked_as_delete_candidate(
        int fraction,
        const boost::optional<unsigned>& max_number_of_selected_candidates,
        const boost::optional<SetOfObjectTypes>& types,
        const Seconds& spread_deletion_in_time)
{
    LOGGING_CONTEXT(log_ctx);

    std::vector<ContactToDelete> contacts;
    std::vector<DomainToDelete> domains;
    std::vector<KeysetToDelete> keysets;
    std::vector<NssetToDelete> nssets;
    const auto number_of_objects = init_objects_to_delete(
            max_number_of_selected_candidates,
            types,
            contacts,
            domains,
            keysets,
            nssets);
    if (d == Debug::on)
    {
        std::cout << "<objects>\n";
        for (const auto& domain : domains)
        {
            std::cout << "<object name='" << domain.domain_fqdn.get_value() << "'/>\n";
        }
        for (const auto& nsset : nssets)
        {
            std::cout << "<object name='" << nsset.nsset_handle.get_value() << "'/>\n";
        }
        for (const auto& keyset : keysets)
        {
            std::cout << "<object name='" << keyset.keyset_handle.get_value() << "'/>\n";
        }
        for (const auto& contact : contacts)
        {
            std::cout << "<object name='" << contact.contact_handle.get_value() << "'/>\n";
        }
        std::cout << "</objects>" << std::endl;
        return;
    }
    if (number_of_objects == 0)
    {
        return;
    }

    const auto divide_by = fraction < 1 ? 1 : fraction;
    const auto number_of_objects_to_delete_now = static_cast<double>(number_of_objects) / divide_by;
    const auto relative_duration_of_one_deletion = 1 / number_of_objects_to_delete_now;
    const bool number_of_objects_to_delete_now_is_whole_number = (number_of_objects % divide_by) == 0;
    const auto max_number_of_deletions = number_of_objects_to_delete_now_is_whole_number
            ? static_cast<unsigned>(number_of_objects_to_delete_now)
            : static_cast<unsigned>(number_of_objects_to_delete_now) + 1;

    std::random_device source_of_randomness;
    typedef std::mt19937 RandomNumberGenerator;
    RandomNumberGenerator rnd_gen(source_of_randomness());
    std::shuffle(domains.begin(), domains.end(), rnd_gen);
    const bool delete_more_than_domains = domains.size() < max_number_of_deletions;
    if (delete_more_than_domains)
    {
        std::shuffle(nssets.begin(), nssets.end(), rnd_gen);
        const bool delete_more_than_nssets = (domains.size() + nssets.size()) < max_number_of_deletions;
        if (delete_more_than_nssets)
        {
            std::shuffle(keysets.begin(), keysets.end(), rnd_gen);
            const bool delete_more_than_keysets = (domains.size() + nssets.size() + keysets.size()) < max_number_of_deletions;
            if (delete_more_than_keysets)
            {
                std::shuffle(contacts.begin(), contacts.end(), rnd_gen);
            }
        }
    }

    std::vector<double> relative_times_of_deletions;
    relative_times_of_deletions.reserve(max_number_of_deletions);
    for (unsigned idx = 0; idx < max_number_of_deletions; ++idx)
    {
        static const auto number_of_possibilities = (1.0 + rnd_gen.max()) - rnd_gen.min();
        const auto current_possibility = rnd_gen() - rnd_gen.min();
        const auto normalized_random_number = current_possibility / number_of_possibilities;
        const auto relative_time_of_deletion = (idx + normalized_random_number) * relative_duration_of_one_deletion;
        if (relative_time_of_deletion <= 1.0)
        {
            relative_times_of_deletions.push_back(relative_time_of_deletion);
        }
    }
    typedef std::chrono::high_resolution_clock Clock;
    const auto start_time = Clock::now();
    unsigned idx_of_deleted_object = 0;
    for (const auto relative_time_of_deletion : relative_times_of_deletions)
    {
        const Seconds time_offset(relative_time_of_deletion * spread_deletion_in_time.count());
        const auto absolute_time_of_deletion = start_time + std::chrono::duration_cast<Clock::duration>(time_offset);
        try
        {
            LibFred::OperationContextCreator ctx;
            std::this_thread::sleep_until(absolute_time_of_deletion);
            if (idx_of_deleted_object < domains.size())
            {
                const unsigned idx_of_deleted_domain = idx_of_deleted_object;
                LibFred::DeleteDomainById(domains[idx_of_deleted_domain].domain_id.get_value()).exec(ctx);
                std::cerr << "domain " << domains[idx_of_deleted_domain].domain_fqdn.get_value() << " "
                             "(domain_id:" << domains[idx_of_deleted_domain].domain_id.get_value() << ", "
                             "zone_id:" << domains[idx_of_deleted_domain].zone_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else if (idx_of_deleted_object < (domains.size() + nssets.size()))
            {
                const unsigned idx_of_deleted_nsset = idx_of_deleted_object - domains.size();
                LibFred::DeleteNssetById(nssets[idx_of_deleted_nsset].nsset_id.get_value()).exec(ctx);
                std::cerr << "nsset " << nssets[idx_of_deleted_nsset].nsset_handle.get_value() << " "
                             "(nsset_id:" << nssets[idx_of_deleted_nsset].nsset_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else if (idx_of_deleted_object < (domains.size() + nssets.size() + keysets.size()))
            {
                const unsigned idx_of_deleted_keyset = idx_of_deleted_object - (domains.size() + nssets.size());
                LibFred::DeleteKeysetById(keysets[idx_of_deleted_keyset].keyset_id.get_value()).exec(ctx);
                std::cerr << "keyset " << keysets[idx_of_deleted_keyset].keyset_handle.get_value() << " "
                             "(keyset_id:" << keysets[idx_of_deleted_keyset].keyset_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else
            {
                const unsigned idx_of_deleted_contact = idx_of_deleted_object - (domains.size() + nssets.size() + keysets.size());
                LibFred::DeleteContactById(contacts[idx_of_deleted_contact].contact_id.get_value()).exec(ctx);
                std::cerr << "contact " << contacts[idx_of_deleted_contact].contact_handle.get_value() << " "
                             "(contact_id:" << contacts[idx_of_deleted_contact].contact_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            ctx.commit_transaction();
        }
        catch (const LibFred::DeleteContactById::Exception& e)
        {
            LOGGER(PACKAGE).error(boost::format("delete contact failed (%1%)") % e.what());
            const unsigned idx_of_deleted_contact = idx_of_deleted_object - (domains.size() + nssets.size() + keysets.size());
            std::cerr << "delete contact " << contacts[idx_of_deleted_contact].contact_handle.get_value() << " "
                         "(contact_id:" << contacts[idx_of_deleted_contact].contact_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteDomainById::Exception& e)
        {
            LOGGER(PACKAGE).error(boost::format("delete domain failed (%1%)") % e.what());
            const unsigned idx_of_deleted_domain = idx_of_deleted_object;
            std::cerr << "delete domain " << domains[idx_of_deleted_domain].domain_fqdn.get_value() << " "
                         "(domain_id:" << domains[idx_of_deleted_domain].domain_id.get_value() << ", "
                         "zone_id:" << domains[idx_of_deleted_domain].zone_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteKeysetById::Exception& e)
        {
            LOGGER(PACKAGE).error(boost::format("delete keyset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_keyset = idx_of_deleted_object - (domains.size() + nssets.size());
            std::cerr << "delete keyset " << keysets[idx_of_deleted_keyset].keyset_handle.get_value() << " "
                         "(keyset_id:" << keysets[idx_of_deleted_keyset].keyset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteNssetById::Exception& e)
        {
            LOGGER(PACKAGE).error(boost::format("delete nsset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_nsset = idx_of_deleted_object - domains.size();
            std::cerr << "delete nsset " << nssets[idx_of_deleted_nsset].nsset_handle.get_value() << " "
                         "(nsset_id:" << nssets[idx_of_deleted_nsset].nsset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            LOGGER(PACKAGE).error(boost::format("operation failed (%1%)") % e.what());
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("unexpected exception");
        }
        ++idx_of_deleted_object;
    }
}

template void delete_objects_marked_as_delete_candidate<Debug::on>(
        int, const boost::optional<unsigned>&, const boost::optional<SetOfObjectTypes>&, const Seconds&);
template void delete_objects_marked_as_delete_candidate<Debug::off>(
        int, const boost::optional<unsigned>&, const boost::optional<SetOfObjectTypes>&, const Seconds&);

} // namespace Admin
