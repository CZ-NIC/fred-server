#ifndef _MODEL_REQUESTPROPERTYVALUE_H_
#define _MODEL_REQUESTPROPERTYVALUE_H_

#include "db_settings.h"
#include "model.h"

class ModelRequestPropertyValue:
    public Model::Base {
public:
    ModelRequestPropertyValue()
    { }
    virtual ~ModelRequestPropertyValue()
    { }
    const Database::DateTime &getEntryTimeBegin() const {
        return m_entryTimeBegin.get();
    }
    const int &getEntryService() const {
        return m_entryService.get();
    }
    const bool &getEntryMonitoring() const {
        return m_entryMonitoring.get();
    }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const int &getEntry() const {
        return m_entry.get();
    }
    const int &getName() const {
        return m_name.get();
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
    /*
    ModelRequestPropertyValue *getParent() {
        return parent.getRelated(this);
    }
    */
    void setEntryTimeBegin(const Database::DateTime &entryTimeBegin) {
        m_entryTimeBegin = entryTimeBegin;
    }
    void setEntryService(const int &entryService) {
        m_entryService = entryService;
    }
    void setEntryMonitoring(const bool &entryMonitoring) {
        m_entryMonitoring = entryMonitoring;
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setEntry(const int &entry) {
        m_entry = entry;
    }
    void setName(const int &name) {
        m_name = name;
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
    /*
    void setParent(ModelRequestPropertyValue *foreign_value) {
        parent.setRelated(this, foreign_value);
    }
     */
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

    typedef Model::Field::List<ModelRequestPropertyValue>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<Database::DateTime> m_entryTimeBegin;
    Field::Field<int> m_entryService;
    Field::Field<bool> m_entryMonitoring;
    Field::Field<unsigned long long> m_id;
    Field::Field<int> m_entry;
    Field::Field<int> m_name;
    Field::Field<std::string> m_value;
    Field::Field<bool> m_output;
    Field::Field<unsigned long long> m_parentId;

    //Field::Lazy::Field<ModelRequestPropertyValue *> m_parent;

public:
    static Model::Field::Basic<ModelRequestPropertyValue, Database::DateTime> entryTimeBegin;
    static Model::Field::Basic<ModelRequestPropertyValue, int> entryService;
    static Model::Field::Basic<ModelRequestPropertyValue, bool> entryMonitoring;
    static Model::Field::PrimaryKey<ModelRequestPropertyValue, unsigned long long> id;
    static Model::Field::Basic<ModelRequestPropertyValue, int> entry;
    static Model::Field::Basic<ModelRequestPropertyValue, int> name;
    static Model::Field::Basic<ModelRequestPropertyValue, std::string> value;
    static Model::Field::Basic<ModelRequestPropertyValue, bool> output;
    static Model::Field::Basic<ModelRequestPropertyValue, unsigned long long> parentId;

    //static Model::Field::Related::OneToOne<ModelRequestPropertyValue, unsigned long long, ModelRequestPropertyValue> parent;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestPropertyValue

#endif // _MODEL_REQUESTPROPERTYVALUE_H_

