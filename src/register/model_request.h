#ifndef _MODEL_REQUEST_H_
#define _MODEL_REQUEST_H_

#include "db_settings.h"
#include "model.h"

class ModelRequest:
    public Model::Base {
public:
    ModelRequest()
    { }
    virtual ~ModelRequest()
    { }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const Database::DateTime &getTimeBegin() const {
        return m_timeBegin.get();
    }
    const Database::DateTime &getTimeEnd() const {
        return m_timeEnd.get();
    }
    const std::string &getSourceIp() const {
        return m_sourceIp.get();
    }
    const std::string &getUserName() const {
        return m_userName.get();
    }
    const unsigned long long &getServiceId() const {
        return m_serviceId.get();
    }
    /*
    ModelService *getService() {
        return service.getRelated(this);
    }
    */
    const unsigned long long &getActionTypeId() const {
        return m_actionTypeId.get();
    }
    /*
    ModelRequestType *getActionType() {
        return actionType.getRelated(this);
    }
    */
    const unsigned long long &getSessionId() const {
        return m_sessionId.get();
    }
    /*
    ModelSession *getSession() {
        return session.getRelated(this);
    }
    */
    const bool &getIsMonitoring() const {
        return m_isMonitoring.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setTimeBegin(const Database::DateTime &timeBegin) {
        m_timeBegin = timeBegin;
    }
    void setTimeEnd(const Database::DateTime &timeEnd) {
        m_timeEnd = timeEnd;
    }
    void setSourceIp(const std::string &sourceIp) {
        m_sourceIp = sourceIp;
    }
    void setUserName(const std::string &userName) {
        m_userName = userName;
    }
    void setServiceId(const unsigned long long &serviceId) {
        m_serviceId = serviceId;
    }
    /*
    void setService(ModelService *foreign_value) {
        service.setRelated(this, foreign_value);
    }
    */
    void setActionTypeId(const unsigned long long &actionTypeId) {
        m_actionTypeId = actionTypeId;
    }
    /*
    void setActionType(ModelRequestType *foreign_value) {
        actionType.setRelated(this, foreign_value);
    }
    */
    void setSessionId(const unsigned long long &sessionId) {
        m_sessionId = sessionId;
    }
    
    /*
    void setSession(ModelSession *foreign_value) {
        session.setRelated(this, foreign_value);
    }
    */
    void setIsMonitoring(const bool &isMonitoring) {
        m_isMonitoring = isMonitoring;
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

    typedef Model::Field::List<ModelRequest>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<Database::DateTime> m_timeBegin;
    Field::Field<Database::DateTime> m_timeEnd;
    Field::Field<std::string> m_sourceIp;
    Field::Field<std::string> m_userName;
    Field::Field<unsigned long long> m_serviceId;
    Field::Field<unsigned long long> m_actionTypeId;
    Field::Field<unsigned long long> m_sessionId;
    Field::Field<bool> m_isMonitoring;

    //Field::Lazy::Field<ModelService *> m_service;
    //Field::Lazy::Field<ModelRequestType *> m_actionType;
    //Field::Lazy::Field<ModelSession *> m_session;

public:
    static Model::Field::PrimaryKey<ModelRequest, unsigned long long> id;
    static Model::Field::Basic<ModelRequest, Database::DateTime> timeBegin;
    static Model::Field::Basic<ModelRequest, Database::DateTime> timeEnd;
    static Model::Field::Basic<ModelRequest, std::string> sourceIp;
    static Model::Field::Basic<ModelRequest, std::string> userName;
    static Model::Field::Basic<ModelRequest, unsigned long long> serviceId;
    static Model::Field::Basic<ModelRequest, unsigned long long> actionTypeId;
    static Model::Field::Basic<ModelRequest, unsigned long long> sessionId;
    static Model::Field::Basic<ModelRequest, bool> isMonitoring;

    //static Model::Field::Related::OneToOne<ModelRequest, unsigned long long, ModelService> service;
    //static Model::Field::Related::OneToOne<ModelRequest, unsigned long long, ModelRequestType> actionType;
    //static Model::Field::Related::OneToOne<ModelRequest, unsigned long long, ModelSession> session;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequest

#endif // _MODEL_REQUEST_H_

