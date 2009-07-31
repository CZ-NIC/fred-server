#ifndef _MODEL_BANKSTATEMENTITEM_H_
#define _MODEL_BANKSTATEMENTITEM_H_

#include "db_settings.h"
#include "model.h"
#include "model_bankstatementhead.h"
#include "model_enumbankcode.h"
#include "model_invoice.h"


class ModelBankStatementItem:
    public Model::Base {
public:
    ModelBankStatementItem()
    { }
    virtual ~ModelBankStatementItem()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getStatementId() const
    {
        return m_statementId.get();
    }
    ModelBankStatementHead *getStatement()
    {
        return statement.getRelated(this);
    }
    const std::string &getAccountNumber() const
    {
        return m_accountNumber.get();
    }
    const std::string &getBankCodeId() const
    {
        return m_bankCodeId.get();
    }
    ModelEnumBankCode *getBankCode()
    {
        return bankCode.getRelated(this);
    }
    const int &getCode() const
    {
        return m_code.get();
    }
    const int &getType() const
    {
        return m_type.get();
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
    const Database::Money &getPrice() const
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
    const unsigned long long &getInvoiceId() const
    {
        return m_invoiceId.get();
    }
    ModelInvoice *getInvoice()
    {
        return invoice.getRelated(this);
    }
    const std::string &getAccountName() const
    {
        return m_accountName.get();
    }
    const Database::DateTime &getCrTime() const
    {
        return m_crTime.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setStatementId(const unsigned long long &statementId)
    {
        m_statementId = statementId;
    }
    void setStatement(ModelBankStatementHead *foreign_value)
    {
        statement.setRelated(this, foreign_value);
    }
    void setAccountNumber(const std::string &accountNumber)
    {
        m_accountNumber = accountNumber;
    }
    void setBankCodeId(const std::string &bankCodeId)
    {
        m_bankCodeId = bankCodeId;
    }
    void setBankCode(ModelEnumBankCode *foreign_value)
    {
        bankCode.setRelated(this, foreign_value);
    }
    void setCode(const int &code)
    {
        m_code = code;
    }
    void setType(const int &type)
    {
        m_type = type;
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
    void setPrice(const Database::Money &price)
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
    void setAccountMemo(const std::string &accountMemo)
    {
        m_accountMemo = accountMemo;
    }
    void setInvoiceId(const unsigned long long &invoiceId)
    {
        m_invoiceId = invoiceId;
    }
    void setInvoice(ModelInvoice *foreign_value)
    {
        invoice.setRelated(this, foreign_value);
    }
    void setAccountName(const std::string &accountName)
    {
        m_accountName = accountName;
    }
    void setCrTime(const Database::DateTime &crTime)
    {
        m_crTime = crTime;
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

    typedef Model::Field::List<ModelBankStatementItem>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_statementId;
    Field::Field<std::string> m_accountNumber;
    Field::Field<std::string> m_bankCodeId;
    Field::Field<int> m_code;
    Field::Field<int> m_type;
    Field::Field<std::string> m_konstSym;
    Field::Field<std::string> m_varSymb;
    Field::Field<std::string> m_specSymb;
    Field::Field<Database::Money> m_price;
    Field::Field<std::string> m_accountEvid;
    Field::Field<Database::Date> m_accountDate;
    Field::Field<std::string> m_accountMemo;
    Field::Field<unsigned long long> m_invoiceId;
    Field::Field<std::string> m_accountName;
    Field::Field<Database::DateTime> m_crTime;

    Field::Lazy::Field<ModelBankStatementHead *> m_statement;
    Field::Lazy::Field<ModelEnumBankCode *> m_bankCode;
    Field::Lazy::Field<ModelInvoice *> m_invoice;

public:
    static Model::Field::PrimaryKey<ModelBankStatementItem, unsigned long long> id;
    static Model::Field::ForeignKey<ModelBankStatementItem, unsigned long long, ModelBankStatementHead> statementId;
    static Model::Field::Basic<ModelBankStatementItem, std::string> accountNumber;
    static Model::Field::ForeignKey<ModelBankStatementItem, std::string, ModelEnumBankCode> bankCodeId;
    static Model::Field::Basic<ModelBankStatementItem, int> code;
    static Model::Field::Basic<ModelBankStatementItem, int> type;
    static Model::Field::Basic<ModelBankStatementItem, std::string> konstSym;
    static Model::Field::Basic<ModelBankStatementItem, std::string> varSymb;
    static Model::Field::Basic<ModelBankStatementItem, std::string> specSymb;
    static Model::Field::Basic<ModelBankStatementItem, Database::Money> price;
    static Model::Field::Basic<ModelBankStatementItem, std::string> accountEvid;
    static Model::Field::Basic<ModelBankStatementItem, Database::Date> accountDate;
    static Model::Field::Basic<ModelBankStatementItem, std::string> accountMemo;
    static Model::Field::ForeignKey<ModelBankStatementItem, unsigned long long, ModelInvoice> invoiceId;
    static Model::Field::Basic<ModelBankStatementItem, std::string> accountName;
    static Model::Field::Basic<ModelBankStatementItem, Database::DateTime> crTime;

    static Model::Field::Related::OneToOne<ModelBankStatementItem, unsigned long long, ModelBankStatementHead> statement;
    static Model::Field::Related::OneToOne<ModelBankStatementItem, std::string, ModelEnumBankCode> bankCode;
    static Model::Field::Related::OneToOne<ModelBankStatementItem, unsigned long long, ModelInvoice> invoice;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelBankStatementItem

#endif // _MODEL_BANKSTATEMENTITEM_H_

