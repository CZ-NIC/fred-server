#ifndef _MODEL_LOGSESSION_H_
#define _MODEL_LOGSESSION_H_

/* << include database library settings here >> */
#include "db_settings.h"
#include "model.h"


class ModelLogSession:
    public Model::Base {
public:
    ModelLogSession()
    { }
    virtual ~ModelLogSession()
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
    const std::string &getLoginTr() const {
        return m_loginTr.get();
    }
    const std::string &getLogoutTr() const {
        return m_logoutTr.get();
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
    void setLoginTr(const std::string &loginTr) {
        m_loginTr = loginTr;
    }
    void setLogoutTr(const std::string &logoutTr) {
        m_logoutTr = logoutTr;
    }
    void setLang(const std::string &lang) {
        m_lang = lang;
    }

    friend class Model::Base;

    void insert() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::insert(this);
        tx.commit();
    }

    void update() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::update(this);
        tx.commit();
    }

    void reload() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::reload(this);
        tx.commit();
    }

    void load(const Database::Row &_data) {
        Model::Base::load(this, _data);
    }

    std::string toString() const {
        return Model::Base::toString(this);
    }

    typedef Model::Field::List<ModelLogSession>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;
    Field::Field<Database::DateTime> m_loginDate;
    Field::Field<Database::DateTime> m_logoutDate;
    Field::Field<std::string> m_loginTr;
    Field::Field<std::string> m_logoutTr;
    Field::Field<std::string> m_lang;


public:
    static Model::Field::PrimaryKey<ModelLogSession, unsigned long long> id;
    static Model::Field::Basic<ModelLogSession, std::string> name;
    static Model::Field::Basic<ModelLogSession, Database::DateTime> loginDate;
    static Model::Field::Basic<ModelLogSession, Database::DateTime> logoutDate;
    static Model::Field::Basic<ModelLogSession, std::string> loginTr;
    static Model::Field::Basic<ModelLogSession, std::string> logoutTr;
    static Model::Field::Basic<ModelLogSession, std::string> lang;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelLogSession

#endif // _MODEL_LOGSESSION_H_

