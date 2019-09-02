/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/cli/remove_delete_candidates.hh"
#include "libfred/opcontext.hh"
#include "libfred/db_settings.hh"
#include "libfred/object/object_state.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"

#include "util/enum_conversion.hh"
#include "util/log/context.hh"
#include "util/random/algorithm/floating_point.hh"
#include "util/random/random.hh"

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
#include <boost/lexical_cast.hpp>

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

ObjectType object_type_from_cli_option(const std::string& cli_option)
{
    try
    {
        return from_db_handle(DbHandle<ObjectType>::construct_from(cli_option));
    }
    catch (const std::invalid_argument&)
    {
        if (cli_option == "1")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::contact;
        }
        if (cli_option == "2")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::nsset;
        }
        if (cli_option == "3")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::domain;
        }
        if (cli_option == "4")
        {
            std::cerr << "integer as object type is deprecated option" << std::endl;
            return ObjectType::keyset;
        }
    }
    throw std::runtime_error("unable convert to object type");
}

boost::optional<SetOfObjectTypes> construct_set_of_object_types_from_cli_options(const std::string& cli_options)
{
    if (cli_options.empty())
    {
        return boost::none;
    }

    SetOfObjectTypes set_of_object_types;

    static const std::regex delimiter(",");
    for (auto object_types_itr = std::sregex_token_iterator(cli_options.begin(), cli_options.end(), delimiter, -1);
         object_types_itr != std::sregex_token_iterator(); ++object_types_itr)
    {
        const ObjectType object_type = object_type_from_cli_option(*object_types_itr);
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

template <typename>
struct ObjectTypeCorrespondingWith { };

template <ObjectType>
struct TagCorrespondingWith { };

struct Domain_;
typedef DbId<Domain_> DomainId;
typedef DbHandle<Domain_> DomainFqdn;
template <>
struct ObjectTypeCorrespondingWith<Domain_>
{
    static constexpr ObjectType value = ObjectType::domain;
};
template <>
struct TagCorrespondingWith<ObjectType::domain>
{
    typedef Domain_ type;
};

struct Contact_;
typedef DbId<Contact_> ContactId;
typedef DbHandle<Contact_> ContactHandle;
template <>
struct ObjectTypeCorrespondingWith<Contact_>
{
    static constexpr ObjectType value = ObjectType::contact;
};
template <>
struct TagCorrespondingWith<ObjectType::contact>
{
    typedef Contact_ type;
};

struct Keyset_;
typedef DbId<Keyset_> KeysetId;
typedef DbHandle<Keyset_> KeysetHandle;
template <>
struct ObjectTypeCorrespondingWith<Keyset_>
{
    static constexpr ObjectType value = ObjectType::keyset;
};
template <>
struct TagCorrespondingWith<ObjectType::keyset>
{
    typedef Keyset_ type;
};

struct Nsset_;
typedef DbId<Nsset_> NssetId;
typedef DbHandle<Nsset_> NssetHandle;
template <>
struct ObjectTypeCorrespondingWith<Nsset_>
{
    static constexpr ObjectType value = ObjectType::nsset;
};
template <>
struct TagCorrespondingWith<ObjectType::nsset>
{
    typedef Nsset_ type;
};

struct Zone_;
typedef DbId<Zone_> ZoneId;

template <ObjectType object_type>
struct NotFound:std::runtime_error
{
    NotFound(const DbId<typename TagCorrespondingWith<object_type>::type>& _id)
        : std::runtime_error(to_db_handle(object_type).get_value() + " not found"),
          id(_id)
    { }
    const DbId<typename TagCorrespondingWith<object_type>::type> id;
};

template <ObjectType object_type>
struct NotExist:std::runtime_error
{
    NotExist(const DbId<typename TagCorrespondingWith<object_type>::type>& _id,
             const DbHandle<typename TagCorrespondingWith<object_type>::type>& _handle)
        : std::runtime_error(to_db_handle(object_type).get_value() + " " +
                             _handle.get_value() + ":" + boost::lexical_cast<std::string>(_id.get_value()) + " not exist"),
          id(_id),
          handle(_handle)
    { }
    const DbId<typename TagCorrespondingWith<object_type>::type> id;
    const DbHandle<typename TagCorrespondingWith<object_type>::type> handle;
};

template <ObjectType object_type>
struct CaseNormalize
{
    static const std::string sql_function;
};
template <ObjectType object_type>
const std::string CaseNormalize<object_type>::sql_function = "UPPER";

template <>
struct CaseNormalize<ObjectType::domain>
{
    static const std::string sql_function;
};
const std::string CaseNormalize<ObjectType::domain>::sql_function = "LOWER";

template <typename T>
const DbId<T>& lock_existing(LibFred::OperationContext& ctx, const DbId<T>& id)
{
    Database::query_param_list params(id.get_value());
    params(to_db_handle(ObjectTypeCorrespondingWith<T>::value).get_value());
    const Database::Result dbres = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT "
                "(SELECT id "
                 "FROM object_registry "
                 "WHERE id=obr.id AND "
                       "type=obr.type AND "
                       "erdate IS NULL "
                 "FOR UPDATE) IS NOT NULL," +
                CaseNormalize<ObjectTypeCorrespondingWith<T>::value>::sql_function + "(name) "
            "FROM object_registry obr "
            "WHERE id=$1::BIGINT AND "
                  "type=get_object_type_id($2::TEXT)",
            // clang-format on
            params);
    if (dbres.size() <= 0)
    {
        throw NotFound<ObjectTypeCorrespondingWith<T>::value>(id);
    }
    const bool object_exists = static_cast<bool>(dbres[0][0]);
    if (!object_exists)
    {
        const std::string object_handle = static_cast<std::string>(dbres[0][1]);
        throw NotExist<ObjectTypeCorrespondingWith<T>::value>(
                id,
                DbHandle<T>::construct_from(object_handle));
    }
    return id;
}

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
    DomainId lock(LibFred::OperationContext& _ctx)const
    {
        return lock_existing(_ctx, domain_id);
    }
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
    ContactId lock(LibFred::OperationContext& _ctx)const
    {
        return lock_existing(_ctx, contact_id);
    }
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
    KeysetId lock(LibFred::OperationContext& _ctx)const
    {
        return lock_existing(_ctx, keyset_id);
    }
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
    NssetId lock(LibFred::OperationContext& _ctx)const
    {
        return lock_existing(_ctx, nsset_id);
    }
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

    Random::Generator rnd_get;
    rnd_get.shuffle(domains.begin(), domains.end());
    const bool delete_more_than_domains = domains.size() < max_number_of_deletions;
    if (delete_more_than_domains)
    {
        rnd_get.shuffle(nssets.begin(), nssets.end());
        const bool delete_more_than_nssets = (domains.size() + nssets.size()) < max_number_of_deletions;
        if (delete_more_than_nssets)
        {
            rnd_get.shuffle(keysets.begin(), keysets.end());
            const bool delete_more_than_keysets = (domains.size() + nssets.size() + keysets.size()) < max_number_of_deletions;
            if (delete_more_than_keysets)
            {
                rnd_get.shuffle(contacts.begin(), contacts.end());
            }
        }
    }

    std::vector<double> relative_times_of_deletions;
    relative_times_of_deletions.reserve(max_number_of_deletions);
    for (unsigned idx = 0; idx < max_number_of_deletions; ++idx)
    {
        const auto relative_time_of_deletion = (idx + rnd_get.get(0.0, 1.0)) * relative_duration_of_one_deletion;
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
                const DomainToDelete& domain = domains[idx_of_deleted_domain];
                LibFred::DeleteDomainById(domain.lock(ctx).get_value()).exec(ctx);
                std::cerr << "domain " << domain.domain_fqdn.get_value() << " "
                             "(domain_id:" << domain.domain_id.get_value() << ", "
                             "zone_id:" << domain.zone_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else if (idx_of_deleted_object < (domains.size() + nssets.size()))
            {
                const unsigned idx_of_deleted_nsset = idx_of_deleted_object - domains.size();
                const NssetToDelete& nsset = nssets[idx_of_deleted_nsset];
                LibFred::DeleteNssetById(nsset.lock(ctx).get_value()).exec(ctx);
                std::cerr << "nsset " << nsset.nsset_handle.get_value() << " "
                             "(nsset_id:" << nsset.nsset_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else if (idx_of_deleted_object < (domains.size() + nssets.size() + keysets.size()))
            {
                const unsigned idx_of_deleted_keyset = idx_of_deleted_object - (domains.size() + nssets.size());
                const KeysetToDelete& keyset = keysets[idx_of_deleted_keyset];
                LibFred::DeleteKeysetById(keyset.lock(ctx).get_value()).exec(ctx);
                std::cerr << "keyset " << keyset.keyset_handle.get_value() << " "
                             "(keyset_id:" << keyset.keyset_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            else
            {
                const unsigned idx_of_deleted_contact = idx_of_deleted_object - (domains.size() + nssets.size() + keysets.size());
                const ContactToDelete& contact = contacts[idx_of_deleted_contact];
                LibFred::DeleteContactById(contact.lock(ctx).get_value()).exec(ctx);
                std::cerr << "contact " << contact.contact_handle.get_value() << " "
                             "(contact_id:" << contact.contact_id.get_value() << ") "
                             "successfully deleted" << std::endl;
            }
            ctx.commit_transaction();
        }
        catch (const NotFound<ObjectType::contact>& e)
        {
            LOGGER.error(boost::format("delete contact failed (%1%)") % e.what());
            const unsigned idx_of_deleted_contact = idx_of_deleted_object - (domains.size() + nssets.size() + keysets.size());
            std::cerr << "delete contact " << contacts[idx_of_deleted_contact].contact_handle.get_value() << " "
                         "(contact_id:" << contacts[idx_of_deleted_contact].contact_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const NotExist<ObjectType::contact>& e)
        {
            LOGGER.info(boost::format("delete contact failed (%1%)") % e.what());
        }
        catch (const NotFound<ObjectType::domain>& e)
        {
            LOGGER.error(boost::format("delete domain failed (%1%)") % e.what());
            const unsigned idx_of_deleted_domain = idx_of_deleted_object;
            std::cerr << "delete domain " << domains[idx_of_deleted_domain].domain_fqdn.get_value() << " "
                         "(domain_id:" << domains[idx_of_deleted_domain].domain_id.get_value() << ", "
                         "zone_id:" << domains[idx_of_deleted_domain].zone_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const NotExist<ObjectType::domain>& e)
        {
            LOGGER.info(boost::format("delete domain failed (%1%)") % e.what());
        }
        catch (const NotFound<ObjectType::keyset>& e)
        {
            LOGGER.error(boost::format("delete keyset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_keyset = idx_of_deleted_object - (domains.size() + nssets.size());
            std::cerr << "delete keyset " << keysets[idx_of_deleted_keyset].keyset_handle.get_value() << " "
                         "(keyset_id:" << keysets[idx_of_deleted_keyset].keyset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const NotExist<ObjectType::keyset>& e)
        {
            LOGGER.info(boost::format("delete keyset failed (%1%)") % e.what());
        }
        catch (const NotFound<ObjectType::nsset>& e)
        {
            LOGGER.error(boost::format("delete nsset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_nsset = idx_of_deleted_object - domains.size();
            std::cerr << "delete nsset " << nssets[idx_of_deleted_nsset].nsset_handle.get_value() << " "
                         "(nsset_id:" << nssets[idx_of_deleted_nsset].nsset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const NotExist<ObjectType::nsset>& e)
        {
            LOGGER.info(boost::format("delete nsset failed (%1%)") % e.what());
        }
        catch (const LibFred::DeleteContactById::Exception& e)
        {
            LOGGER.error(boost::format("delete contact failed (%1%)") % e.what());
            const unsigned idx_of_deleted_contact = idx_of_deleted_object - (domains.size() + nssets.size() + keysets.size());
            std::cerr << "delete contact " << contacts[idx_of_deleted_contact].contact_handle.get_value() << " "
                         "(contact_id:" << contacts[idx_of_deleted_contact].contact_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteDomainById::Exception& e)
        {
            LOGGER.error(boost::format("delete domain failed (%1%)") % e.what());
            const unsigned idx_of_deleted_domain = idx_of_deleted_object;
            std::cerr << "delete domain " << domains[idx_of_deleted_domain].domain_fqdn.get_value() << " "
                         "(domain_id:" << domains[idx_of_deleted_domain].domain_id.get_value() << ", "
                         "zone_id:" << domains[idx_of_deleted_domain].zone_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteKeysetById::Exception& e)
        {
            LOGGER.error(boost::format("delete keyset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_keyset = idx_of_deleted_object - (domains.size() + nssets.size());
            std::cerr << "delete keyset " << keysets[idx_of_deleted_keyset].keyset_handle.get_value() << " "
                         "(keyset_id:" << keysets[idx_of_deleted_keyset].keyset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const LibFred::DeleteNssetById::Exception& e)
        {
            LOGGER.error(boost::format("delete nsset failed (%1%)") % e.what());
            const unsigned idx_of_deleted_nsset = idx_of_deleted_object - domains.size();
            std::cerr << "delete nsset " << nssets[idx_of_deleted_nsset].nsset_handle.get_value() << " "
                         "(nsset_id:" << nssets[idx_of_deleted_nsset].nsset_id.get_value() << ") "
                         "failed: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            LOGGER.error(boost::format("operation failed (%1%)") % e.what());
        }
        catch (...)
        {
            LOGGER.error("unexpected exception");
        }
        ++idx_of_deleted_object;
    }
}

template void delete_objects_marked_as_delete_candidate<Debug::on>(
        int, const boost::optional<unsigned>&, const boost::optional<SetOfObjectTypes>&, const Seconds&);
template void delete_objects_marked_as_delete_candidate<Debug::off>(
        int, const boost::optional<unsigned>&, const boost::optional<SetOfObjectTypes>&, const Seconds&);

} // namespace Admin
