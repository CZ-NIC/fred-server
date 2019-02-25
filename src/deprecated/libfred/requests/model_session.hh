/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef MODEL_SESSION_HH_BF2491A08D684031867D641734B517EF
#define MODEL_SESSION_HH_BF2491A08D684031867D641734B517EF

#include "libfred/db_settings.hh"
#include "src/util/db/model/model.hh"


class ModelSession:
    public Model::Base {
public:
    ModelSession()
    { }
    virtual ~ModelSession()
    { }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    bool isUserNameSet() const {
        return m_userName.isChanged();
    }
    const std::string &getUserName() const {
        return m_userName.get();
    }
    bool isUserIdSet() const {
        return m_userId.isChanged();
    }
    const unsigned long long &getUserId() const {
        return m_userId.get();
    }
    const Database::DateTime &getLoginDate() const {
        return m_loginDate.get();
    }
    const Database::DateTime &getLogoutDate() const {
        return m_logoutDate.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setUserName(const std::string &userName) {
        m_userName = userName;
    }
    void setUserId(const unsigned long long &userId) {
        m_userId = userId;
    }
    void setLoginDate(const Database::DateTime &loginDate) {
        m_loginDate = loginDate;
    }
    void setLogoutDate(const Database::DateTime &logoutDate) {
        m_logoutDate = logoutDate;
    }

    friend class Model::Base;

    void insert() {
        Database::Connection conn = Database::Manager::acquire();
        Model::Base::insert(this);
    }

    void update() {
        Database::Connection conn = Database::Manager::acquire();
        Model::Base::update(this);
    }

    void reload() {
        Database::Connection conn = Database::Manager::acquire();
        Model::Base::reload(this);
    }

    void load(const Database::Row &_data) {
        Model::Base::load(this, _data);
    }

    std::string toString() const {
        return Model::Base::toString(this);
    }

    typedef Model::Field::List<ModelSession>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_userName;
    Field::Field<unsigned long long> m_userId;
    Field::Field<Database::DateTime> m_loginDate;
    Field::Field<Database::DateTime> m_logoutDate;


public:
    static Model::Field::PrimaryKey<ModelSession, unsigned long long> id;
    static Model::Field::Basic<ModelSession, std::string> userName;
    static Model::Field::Basic<ModelSession, unsigned long long> userId;
    static Model::Field::Basic<ModelSession, Database::DateTime> loginDate;
    static Model::Field::Basic<ModelSession, Database::DateTime> logoutDate;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelSession

#endif // _MODEL_SESSION_H_

