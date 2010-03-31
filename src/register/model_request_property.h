#ifndef _MODEL_REQUESTPROPERTY_H_
#define _MODEL_REQUESTPROPERTY_H_

#include "db_settings.h"
#include "model.h"


class ModelRequestProperty:
    public Model::Base {
public:
    ModelRequestProperty()
    { }
    virtual ~ModelRequestProperty()
    { }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const std::string &getName() const {
        return m_name.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setName(const std::string &name) {
        m_name = name;
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

    typedef Model::Field::List<ModelRequestProperty>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;


public:
    static Model::Field::PrimaryKey<ModelRequestProperty, unsigned long long> id;
    static Model::Field::Basic<ModelRequestProperty, std::string> name;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestProperty

#endif // _MODEL_REQUESTPROPERTY_H_

