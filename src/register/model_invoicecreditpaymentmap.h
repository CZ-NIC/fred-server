#ifndef _MODEL_INVOICECREDITPAYMENTMAP_H_
#define _MODEL_INVOICECREDITPAYMENTMAP_H_

#include "model_invoice.h"

class ModelInvoiceCreditPaymentMap:
    public Model::Base {
public:
    ModelInvoiceCreditPaymentMap()
    { }
    virtual ~ModelInvoiceCreditPaymentMap()
    { }
    const unsigned long long &getInvoiceId() const
    {
        return m_invoiceId.get();
    }
    void setInvoiceId(const unsigned long long &invoiceId)
    {
        m_invoiceId = invoiceId;
    }
    const unsigned long long &getAdvanceInvoiceId() const
    {
        return m_advanceInvoiceId.get();
    }
    void setAdvanceInvoiceId(const unsigned long long &advanceInvoiceId)
    {
        m_advanceInvoiceId = advanceInvoiceId;
    }
    ModelInvoice *getAdvanceInvoice()
    {
        return advanceInvoice.getRelated(this);
    }
    void setAdvanceInvoice(ModelInvoice *advInv)
    {
        advanceInvoice.setRelated(this, advInv);
    }
    const Database::Money &getCredit() const
    {
        return m_credit.get();
    }
    void setCredit(const Database::Money &credit)
    {
        m_credit = credit;
    }
    const Database::Money &getBalance() const
    {
        return m_balance.get();
    }
    void setBalance(const Database::Money &balance)
    {
        m_balance = balance;
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
    Field::Field<unsigned long long>      m_invoiceId;
    Field::Field<unsigned long long>      m_advanceInvoiceId;
    Field::Field<Database::Money>   m_credit;
    Field::Field<Database::Money>   m_balance;

    Field::Lazy::Field<ModelInvoice *>  m_advanceInvoice;

    typedef Model::Field::List<ModelInvoiceCreditPaymentMap> field_list;

public:
    static Model::Field::PrimaryKey<ModelInvoiceCreditPaymentMap, unsigned long long> invoiceId;
    static Model::Field::ForeignKey<ModelInvoiceCreditPaymentMap, unsigned long long, ModelInvoice>   advanceInvoiceId;
    static Model::Field::Basic<ModelInvoiceCreditPaymentMap, Database::Money>   credit;
    static Model::Field::Basic<ModelInvoiceCreditPaymentMap, Database::Money>   balance;

    static Model::Field::Related::OneToOne<ModelInvoiceCreditPaymentMap, unsigned long long, ModelInvoice>    advanceInvoice;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;

}; // class ModelInvoiceCreditPaymentMap

#endif // _MODEL_INVOICECREDITPAYMENTMAP_H_
