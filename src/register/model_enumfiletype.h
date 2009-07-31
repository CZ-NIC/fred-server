#ifndef _MODEL_ENUMFILETYPE_H_
#define _MODEL_ENUMFILETYPE_H_

#include "db_settings.h"
#include "model.h"


class ModelEnumFileType:
    public Model::Base {
public:
    ModelEnumFileType()
    { }
    virtual ~ModelEnumFileType()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const std::string &getName() const
    {
        return m_name.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setName(const std::string &name)
    {
        m_name = name;
    }

    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::update(this);
        tx.commit();
    }

    void reload()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::reload(this);
        tx.commit();
    }

    void load(const Database::Row &_data)
    {
        Model::Base::load(this, _data);
    }

    typedef Model::Field::List<ModelEnumFileType>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;


public:
    static Model::Field::PrimaryKey<ModelEnumFileType, unsigned long long> id;
    static Model::Field::Basic<ModelEnumFileType, std::string> name;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelEnumFileType

#endif // _MODEL_ENUMFILETYPE_H_

