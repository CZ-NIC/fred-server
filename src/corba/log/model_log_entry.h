#ifndef _MODEL_LOGENTRY_H_
#define _MODEL_LOGENTRY_H_

/* << include database library settings here >> */
#include "db_settings.h"
#include "model.h"
#include "model_log_action_type.h"


class ModelLogEntry:
    public Model::Base {
public:
    ModelLogEntry()
    { }
    virtual ~ModelLogEntry()
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
    const int &getService() const {
        return m_service.get();
    }
    const unsigned long long &getActionTypeId() const {
        return m_actionTypeId.get();
    }
    ModelLogActionType *getActionType() {
        return actionType.getRelated(this);
    }
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
    void setService(const int &service) {
        m_service = service;
    }
    void setActionTypeId(const unsigned long long &actionTypeId) {
        m_actionTypeId = actionTypeId;
    }
    void setActionType(ModelLogActionType *foreign_value) {
        actionType.setRelated(this, foreign_value);
    }
    void setIsMonitoring(const bool &isMonitoring) {
        m_isMonitoring = isMonitoring;
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

    typedef Model::Field::List<ModelLogEntry>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<Database::DateTime> m_timeBegin;
    Field::Field<Database::DateTime> m_timeEnd;
    Field::Field<std::string> m_sourceIp;
    Field::Field<int> m_service;
    Field::Field<unsigned long long> m_actionTypeId;
    Field::Field<bool> m_isMonitoring;

    Field::Lazy::Field<ModelLogActionType *> m_actionType;

public:
    static Model::Field::PrimaryKey<ModelLogEntry, unsigned long long> id;
    static Model::Field::Basic<ModelLogEntry, Database::DateTime> timeBegin;
    static Model::Field::Basic<ModelLogEntry, Database::DateTime> timeEnd;
    static Model::Field::Basic<ModelLogEntry, std::string> sourceIp;
    static Model::Field::Basic<ModelLogEntry, int> service;
    static Model::Field::ForeignKey<ModelLogEntry, unsigned long long, ModelLogActionType> actionTypeId;
    static Model::Field::Basic<ModelLogEntry, bool> isMonitoring;

    static Model::Field::Related::OneToOne<ModelLogEntry, unsigned long long, ModelLogActionType> actionType;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelLogEntry

#endif // _MODEL_LOGENTRY_H_

