#ifndef _MODEL_SESSION_H_
#define _MODEL_SESSION_H_

#include "db_settings.h"
#include "model.h"


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
    const std::string &getName() const {
        return m_name.get();
    }
    const Database::DateTime &getLoginDate() const {
        return m_loginDate.get();
    }
    const Database::DateTime &getLogoutDate() const {
        return m_logoutDate.get();
    }
    const std::string &getLang() const {
        return m_lang.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setName(const std::string &name) {
        m_name = name;
    }
    void setLoginDate(const Database::DateTime &loginDate) {
        m_loginDate = loginDate;
    }
    void setLogoutDate(const Database::DateTime &logoutDate) {
        m_logoutDate = logoutDate;
    }
    void setLang(const std::string &lang) {
        m_lang = lang;
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
    Field::Field<std::string> m_name;
    Field::Field<Database::DateTime> m_loginDate;
    Field::Field<Database::DateTime> m_logoutDate;
    Field::Field<std::string> m_lang;


public:
    static Model::Field::PrimaryKey<ModelSession, unsigned long long> id;
    static Model::Field::Basic<ModelSession, std::string> name;
    static Model::Field::Basic<ModelSession, Database::DateTime> loginDate;
    static Model::Field::Basic<ModelSession, Database::DateTime> logoutDate;
    static Model::Field::Basic<ModelSession, std::string> lang;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelSession

#endif // _MODEL_SESSION_H_

