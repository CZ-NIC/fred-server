#ifndef _MODEL_LOGACTIONTYPE_H_
#define _MODEL_LOGACTIONTYPE_H_

/* << include database library settings here >> */
#include "db_settings.h"
#include "model.h"


class ModelLogActionType:
    public Model::Base {
public:
    ModelLogActionType()
    { }
    virtual ~ModelLogActionType()
    { }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const std::string &getStatus() const {
        return m_status.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setStatus(const std::string &status) {
        m_status = status;
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

    typedef Model::Field::List<ModelLogActionType>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_status;


public:
    static Model::Field::PrimaryKey<ModelLogActionType, unsigned long long> id;
    static Model::Field::Basic<ModelLogActionType, std::string> status;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelLogActionType

#endif // _MODEL_LOGACTIONTYPE_H_

