#ifndef MODEL_REQUEST_PROPERTY_NAME_HH_84796DAB95D94B52BEB7508325CF9B0B
#define MODEL_REQUEST_PROPERTY_NAME_HH_84796DAB95D94B52BEB7508325CF9B0B

#include "libfred/db_settings.hh"
#include "src/util/db/model/model.hh"


class ModelRequestPropertyName:
    public Model::Base {
public:
    ModelRequestPropertyName()
    { }
    virtual ~ModelRequestPropertyName()
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

    typedef Model::Field::List<ModelRequestPropertyName>  field_list;
    static const field_list& getFields() {
        return fields;
    }
    static const std::string &getTableName() {
        return table_name;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;


public:
    static Model::Field::PrimaryKey<ModelRequestPropertyName, unsigned long long> id;
    static Model::Field::Basic<ModelRequestPropertyName, std::string> name;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestPropertyName

#endif // _MODEL_REQUESTPROPERTY_H_

