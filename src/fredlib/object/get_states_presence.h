/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
*  header of Fred::Object::State class
*/
#ifndef GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82

#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"

#include <vector>
#include <sstream>

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

/**
 * @section get_states_presence_sample Sample of intended usage
 * @code{.cpp}
 * typedef State::set< State::DELETE_CANDIDATE,
 *                     State::DELETE_WARNING,
 *                     State::VALIDATED_CONTACT >::type states_for_check;
 * const Get< Type::CONTACT >::States< states_for_check >::Presence states_presence =
 *     Get< Type::CONTACT >(contact_id).states< states_for_check >().presence(ctx);
 * @endcode
 */

/**
 * Exception class used when given object doesn't exist.
 */
class doesnt_exist:public std::runtime_error
{
public:
    doesnt_exist()
    :   std::runtime_error("object doesn't exist")
    { }
    doesnt_exist(const std::string &_msg)
    :   std::runtime_error(_msg)
    { }
};

/**
 * Template class for getting information about fred object of given type.
 * @tparam OBJECT_TYPE type of fred object
 */
template < Type::Value OBJECT_TYPE >
class Get
{
public:
    /**
     * Object identified by numeric id.
     * @param _object_id object numeric id
     */
    Get(Id _object_id):id_(_object_id), handle_() { }
    /**
     * Object identified by string handle.
     * @param _object_handle object string handle
     */
    Get(const std::string &_object_handle):id_(INVALID_ID), handle_(_object_handle) { }
    /**
     * Exception class used when object of given type doesn't exist.
     */
    class object_doesnt_exist:public doesnt_exist
    {
    public:
        object_doesnt_exist()
        :   doesnt_exist(Type(OBJECT_TYPE).into< std::string >() + " doesn't exist")
        { }
        object_doesnt_exist(const std::string &_msg)
        :   doesnt_exist(_msg)
        { }
    };
    /**
     * Template class for getting information about given set of states.
     */
    template < typename STATES >
    class States
    {
    public:
        /**
         * Copy constructor.
         * @param _src source
         */
        States(const States &_src):object_(_src.object_) { }
        /**
         * Collects information about presence of given states.
         */
        class Presence
        {
        public:
            /**
             * Copy constructor.
             * @param _src source
             */
            Presence(const Presence &_src):state_presents_(_src.state_presents_) { }
            typedef bool Presents;///< true if state presents
            /**
             * Get information about given state.
             * @tparam STATE inspected state
             * @return true if given state presents
             * @note STATE must be member of STATES set
             */
            template < State::Value STATE >
            Presents get()const
            {
                return state_presents_[STATES::template at< STATE >::position];
            }
        private:
            Presence(OperationContext &_ctx, const Get &_object);
            template < int IDX = 0, int REST = STATES::count >
            class process_db_result;
            template < typename SET_OF_STATES = STATES, typename X = bool >
            class query;
            std::vector< Presents > state_presents_;
            friend class States;
        };
        /**
         * Obtains information about given states presence.
         * @param _ctx operation context
         * @return information about given states presence
         * @throw object_doesnt_exist in case of object doesn't exist in database
         */
        Presence presence(OperationContext &_ctx)const { return Presence(_ctx, object_); }
    private:
        States(const Get *_src_ptr):object_(*_src_ptr) { }
        friend class Get;
        const Get &object_;
    };
    /**
     * Creates instance of States< STATES > class used for obtaining information about given set of states.
     * @tparam STATES set of fred object states
     */
    template < typename STATES >
    States< STATES > states()const
    {
        return States< STATES >(this);
    }
private:
    enum { INVALID_ID = 0 };
    const Id id_;
    const std::string handle_;
    bool identified_by_handle()const { return !handle_.empty(); }
};

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
        template < typename X >
        class Get< OBJECT_TYPE >::States< STATES >::Presence::query< typename State::set< >::type, X >
        {
        private:
            static void append(const std::string&, const std::string&, const Database::query_param_list&) { }
            template < typename, typename >
            friend class query;
        };

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
        template < typename SET_OF_STATES, typename X >
        class Get< OBJECT_TYPE >::States< STATES >::Presence::query
        {
        public:
            static std::string by_id(Database::query_param_list &_params)
            {
                return query::by< ID >(_params);
            }
            static std::string by_handle(Database::query_param_list &_params)
            {
                return query::by< HANDLE >(_params);
            }
        private:
            static void add(std::string &_columns, std::string &_states, Database::query_param_list &_params)
            {
                static const std::string state_name = State(SET_OF_STATES::value).into< std::string >();
                const std::string param_idx = _params.add(state_name);
                std::cout << "$" << param_idx << "::TEXT = '" << state_name << "'" << std::endl;
                _columns.append("BOOL_OR(eos.name=$" + param_idx + "::TEXT)");
                _states.append("$" + param_idx + "::TEXT");
                typedef query< typename SET_OF_STATES::tail, X > tail;
                tail::append(_columns, _states, _params);
            }
            static void append(std::string &_columns, std::string &_states, Database::query_param_list &_params)
            {
                _columns.append(",");
                _states.append(",");
                query::add(_columns, _states, _params);
            }
            enum ObjectIdentifiedBy
            {
                ID,
                HANDLE
            };
            template < ObjectIdentifiedBy IDENTIFIED_BY, Type::Value TYPE_OF_OBJECT = OBJECT_TYPE, int Y = 0 >
            struct sql_object_registry;
            template < Type::Value TYPE_OF_OBJECT, int Y >
            struct sql_object_registry< ID, TYPE_OF_OBJECT, Y >
            {
                static std::string find(::size_t _param_pos, const std::string &_prefix = "")
                {
                    const std::string value = "$" + boost::lexical_cast< std::string >(_param_pos) + "::BIGINT";
                    // object_registry_pkey PRIMARY KEY (id) =>
                    //            obr.id=$1::BIGINT
                    return _prefix + "id=" + value;
                }
            };
            template < int Y >
            struct sql_object_registry< HANDLE, Type::CONTACT, Y >
            {
                static std::string find(::size_t _param_pos, const std::string &_prefix = "")
                {
                    const std::string value = "$" + boost::lexical_cast< std::string >(_param_pos) + "::TEXT";
                    // object_registry_upper_name_1_idx (UPPER(name)) WHERE type=1 =>
                    //                 UPPER(obr.name)=UPPER($1::TEXT) AND obr.type=1
                    return "UPPER(" + _prefix + "name)=UPPER(" + value + ") AND " + _prefix + "type=1";
                }
            };
            template < int Y >
            struct sql_object_registry< HANDLE, Type::NSSET, Y >
            {
                static std::string find(::size_t _param_pos, const std::string &_prefix = "")
                {
                    const std::string value = "$" + boost::lexical_cast< std::string >(_param_pos) + "::TEXT";
                    // object_registry_upper_name_2_idx (UPPER(name)) WHERE type=2 =>
                    //                 UPPER(obr.name)=UPPER($1::TEXT) AND obr.type=2
                    return "UPPER(" + _prefix + "name)=UPPER(" + value + ") AND " + _prefix + "type=2";
                }
            };
            template < int Y >
            struct sql_object_registry< HANDLE, Type::DOMAIN, Y >
            {
                static std::string find(::size_t _param_pos, const std::string &_prefix = "")
                {
                    const std::string value = "$" + boost::lexical_cast< std::string >(_param_pos) + "::TEXT";
                    // object_registr_name_3_idx (name) WHERE type=3 =>
                    //            obr.name=$1::TEXT AND obr.type=3
                    return _prefix + "name=" + value + " AND " + _prefix + "type=3";
                }
            };
            template < int Y >
            struct sql_object_registry< HANDLE, Type::KEYSET, Y >
            {
                static std::string find(::size_t _param_pos, const std::string &_prefix = "")
                {
                    const std::string value = "$" + boost::lexical_cast< std::string >(_param_pos) + "::TEXT";
                    // object_registry_upper_name_4_idx (UPPER(name)) WHERE type=4 =>
                    //                 UPPER(obr.name)=UPPER($1::TEXT) AND obr.type=4
                    return "UPPER(" + _prefix + "name)=UPPER(" + value + ") AND " + _prefix + "type=4";
                }
            };
            template < ObjectIdentifiedBy IDENTIFIED_BY >
            static std::string by(Database::query_param_list &_params)
            {
                std::string columns;
                std::string states;
                query::add(columns, states, _params);
                return "SELECT " + columns + " "
                       "FROM object_registry obr "
                       "LEFT JOIN object_state os ON os.object_id=obr.id AND os.valid_to IS NULL "
                       "LEFT JOIN enum_object_states eos ON eos.id=os.state_id AND eos.name IN (" + states + ") "
                       "WHERE " + sql_object_registry< IDENTIFIED_BY >::find(_params.size() + 1, "obr.") + " AND "
                             "obr.erdate IS NULL AND "
                             "obr.type=(SELECT id FROM enum_object_type "
                                       "WHERE name='" + Type(OBJECT_TYPE).into< std::string >() + "') "
                       "GROUP BY obr.id";
            }
            template < typename, typename >
            friend class query;
        };

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
        template < int IDX >
        class Get< OBJECT_TYPE >::States< STATES >::Presence::process_db_result< IDX, 0 >
        {
        public:
            process_db_result(const Database::Row&) { }
            void into(std::vector< Presents >&)const { }
        };

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
        template < int IDX, int REST >
        class Get< OBJECT_TYPE >::States< STATES >::Presence::process_db_result
        {
        public:
            process_db_result(const Database::Row &_row):columns_(_row) { }
            void into(std::vector< Presents > &_state_presents)const
            {
                _state_presents.push_back(!columns_[IDX].isnull() && static_cast< Presents >(columns_[IDX]));
                process_db_result< IDX + 1, REST - 1 >(columns_).into(_state_presents);
            }
        private:
            const Database::Row &columns_;
        };

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
    Get< OBJECT_TYPE >::States< STATES >::Presence::Presence(OperationContext &_ctx, const Get &_object)
    {
        std::string sql;
        Database::query_param_list params;
        if (_object.identified_by_handle()) {
            sql = query<>::by_handle(params);
            params(_object.handle_);
        }
        else {
            sql = query<>::by_id(params);
            params(_object.id_);
        }
        const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
        if (dbres.size() <= 0) {
            throw object_doesnt_exist();
        }
        state_presents_.reserve(STATES::count);
        process_db_result<>(dbres[0]).into(state_presents_);
    }

}//Fred::Object
}//Fred

#endif//GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82
