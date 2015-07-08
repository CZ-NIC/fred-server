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

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

/**
 * @section sample Sample of intended usage
 * @code{.cpp}
 * typedef State::set< State::DELETE_CANDIDATE,
 *                     State::DELETE_WARNING,
 *                     State::VALIDATED_CONTACT > states_for_check;
 * const Get< Type::CONTACT >::States< SetOfStates >::Presence states_presence =
 *     Get< Type::CONTACT >(contact_id).states< states_for_check >().presence(ctx);
 * @endcode
 */

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
     * Exception class used when given object doesn't exist.
     */
    class object_doesnt_exist:public std::runtime_error
    {
    public:
        object_doesnt_exist()
        :   std::runtime_error(Type(OBJECT_TYPE).into< std::string >() + " doesn't exist")
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
            static std::string& append_column(std::string &_columns) { return _columns; }
            template < typename FRIEND_STATES, typename FRIEND_X >
            friend class query;
        };

template < Type::Value OBJECT_TYPE >
    template < typename STATES >
        template < typename SET_OF_STATES, typename X >
        class Get< OBJECT_TYPE >::States< STATES >::Presence::query
        {
        public:
            static std::string by_id()
            {
                static std::string sql;
                if (sql.empty()) {
                    std::string columns;
                    query::add_column(columns);
                    sql = "SELECT " + columns + " "
                          "FROM object_registry obr "
                          "LEFT JOIN object_state os ON os.object_id=obr.id AND os.valid_to IS NULL "
                          "LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
                          "WHERE obr.id=$1::BIGINT AND "
                                "obr.erdate IS NULL AND "
                                "obr.type=(SELECT id FROM enum_object_type "
                                          "WHERE name='" + Type(OBJECT_TYPE).into< std::string >() + "') "
                          "GROUP BY obr.id";
                }
                return sql;
            }
            static std::string by_handle()
            {
                static std::string sql;
                if (sql.empty()) {
                    std::string columns;
                    query::add_column(columns);
                    sql = "SELECT " + columns + " "
                          "FROM object_registry obr "
                          "LEFT JOIN object_state os ON os.object_id=obr.id AND os.valid_to IS NULL "
                          "LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
                          "WHERE obr.name=$1::TEXT AND "
                                "obr.erdate IS NULL AND "
                                "obr.type=(SELECT id FROM enum_object_type "
                                          "WHERE name='" + Type(OBJECT_TYPE).into< std::string >() + "') "
                          "GROUP BY obr.id";
                }
                return sql;
            }
        private:
            static std::string& add_column(std::string &columns)
            {
                static const std::string state_name = State(SET_OF_STATES::value).into< std::string >();
                columns.append("BOOL_OR(eos.name='" + state_name + "')");
                typedef query< typename SET_OF_STATES::tail, X > tail;
                return tail::append_column(columns);
            }
            static std::string& append_column(std::string &columns)
            {
                columns.append(",");
                return query::add_column(columns);
            }
            template < typename FRIEND_STATES, typename FRIEND_X >
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
        const Database::Result dbres = _object.identified_by_handle()
            ? _ctx.get_conn().exec_params(query<>::by_handle(), Database::query_param_list(_object.handle_))
            : _ctx.get_conn().exec_params(query<>::by_id(),     Database::query_param_list(_object.id_));
        if (dbres.size() <= 0) {
            throw object_doesnt_exist();
        }
        state_presents_.reserve(STATES::count);
        process_db_result<>(dbres[0]).into(state_presents_);
    }

}//Fred::Object
}//Fred

#endif//GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82
