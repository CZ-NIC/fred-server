#ifndef _MODEL_REQUESTTYPE_H_
#define _MODEL_REQUESTTYPE_H_

#include "db_settings.h"
#include "model.h"


class ModelRequestType:
    public Model::Base {
public:
    ModelRequestType()
    { }
    virtual ~ModelRequestType()
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

    typedef Model::Field::List<ModelRequestType>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_status;


public:
    static Model::Field::PrimaryKey<ModelRequestType, unsigned long long> id;
    static Model::Field::Basic<ModelRequestType, std::string> status;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestType

#endif // _MODEL_REQUESTTYPE_H_

