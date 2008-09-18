#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <boost/format.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "settings.h"
#include "keyset_filter.h"
#include "model_filters.h"
#include "db/manager.h"
#include "log/logger.h"

using namespace Database;
using namespace Database::Filters;

Connection* init_connection() {
  std::string conninfo = "host=localhost dbname=fred user=fred";
  Manager db_manager(conninfo);
  Connection *conn = db_manager.getConnection();
  return conn;
}

void exec_and_print(SelectQuery& _q, Union& _f) {
  _f.serialize(_q);

  std::auto_ptr<Connection> conn(init_connection());
  Result result = conn->exec(_q);

  for (Result::Iterator it = result.begin(); it != result.end(); ++it) {
    Row row = *it;
    for (Row::Iterator column = row.begin(); column != row.end(); ++column) {
      std::cout << *column << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "(" << result.size() << " rows)" << std::endl << std::endl;
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
    //    exec_and_print(sq, uf);
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
    //    exec_and_print(sq, uf);
    //    exec_and_print(sq, uf);
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
    //    exec_and_print(sq, uf);
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
//    exec_and_print(sq, uf);
//      
//    return 1;
//    
//    Mail *m = new MailImpl();
//    //m->addId().setValue(Database::ID(28));
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
//    exec_and_print(sq, uf);
//    
//    return 1;

//    EppAction *epp_filter = new EppActionImpl();
//    epp_filter->addEppCodeResponse().setValue(2504);
//
//    uf.addFilter(epp_filter);
//    sq1 = new SelectQuery();
//    sq1->addSelect("*", epp_filter->joinActionTable());
//    uf.addQuery(sq1);
//
//    exec_and_print(sq, uf);   
//    
//    return 1;

    Settings sett;
    sett.set("filter.history", "on");

    Domain *df_test = new DomainHistoryImpl();
    df_test->addHandle().setValue("d*");
    df_test->addZoneId().setValue(Database::ID(3));
    df_test->addRegistrant().addHandle().setValue("CID:TOM");
    df_test->addObjectState().addStateId().setValue(Database::ID(15));

    uf.addFilter(df_test);
    uf.settings(&sett);
    sq1 = new SelectQuery();
    sq1->addSelect(new Column("historyid", df_test->joinObjectTable(), "DISTINCT"));
    sq1->addSelect("id roid name", df_test->joinObjectRegistryTable());
    uf.addQuery(sq1);

    exec_and_print(sq, uf);



    return 1;
#if 0
    // KeySet *k = new KeySetImpl();
    // k->addTechContact().addCity().setValue("Jihlava");
    // k->addId().setValue(DBase::ID(18));
    // k->addDomain().addId().setValue(DBase::ID(16));
    // uf.addFilter(k);
    // sq1 = new SelectQuery();
    // sq1->addSelect("*", k->joinKeySetTable());
    // uf.addQuery(sq1);
    // exec_and_print__(sq, uf);

    return 1;
    
    Invoice *i = new InvoiceImpl();
    //i->addFile().addName().setValue("soubor.pdf");
    i->addRegistrar().addHandle().setValue("REG-FRED_A");
   
    uf.addFilter(i);
    sq1 = new SelectQuery();
    sq1->addSelect("*", i->joinInvoiceTable());
    uf.addQuery(sq1);

    exec_and_print(sq, uf);
   
    return 1;    

    Database::InsertQuery iq("public_request");
    iq.add("state", Database::Value(10));
    iq.add("name", Database::Value("blabla"));
    iq.make();
    std::cout << iq.str() << std::endl;
    return 1;


    Database::SelectQuery sq1__("*", "domain");
    Database::SelectQuery sq2__("discloseemail", "contact");
    Connection *c__ = init_connection();
    Result r1__ = c__->exec(sq1__);
    Result r2__ = c__->exec(sq2__);

    std::cout << "result 1 has " << r1__.size() << " rows" << std::endl;
    std::cout << "result 2 has " << r2__.size() << " rows" << std::endl;

    
    return 1;
    
    Database::Filters::PublicRequest *r = new Database::Filters::PublicRequestImpl();
    r->addCreateTime().setValue(DateTimeInterval(LAST_WEEK, -1));
    Object &ro = r->addObject();
    ro.addHandle().setValue("CID:MAN4");
//    EppAction &re = r->addEppAction();
//    re.addSvTRID().setValue("ccReg-0000000058");
   
    uf.addFilter(r);
    sq1 = new SelectQuery();
    sq1->addSelect("*", r->joinRequestTable());
    uf.addQuery(sq1);

    exec_and_print(sq, uf);

    return 1;


    Domain *d1 = new DomainHistoryImpl();
    d1->addObjectState().addStateId().setValue(Database::ID(14));
    //d1->addExpirationDate(DateInterval(PAST_MONTH, 1));
    
    Contact &d1c = d1->addRegistrant();
    d1c.addName().setValue("Ondřej*");
    // Filters::Value<std::string>& v2 = d1c->addCity(Database::Null<std::string>("Lysa nad Labem"));
    // v2.setValue("Praha 2");
    
    // NSSet *d1n = d1->addNSSet();
    // Filters::Value<std::string>& v3 = d1n->addHostFQDN("");
    // v3.setValue("ns3.domain.cz");
    
    uf.addFilter(d1);
    
    sq1 = new SelectQuery();
    sq1->addSelect("id name", d1->joinObjectRegistryTable());
    uf.addQuery(sq1);
    exec_and_print(sq, uf);
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
    exec_and_print(sq, uf2);
    
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
    exec_and_print(sq, uf);

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

#endif // #if 0
  }
  catch (Database::Exception& e) {
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

