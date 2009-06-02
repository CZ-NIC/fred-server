#ifndef _MODEL_LOGPROPERTYVALUE_H_
#define _MODEL_LOGPROPERTYVALUE_H_

/* << include database library settings here >> */
#include "db_settings.h"
#include "model.h"
#include "model_log_entry.h"
#include "model_log_property_name.h"
#include "model_log_property_value.h"


class ModelLogPropertyValue:
    public Model::Base {
public:
    ModelLogPropertyValue()
    { }
    virtual ~ModelLogPropertyValue()
    { }
    const Database::DateTime &getEntryTimeBegin() const {
        return m_entryTimeBegin.get();
    }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const unsigned long long &getEntryId() const {
        return m_entryId.get();
    }
    ModelLogEntry *getEntry() {
        return entry.getRelated(this);
    }
    const unsigned long long &getNameId() const {
        return m_nameId.get();
    }
    ModelLogPropertyName *getName() {
        return name.getRelated(this);
    }
    const std::string &getValue() const {
        return m_value.get();
    }
    const bool &getOutput() const {
        return m_output.get();
    }
    const unsigned long long &getParentId() const {
        return m_parentId.get();
    }
    ModelLogPropertyValue *getParent() {
        return parent.getRelated(this);
    }
    void setEntryTimeBegin(const Database::DateTime &entryTimeBegin) {
        m_entryTimeBegin = entryTimeBegin;
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setEntryId(const unsigned long long &entryId) {
        m_entryId = entryId;
    }
    void setEntry(ModelLogEntry *foreign_value) {
        entry.setRelated(this, foreign_value);
    }
    void setNameId(const unsigned long long &nameId) {
        m_nameId = nameId;
    }
    void setName(ModelLogPropertyName *foreign_value) {
        name.setRelated(this, foreign_value);
    }
    void setValue(const std::string &value) {
        m_value = value;
    }
    void setOutput(const bool &output) {
        m_output = output;
    }
    void setParentId(const unsigned long long &parentId) {
        m_parentId = parentId;
    }
    void setParent(ModelLogPropertyValue *foreign_value) {
        parent.setRelated(this, foreign_value);
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

    typedef Model::Field::List<ModelLogPropertyValue>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<Database::DateTime> m_entryTimeBegin;
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_entryId;
    Field::Field<unsigned long long> m_nameId;
    Field::Field<std::string> m_value;
    Field::Field<bool> m_output;
    Field::Field<unsigned long long> m_parentId;

    Field::Lazy::Field<ModelLogEntry *> m_entry;
    Field::Lazy::Field<ModelLogPropertyName *> m_name;
    Field::Lazy::Field<ModelLogPropertyValue *> m_parent;

public:
    static Model::Field::Basic<ModelLogPropertyValue, Database::DateTime> entryTimeBegin;
    static Model::Field::PrimaryKey<ModelLogPropertyValue, unsigned long long> id;
    static Model::Field::ForeignKey<ModelLogPropertyValue, unsigned long long, ModelLogEntry> entryId;
    static Model::Field::ForeignKey<ModelLogPropertyValue, unsigned long long, ModelLogPropertyName> nameId;
    static Model::Field::Basic<ModelLogPropertyValue, std::string> value;
    static Model::Field::Basic<ModelLogPropertyValue, bool> output;
    static Model::Field::ForeignKey<ModelLogPropertyValue, unsigned long long, ModelLogPropertyValue> parentId;

    static Model::Field::Related::OneToOne<ModelLogPropertyValue, unsigned long long, ModelLogEntry> entry;
    static Model::Field::Related::OneToOne<ModelLogPropertyValue, unsigned long long, ModelLogPropertyName> name;
    static Model::Field::Related::OneToOne<ModelLogPropertyValue, unsigned long long, ModelLogPropertyValue> parent;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelLogPropertyValue

#endif // _MODEL_LOGPROPERTYVALUE_H_

