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
    const Database::DateTime &getRequestTimeBegin() const {
        return m_requestTimeBegin.get();
    }
    const int &getRequestService() const {
        return m_requestServiceId.get();
    }
    const bool &getRequestMonitoring() const {
        return m_requestMonitoring.get();
    }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const unsigned long long &getRequestId() const {
        return m_requestId.get();
    }
    const int &getPropertyNameId() const {
        return m_propertyNameId.get();
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
    void setRequestTimeBegin(const Database::DateTime &requestTimeBegin) {
        m_requestTimeBegin = requestTimeBegin;
    }
    void setRequestServiceId(const int &requestServiceId) {
        m_requestServiceId = requestServiceId;
    }
    void setRequestMonitoring(const bool &requestMonitoring) {
        m_requestMonitoring = requestMonitoring;
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setRequestId(const unsigned long long &requestId) {
        m_requestId = requestId;
    }
    void setPropertyNameId(const int &propertyNameId) {
        m_propertyNameId = propertyNameId;
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
    Field::Field<Database::DateTime> m_requestTimeBegin;
    Field::Field<int> m_requestServiceId;
    Field::Field<bool> m_requestMonitoring;
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_requestId;
    Field::Field<int> m_propertyNameId;
    Field::Field<std::string> m_value;
    Field::Field<bool> m_output;
    Field::Field<unsigned long long> m_parentId;

    //Field::Lazy::Field<ModelRequestPropertyValue *> m_parent;

public:
    static Model::Field::Basic<ModelRequestPropertyValue, Database::DateTime> requestTimeBegin;
    static Model::Field::Basic<ModelRequestPropertyValue, int> requestServiceId;
    static Model::Field::Basic<ModelRequestPropertyValue, bool> requestMonitoring;
    static Model::Field::PrimaryKey<ModelRequestPropertyValue, unsigned long long> id;
    static Model::Field::Basic<ModelRequestPropertyValue, unsigned long long> requestId;
    static Model::Field::Basic<ModelRequestPropertyValue, int> propertyNameId;
    static Model::Field::Basic<ModelRequestPropertyValue, std::string> value;
    static Model::Field::Basic<ModelRequestPropertyValue, bool> output;
    static Model::Field::Basic<ModelRequestPropertyValue, unsigned long long> parentId;

    //static Model::Field::Related::OneToOne<ModelRequestPropertyValue, unsigned long long, ModelRequestPropertyValue> parent;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestPropertyValue

#endif // _MODEL_REQUESTPROPERTYVALUE_H_

