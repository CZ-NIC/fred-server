#ifndef _MODEL_BANKACCOUNT_H_
#define _MODEL_BANKACCOUNT_H_

#include "db_settings.h"
#include "util.h"
#include "model.h"
#include "model_zone.h"

class ModelBankAccount:
    public Model::Base {
public:
    ModelBankAccount()
    { }
    virtual ~ModelBankAccount()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    const unsigned long long getZoneId() const
    {
        return m_zoneId.get();
    }
    void setZoneId(const unsigned long long &zoneId)
    {
        m_zoneId = zoneId;
    }
    ModelZone *getZone()
    {
        return zone.getRelated(this);
    }
    void setZone(ModelZone *zon)
    {
        zone.setRelated(this, zon);
    }
    const std::string &getAccountNumber() const
    {
        return m_accountNumber.get();
    }
    void setAccountNumber(const std::string &accountNumber)
    {
        m_accountNumber = accountNumber;
    }
    const std::string &getAccountName() const
    {
        return m_accountName.get();
    }
    void setAccountName(const std::string &accountName)
    {
        m_accountName = accountName;
    }
    const std::string &getBankCode() const
    {
        return m_bankCode.get();
    }
    void setBankCode(const std::string &bankCode)
    {
        m_bankCode = bankCode;
    }
    const Database::Money &getBalance() const
    {
        return m_balance.get();
    }
    void setBalance(const Database::Money &balance)
    {
        m_balance = balance;
    }
    const Database::Date &getLastDate() const
    {
        return m_lastDate.get();
    }
    void setLastDate(const Database::Date &lastDate)
    {
        m_lastDate = lastDate;
    }
    const int &getLastNum() const
    {
        return m_lastNum.get();
    }
    void setLastNum(const int &lastNum)
    {
        m_lastNum = lastNum;
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
    Field::Field<unsigned long long>    m_id;
    Field::Field<unsigned long long>    m_zoneId;
    Field::Field<std::string>           m_accountNumber;
    Field::Field<std::string>           m_accountName;
    Field::Field<std::string>           m_bankCode;
    Field::Field<Database::Money>       m_balance;
    Field::Field<Database::Date>        m_lastDate;
    Field::Field<int>                   m_lastNum;

    Field::Lazy::Field<ModelZone *>          m_zone;

    typedef Model::Field::List<ModelBankAccount> field_list;
public:
    static Model::Field::PrimaryKey<ModelBankAccount, unsigned long long>    id;
    static Model::Field::ForeignKey<ModelBankAccount, unsigned long long, ModelZone>      zoneId;
    static Model::Field::Basic<ModelBankAccount, std::string>                accountNumber;
    static Model::Field::Basic<ModelBankAccount, std::string>                accountName;
    static Model::Field::Basic<ModelBankAccount, std::string>                bankCode;
    static Model::Field::Basic<ModelBankAccount, Database::Money>            balance;
    static Model::Field::Basic<ModelBankAccount, Database::Date>             lastDate;
    static Model::Field::Basic<ModelBankAccount, int>                        lastNum;

    static Model::Field::Related::OneToOne<ModelBankAccount, unsigned long long, ModelZone>       zone;

    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;

}; // class ModelBankAccount;
#endif // _MODEL_BANKACCOUNT_H_
