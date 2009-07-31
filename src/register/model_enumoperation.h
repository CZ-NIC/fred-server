#ifndef _MODEL_ENUMOPERATION_H_
#define _MODEL_ENUMOPERATION_H_

#include <string>
#include "db_settings.h"
#include "model.h"

class ModelEnumOperation:
    public Model::Base {
public:
    ModelEnumOperation()
    { }
    virtual ~ModelEnumOperation()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    const std::string &getOperation() const
    {
        return m_operation.get();
    }
    void setOperation(const std::string &operation)
    {
        m_operation = operation;
    }

    friend class Model::Base;

    void insert()
    {
        Model::Base::insert(this);
    }
    void update()
    {
        Model::Base::update(this);
    }


    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction transaction(c);
        Model::Base::reload(this);
        transaction.commit();
    }

protected:
    Field::Field<unsigned long long>      m_id;
    Field::Field<std::string>       m_operation;

    typedef Model::Field::List<ModelEnumOperation>  field_list;
public:
    static Model::Field::PrimaryKey<ModelEnumOperation, unsigned long long>   id;
    static Model::Field::Basic<ModelEnumOperation, std::string>         operation;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;
}; // class ModelEnumOperation

#endif // _MODEL_ENUMOPERATION_H_
