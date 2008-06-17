#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <boost/format.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "model_filters.h"
#include "db/dbs.h"
#include "log/logger.h"

using namespace DBase;
using namespace DBase::Filters;

Connection* init_connection() {
  std::string conninfo = "host=localhost dbname=fred user=fred";
  std::auto_ptr<Manager> m(new PSQLManager(conninfo));
  Connection *conn = m->getConnection();
  return conn;
}

void print(Result *r) {
  std::auto_ptr<ResultIterator> it(r->getIterator());
  it->first();

  while (!it->isDone()) {
    while (it->isNextValue()) {
      std::cout << it->getNextValue() << " ";
    }
    it->next();
    std::cout << std::endl;
  }
  std::cout << "(" << r->getNumRows() << " rows)" << std::endl << std::endl;
}

void exec_and_print(SelectQuery& _q) {
  std::auto_ptr<Connection> conn(init_connection());
  std::auto_ptr<Result> r(conn->exec(_q));
  print(r.get());
}

void exec_and_print(SelectQuery& _q, Union& _f) {
  _f.serialize(_q);
  //Transaction t1 = conn->getTransaction();

  Connection *conn = init_connection();
  std::auto_ptr<Result> r(conn->exec(_q));

  //std::auto_ptr<Result__> r1__(conn->exec__(q));
  //Result__::Iterator it__ = r1__->begin();
  //std::cout << it__.getValue() << std::endl;
  //t1.commit();

  //	ID _id = it->getNextValue();
  //	std::string _roid = it->getNextValue();
  //	int _type = it->getNextValue();
  //	std::string _name = it->getNextValue();
  //	int _crid = it->getNextValue();
  //	DateTime _crdate = it->getNextValue();
  //	Null<DateTime> _erdate = it->getNextValue();
  //	int _crhistoryid = it->getNextValue();
  //	int _historyid = it->getNextValue();
  //	std::string _org = it->getNextValue();
  //	std::string _city = it->getNextValue();

  print(r.get());
  delete conn;
}

void exec_and_print__(SelectQuery& _q, Union& _f) {
  _f.serialize(_q);
  //_q.make();

  std::auto_ptr<Connection> conn(init_connection());
  //std::auto_ptr<Result> r(conn->exec(_q));  
  std::auto_ptr<Result__> r(conn->exec__(_q));

  Result__::Iterator it = r->begin();
  while (!it.isDone()) {
    Row row = it.getValue();
    for (Row::Iterator col_it = row.begin(); col_it != row.end(); ++col_it) {
      std::cout << *col_it << " ";
    }
    it.next();
    std::cout << std::endl;
  }
  std::cout << "(" << r->getNumRows() << " rows)" << std::endl << std::endl;

}

void fill_and_print_formater(const std::vector<std::string>& _store,
    boost::format& _frmt) {
  _frmt.clear();
  for (unsigned i = 0; i < _store.size(); ++i) {
    _frmt.bind_arg(i + 1, _store[i]);
  }
}

int main(int argc, char *argv[]) {
  try {
    Logging::Manager::instance_ref().get("db").addHandler(Logging::Log::LT_CONSOLE);
    Logging::Manager::instance_ref().get("db").setLevel(Logging::Log::LL_DEBUG);
    Logging::Manager::instance_ref().get("tracer").addHandler(Logging::Log::LT_CONSOLE);
    Logging::Manager::instance_ref().get("tracer").setLevel(Logging::Log::LL_TRACE);

    SelectQuery sq;
    Union uf;
    //		
    //		EppAction *ef = new EppActionImpl();
    //		Object *of = ef->addObject();
    //		ef->addSession();
    //		//es->addLogin(DateInterval(Date(2007,9,1), Date(2008,1,31)));
    //		Registrar *rf = ef->addRegistrar();
    //		rf->addOrganization(Null<std::string>()).setNOT(1);
    //		rf->addCity(std::string("Praha*"));
    //
    //		sq.addSelect("*", of->joinObjectRegistryTable());
    //		sq.addSelect("organization", rf->joinRegistrarTable());
    //		sq.addSelect("city", rf->joinRegistrarTable());
    //
    //		uf.add(ef);
    //
    //		exec_and_print(sq, uf);
    //
    //		std::vector<std::string> &prep = sq.where_prepared_values();
    //		prep[0] = "Praha 8";
    //
    //		exec_and_print(sq);
    //		sq.clear();


    SelectQuery *sq1; //, *sq2;
    // sq1 = new SelectQuery();
    // sq2 = new SelectQuery();

    //    FilterFilter *ff = new FilterFilterImpl();
    //    sq1->addSelect("id type name userid groupid", ff->joinFilterTable());
    //    uf.clear();
    //    uf.addFilter(ff);
    //    uf.addQuery(sq1);
    //    exec_and_print__(sq, uf);
    //    sq.clear();
    //    uf.clear();
    //
    //    return 1;
    //
    //    sq1 = new SelectQuery();
    //
    //    Registrar *r1 = new RegistrarImpl();
    //    r1->addHandle("*");
    //    Registrar *r2 = new RegistrarImpl();
    //    r2->addHandle("*");
    //    sq1->addSelect("id name handle url", r1->joinRegistrarTable());
    //    sq2->addSelect("id name handle url", r2->joinRegistrarTable());
    //    uf.clear();
    //    uf.addFilter(r1);
    //    uf.addQuery(sq1);
    //    uf.addFilter(r2);
    //    uf.addQuery(sq2);
    //    exec_and_print__(sq, uf);
    //    exec_and_print__(sq, uf);
    //    sq.clear();
    //    uf.clear();

    //    Domain *f = new DomainImpl();
    //    Contact *f2 = f->addTempContact();
    //    f2->addEmail(std::string("*@mymail.cz"));
    //    f2->addCity(std::string("Praha"));
    //    f->addUpdateTime(DateTimeInterval(LAST_MONTH, -4, Date(2007,9,25), DateTime("2007-09-26 23:59:57")));
    //    //f->addUpdateTime(DateTimeInterval(PAST_MONTH, -8));
    //
    //    //NSSet *nf = f->addNSSet();
    //    //nf->addHostIP("217.31.206.130");
    //
    //    //f->addName("d*");
    //    //f.addEraseDate(Null<DateTimeInterval>());
    //    //f.addID(78);
    //    f->addExpirationDate(DateInterval(LAST_MONTH,1));
    //
    //    sq1 = new SelectQuery();
    //    sq1->addSelect("roid", f->joinObjectRegistryTable());
    //    sq1->addSelect("exdate",f->joinDomainTable());
    //    //sq1->addSelect("name city email", f2->joinContactTable());
    //
    //    uf.clear();
    //    uf.addFilter(f);
    //    uf.addQuery(sq1);
    //    exec_and_print__(sq, uf);
    //    sq.clear();
    //    uf.clear();
    //
    //    return 1;
    
//    File *f = new FileImpl();
//    f->addMimeType().setValue("*/pdf");
//    
//    uf.addFilter(f);
//    sq1 = new SelectQuery();
//    sq1->addSelect("id", f->joinFileTable());
//    uf.addQuery(sq1);
//      
//    exec_and_print__(sq, uf);
//      
//    return 1;
//    
//    Mail *m = new MailImpl();
//    //m->addId().setValue(DBase::ID(28));
//    //m->addMessage().setValue("Contact data change");
//    //m->addHandle().setValue("CID:JIRI");
//    //m->addType().setValue(1);
//    m->addAttachment().addName().setValue("150800001.pdf");
//    
//    uf.addFilter(m);
//    sq1 = new SelectQuery();
//    sq1->addSelect("id", m->joinMailTable());
//    uf.addQuery(sq1);
//    
//    exec_and_print__(sq, uf);
//    
//    return 1;
    
    
    Invoice *i = new InvoiceImpl();
    //i->addType().setValue(0);
    i->addNumber().setValue("1*");
    //i->addFile().addName().setValue("150800001.pdf");
   
    uf.addFilter(i);
    sq1 = new SelectQuery();
    sq1->addSelect("*", i->joinInvoiceTable());
    uf.addQuery(sq1);

    exec_and_print__(sq, uf);
   
    return 1;    

    DBase::InsertQuery iq("public_request");
    iq.add("state", DBase::Value(10));
    iq.add("name", DBase::Value("blabla"));
    iq.make();
    std::cout << iq.str() << std::endl;
    return 1;


    DBase::SelectQuery sq1__("*", "domain");
    DBase::SelectQuery sq2__("discloseemail", "contact");
    Connection *c__ = init_connection();
    Result *r1__ = c__->exec(sq1__);
    Result *r2__ = c__->exec(sq2__);
    
    std::auto_ptr<ResultIterator> it(r2__->getIterator());
    it->first();

    while (!it->isDone()) {
      while (it->isNextValue()) {
        bool flag =  it->getNextValue();
        std::cout << (flag ? "true" : "false");
      }
      it->next();
      std::cout << std::endl;
    }
    std::cout << "(" << r2__->getNumRows() << " rows)" << std::endl << std::endl;
    
    
    std::auto_ptr<ResultIterator> it1(r1__->getIterator());
    it1->first();

    while (!it1->isDone()) {
      while (it1->isNextValue()) {
        std::cout << it1->getNextValue() << " ";
      }
      it1->next();
      std::cout << std::endl;
    }
    std::cout << "(" << r1__->getNumRows() << " rows)" << std::endl << std::endl;
              

    return 1;
    
    DBase::Filters::PublicRequest *r = new DBase::Filters::PublicRequestImpl();
    r->addCreateTime().setValue(DateTimeInterval(LAST_WEEK, -1));
    Object &ro = r->addObject();
    ro.addHandle().setValue("CID:MAN4");
//    EppAction &re = r->addEppAction();
//    re.addSvTRID().setValue("ccReg-0000000058");
   
    uf.addFilter(r);
    sq1 = new SelectQuery();
    sq1->addSelect("*", r->joinRequestTable());
    uf.addQuery(sq1);

    exec_and_print__(sq, uf);

    return 1;


    Domain *d1 = new DomainHistoryImpl();
    ObjectState &s = d1->addState();
    s.addId().setValue(DBase::Null<int>(14));
    //d1->addExpirationDate(DateInterval(PAST_MONTH, 1));
    
    Contact &d1c = d1->addRegistrant();
    d1c.addName().setValue("Ondřej*");
    // Filters::Value<std::string>& v2 = d1c->addCity(DBase::Null<std::string>("Lysa nad Labem"));
    // v2.setValue("Praha 2");
    
    // NSSet *d1n = d1->addNSSet();
    // Filters::Value<std::string>& v3 = d1n->addHostFQDN("");
    // v3.setValue("ns3.domain.cz");
    
    uf.addFilter(d1);
    
    sq1 = new SelectQuery();
    sq1->addSelect("id name", d1->joinObjectRegistryTable());
    uf.addQuery(sq1);
    exec_and_print__(sq, uf);
    return 1;
   
    std::ofstream ofsd;
    ofsd.open("test-d-filter.xml", std::ios_base::trunc);
    assert(ofsd.good());
    boost::archive::xml_oarchive save_d(ofsd);
    save_d << BOOST_SERIALIZATION_NVP(uf);
    ofsd.close();
   
 
    sq.clear();
    uf.clear();
    
    Union uf2;

    std::ifstream ifsd("test-d-filter.xml");
    assert(ifsd.good());
    boost::archive::xml_iarchive load_d(ifsd);
    load_d >> BOOST_SERIALIZATION_NVP(uf2);
    ifsd.close();

    Union::iterator dfit = uf2.begin();
    Domain *d2 = dynamic_cast<DomainHistoryImpl *>(*dfit);
    
    sq1 = new SelectQuery();
    sq1->addSelect("id crid name erdate", d2->joinObjectRegistryTable());
    sq1->addSelect("clid", d2->joinObjectTable());
    sq1->addSelect("id", d2->joinDomainTable());

    uf2.addQuery(sq1);
    exec_and_print__(sq, uf2);
    
    return 1;

    /*************************************************************************/

    Contact *c1 = new ContactImpl();
    c1->addName().setValue("Fred");

    uf.clear();
    uf.addFilter(c1);

    boost::archive::xml_oarchive save_c(std::cout);
    save_c << BOOST_SERIALIZATION_NVP(uf);

    sq.clear();
    uf.clear();

    std::ifstream ifsc("test-c-filter.xml");
    assert(ifsc.good());

    boost::archive::xml_iarchive load_c(ifsc);
    load_c >> BOOST_SERIALIZATION_NVP(uf);

    Union::iterator cfit = uf.begin();
    Contact *c2 = dynamic_cast<ContactImpl *>(*cfit);

    sq1 = new SelectQuery();
    sq1->addSelect("id crid name erdate", c2->joinObjectRegistryTable());

    uf.addQuery(sq1);
    exec_and_print__(sq, uf);

    //
    //		EppAction eaf;
    //		eaf.addSession()->addRegistrar()->addHandle("REG-CZNIC");
    //		//eaf.addStart(DateTimeInterval(PAST_MONTH, -8));
    //		eaf.addObject()->addName("nic.cz");
    //
    //		sq.addSelect("clienttrid", eaf.joinActionTable());
    //		exec_and_print(sq, eaf);
    //		sq.clear();
    //
    //		Contact cf2;
    //		cf2.addName(std::string("Š*"));
    //
    //		sq.addSelect("name", cf2.joinContactTable());
    //		exec_and_print(sq, cf2);
    //		sq.clear();

  }
  catch (Exception& e) {
    std::cout << e.what() << std::endl;
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unknown exception catched" << std::endl;
  }

  return 1;
}

