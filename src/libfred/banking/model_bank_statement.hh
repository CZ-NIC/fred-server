#ifndef MODEL_BANK_STATEMENT_HH_941F6CC8C9F04564A535EEB7DA882204
#define MODEL_BANK_STATEMENT_HH_941F6CC8C9F04564A535EEB7DA882204

#include "src/libfred/db_settings.hh"
#include "src/util/db/model/model.hh"



class ModelBankStatement:
    public Model::Base {
public:
    ModelBankStatement()
    { }
    virtual ~ModelBankStatement()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getAccountId() const
    {
        return m_accountId.get();
    }
    /*
    ModelBankAccount *getAccount()
    {
        return account.getRelated(this);
    }
    */
    const int &getNum() const
    {
        return m_num.get();
    }
    const Database::Date &getCreateDate() const
    {
        return m_createDate.get();
    }
    const Database::Date &getBalanceOldDate() const
    {
        return m_balanceOldDate.get();
    }
    std::string getBalanceOld() const
    {
        return m_balanceOld.get();
    }
    std::string getBalanceNew() const
    {
        return m_balanceNew.get();
    }
    std::string getBalanceCredit() const
    {
        return m_balanceCredit.get();
    }
    std::string getBalanceDebet() const
    {
        return m_balanceDebet.get();
    }
    const unsigned long long &getFileId() const
    {
        return m_fileId.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setAccountId(const unsigned long long &accountId)
    {
        m_accountId = accountId;
    }
    /*
    void setAccount(ModelBankAccount *foreign_value)
    {
        account.setRelated(this, foreign_value);
    }
    */
    void setNum(const int &num)
    {
        m_num = num;
    }
    void setCreateDate(const Database::Date &createDate)
    {
        m_createDate = createDate;
    }
    void setBalanceOldDate(const Database::Date &balanceOldDate)
    {
        m_balanceOldDate = balanceOldDate;
    }
    void setBalanceOld(const std::string &balanceOld)
    {
        m_balanceOld = balanceOld;
    }
    void setBalanceNew(const std::string &balanceNew)
    {
        m_balanceNew = balanceNew;
    }
    void setBalanceCredit(const std::string &balanceCredit)
    {
        m_balanceCredit = balanceCredit;
    }
    void setBalanceDebet(const std::string &balanceDebet)
    {
        m_balanceDebet = balanceDebet;
    }
    void setFileId(const unsigned long long &fileId)
    {
        m_fileId = fileId;
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

    typedef Model::Field::List<ModelBankStatement>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_accountId;
    Field::Field<int> m_num;
    Field::Field<Database::Date> m_createDate;
    Field::Field<Database::Date> m_balanceOldDate;
    Field::Field<std::string> m_balanceOld;
    Field::Field<std::string> m_balanceNew;
    Field::Field<std::string> m_balanceCredit;
    Field::Field<std::string> m_balanceDebet;
    Field::Field<unsigned long long> m_fileId;

    //Field::Lazy::Field<ModelBankAccount *> m_account;

public:
    static Model::Field::PrimaryKey<ModelBankStatement, unsigned long long> id;
    static Model::Field::Basic<ModelBankStatement, unsigned long long> accountId;
    static Model::Field::Basic<ModelBankStatement, int> num;
    static Model::Field::Basic<ModelBankStatement, Database::Date> createDate;
    static Model::Field::Basic<ModelBankStatement, Database::Date> balanceOldDate;
    static Model::Field::Basic<ModelBankStatement, std::string> balanceOld;
    static Model::Field::Basic<ModelBankStatement, std::string> balanceNew;
    static Model::Field::Basic<ModelBankStatement, std::string> balanceCredit;
    static Model::Field::Basic<ModelBankStatement, std::string> balanceDebet;
    static Model::Field::Basic<ModelBankStatement, unsigned long long> fileId;

    //static Model::Field::Related::OneToOne<ModelBankStatement, unsigned long long, ModelBankAccount> account;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelBankStatement

#endif // _MODEL_BANK_STATEMENT_H_

