#ifndef MODEL_BANK_PAYMENT_HH_129A3EF858334CE380D6A07003BF0D0C
#define MODEL_BANK_PAYMENT_HH_129A3EF858334CE380D6A07003BF0D0C

#include "src/libfred/db_settings.hh"
#include "src/util/db/model/model.hh"

class ModelBankPayment:
    public Model::Base {
public:
    ModelBankPayment()
    { }
    virtual ~ModelBankPayment()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getStatementId() const
    {
        return m_statementId.get();
    }
    const unsigned long long &getAccountId() const
    {
        return m_accountId.get();
    }
    const std::string &getAccountNumber() const
    {
        return m_accountNumber.get();
    }
    const std::string &getBankCodeId() const
    {
        return m_bankCodeId.get();
    }
    const int &getCode() const
    {
        return m_code.get();
    }
    const int &getType() const
    {
        return m_type.get();
    }
    const int &getStatus() const
    {
        return m_status.get();
    }
    const std::string &getKonstSym() const
    {
        return m_konstSym.get();
    }
    const std::string &getVarSymb() const
    {
        return m_varSymb.get();
    }
    const std::string &getSpecSymb() const
    {
        return m_specSymb.get();
    }
    std::string getPrice() const
    {
        return m_price.get();
    }
    const std::string &getAccountEvid() const
    {
        return m_accountEvid.get();
    }
    const Database::Date &getAccountDate() const
    {
        return m_accountDate.get();
    }
    const std::string &getAccountMemo() const
    {
        return m_accountMemo.get();
    }
    const std::string &getAccountName() const
    {
        return m_accountName.get();
    }
    const Database::DateTime &getCrTime() const
    {
        return m_crTime.get();
    }
    const std::string &getUuid() const
    {
        return m_uuid.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setStatementId(const unsigned long long &statementId)
    {
        m_statementId = statementId;
    }
    void setAccountId(const unsigned long long &accountId)
    {
        m_accountId = accountId;
    }
    void setAccountNumber(const std::string &accountNumber)
    {
        m_accountNumber = accountNumber;
    }
    void setBankCodeId(const std::string &bankCodeId)
    {
        m_bankCodeId = bankCodeId;
    }
    void setCode(const int &code)
    {
        m_code = code;
    }
    void setType(const int &type)
    {
        m_type = type;
    }
    void setStatus(const int &status)
    {
        m_status = status;
    }
    void setKonstSym(const std::string &konstSym)
    {
        m_konstSym = konstSym;
    }
    void setVarSymb(const std::string &varSymb)
    {
        m_varSymb = varSymb;
    }
    void setSpecSymb(const std::string &specSymb)
    {
        m_specSymb = specSymb;
    }
    void setPrice(const std::string &price)
    {
        m_price = price;
    }
    void setAccountEvid(const std::string &accountEvid)
    {
        m_accountEvid = accountEvid;
    }
    void setAccountDate(const Database::Date &accountDate)
    {
        m_accountDate = accountDate;
    }

    void setAccountMemo(const std::string &accountMemo);

    void setInvoiceId(const unsigned long long &invoiceId)
    {
        m_invoiceId = invoiceId;
    }

    void setAccountName(const std::string &accountName);

    void setCrTime(const Database::DateTime &crTime)
    {
        m_crTime = crTime;
    }

    void setUuid(const std::string &uuid)
    {
        m_uuid = uuid;
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

    typedef Model::Field::List<ModelBankPayment>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_statementId;
    Field::Field<unsigned long long> m_accountId;
    Field::Field<std::string> m_accountNumber;
    Field::Field<std::string> m_bankCodeId;
    Field::Field<int> m_code;
    Field::Field<int> m_type;
    Field::Field<int> m_status;
    Field::Field<std::string> m_konstSym;
    Field::Field<std::string> m_varSymb;
    Field::Field<std::string> m_specSymb;
    Field::Field<std::string> m_price;
    Field::Field<std::string> m_accountEvid;
    Field::Field<Database::Date> m_accountDate;
    Field::Field<std::string> m_accountMemo;
    Field::Field<unsigned long long> m_invoiceId;
    Field::Field<std::string> m_accountName;
    Field::Field<Database::DateTime> m_crTime;
    Field::Field<std::string> m_uuid;

public:
    static Model::Field::PrimaryKey<ModelBankPayment, unsigned long long> id;
    static Model::Field::Basic<ModelBankPayment, unsigned long long> statementId;
    static Model::Field::Basic<ModelBankPayment, unsigned long long> accountId;
    static Model::Field::Basic<ModelBankPayment, std::string> accountNumber;
    static Model::Field::Basic<ModelBankPayment, std::string> bankCodeId;
    static Model::Field::Basic<ModelBankPayment, int> code;
    static Model::Field::Basic<ModelBankPayment, int> type;
    static Model::Field::Basic<ModelBankPayment, int> status;
    static Model::Field::Basic<ModelBankPayment, std::string> konstSym;
    static Model::Field::Basic<ModelBankPayment, std::string> varSymb;
    static Model::Field::Basic<ModelBankPayment, std::string> specSymb;
    static Model::Field::Basic<ModelBankPayment, std::string> price;
    static Model::Field::Basic<ModelBankPayment, std::string> accountEvid;
    static Model::Field::Basic<ModelBankPayment, Database::Date> accountDate;
    static Model::Field::Basic<ModelBankPayment, std::string> accountMemo;
    static Model::Field::Basic<ModelBankPayment, unsigned long long> invoiceId;
    static Model::Field::Basic<ModelBankPayment, std::string> accountName;
    static Model::Field::Basic<ModelBankPayment, Database::DateTime> crTime;
    static Model::Field::Basic<ModelBankPayment, std::string> uuid;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelBankPayment

#endif // _MODEL_BANK_PAYMENT_H_

