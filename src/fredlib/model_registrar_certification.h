#ifndef _MODEL_REGISTRAR_CERTIFICATION_H_
#define _MODEL_REGISTRAR_CERTIFICATION_H_

#include "db_settings.h"
#include "model.h"

class ModelRegistrarCertification:
    public Model::Base {
public:
    ModelRegistrarCertification()
    { }
    virtual ~ModelRegistrarCertification()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getRegistrarId() const
    {
        return m_registrar_id.get();
    }
    const Database::Date &getValidFrom() const
    {
        return m_valid_from.get();
    }
    const Database::Date &getValidUntil() const
    {
        return m_valid_until.get();
    }
    const int &getClassification() const
    {
        return m_classification.get();
    }
    const unsigned long long &getEvalFileId() const
    {
        return m_eval_file_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setRegistrarId(const unsigned long long &registrarId)
    {
        m_registrar_id = registrarId;
    }
    void setValidFrom(const Database::Date &validFrom)
    {
        m_valid_from = validFrom;
    }
    void setValidUntil(const Database::Date &validUntil)
    {
        m_valid_until = validUntil;
    }
    void setClassification(const int &classification)
    {
        m_classification = classification;
    }
    void setEvalFileId(const unsigned long long &eval_file_id)
    {
        m_eval_file_id = eval_file_id;
    }
    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        //serialization of constraint check in trigger function
        conn.exec("LOCK TABLE registrar_certification IN ACCESS EXCLUSIVE MODE");
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        //serialization of constraint check in trigger function
        conn.exec("LOCK TABLE registrar_certification IN ACCESS EXCLUSIVE MODE");
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

    typedef Model::Field::List<ModelRegistrarCertification>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_registrar_id;
    Field::Field<Database::Date> m_valid_from;
    Field::Field<Database::Date> m_valid_until;
    Field::Field<int> m_classification;
    Field::Field<unsigned long long> m_eval_file_id;

public:
    static Model::Field::PrimaryKey<ModelRegistrarCertification, unsigned long long> id;
    static Model::Field::Basic<ModelRegistrarCertification, unsigned long long> registrar_id;
    static Model::Field::Basic<ModelRegistrarCertification, Database::Date> valid_from;
    static Model::Field::Basic<ModelRegistrarCertification, Database::Date> valid_until;
    static Model::Field::Basic<ModelRegistrarCertification, int> classification;
    static Model::Field::Basic<ModelRegistrarCertification, unsigned long long> eval_file_id;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRegistrarCertification

#endif // _MODEL_REGISTRAR_CERTIFICATION_H_
