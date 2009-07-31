#ifndef _MODEL_ENUMBANKCODE_H_
#define _MODEL_ENUMBANKCODE_H_

#include "db_settings.h"
#include "model.h"


class ModelEnumBankCode:
    public Model::Base {
public:
    ModelEnumBankCode()
    { }
    virtual ~ModelEnumBankCode()
    { }
    const std::string &getCode() const
    {
        return m_code.get();
    }
    const std::string &getNameShort() const
    {
        return m_nameShort.get();
    }
    const std::string &getNameFull() const
    {
        return m_nameFull.get();
    }
    void setCode(const std::string &code)
    {
        m_code = code;
    }
    void setNameShort(const std::string &nameShort)
    {
        m_nameShort = nameShort;
    }
    void setNameFull(const std::string &nameFull)
    {
        m_nameFull = nameFull;
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

    std::string toString() const
    {
        return Model::Base::toString(this);
    }

    typedef Model::Field::List<ModelEnumBankCode>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<std::string> m_code;
    Field::Field<std::string> m_nameShort;
    Field::Field<std::string> m_nameFull;


public:
    static Model::Field::PrimaryKey<ModelEnumBankCode, std::string> code;
    static Model::Field::Basic<ModelEnumBankCode, std::string> nameShort;
    static Model::Field::Basic<ModelEnumBankCode, std::string> nameFull;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelEnumBankCode

#endif // _MODEL_ENUMBANKCODE_H_

