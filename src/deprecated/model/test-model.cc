/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/model/model_filters.hh"
#include "util/log/add_log_device.hh"
#include "util/log/logger.hh"
#include "libfred/opcontext.hh"

#include "src/deprecated/model/registrar_filter.hh"

#include <boost/format.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>

void init_connection()
{
    try
    {
        const std::string conn_info = "host=localhost dbname=fred user=fred port=5432";
        Database::emplace_default_manager<Database::StandaloneManager>(conn_info);
    }
    catch (const Database::Exception& e)
    {
        std::cout << "Error while connecting to the database " << e.what() << std::endl;
    }
}

void exec_and_print(Database::SelectQuery& _q, Database::Filters::Union& _f)
{
    _f.serialize(_q);

    const std::string info_query_str = str(boost::format("%1% LIMIT %2%") % _q.str() % 1000);

    std::cout << "\n" << "exec_and_print: " << info_query_str << "\n" << std::endl;

    init_connection();
    LibFred::OperationContextCreator ctx;
    const Database::Result result = ctx.get_conn().exec(info_query_str);

    for (Database::Result::Iterator it = result.begin(); it != result.end(); ++it)
    {
        const Database::Row row = *it;
        for (Database::Row::Iterator column = row.begin(); column != row.end(); ++column)
        {
            std::cout << *column << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "(" << result.size() << " rows)" << std::endl << std::endl;
}

int main()
{
    try
    {
        Logging::add_console_device(LOGGER, Logging::Log::Severity::debug);

        //Zone Test
        bool at_least_one = false;
        Database::Filters::Zone* const zoneFilter = new Database::Filters::ZoneSoaImpl(true);
        Database::Filters::Union* const uf = new Database::Filters::Union();
        uf->addFilter(zoneFilter);

        Database::SelectQuery info_query;
        std::unique_ptr<Database::Filters::Iterator> fit(uf->createIterator());
        for (fit->first(); !fit->isDone(); fit->next())
        {
            Database::Filters::Zone* const zf =
            dynamic_cast<Database::Filters::Zone*>(fit->get());
            if (zf == nullptr)
            {
                continue;
            }
            Database::SelectQuery* const tmp = new Database::SelectQuery();
            tmp->select() << "z.id, z.fqdn, z.ex_period_min, z.ex_period_max"
            ", z.val_period, z.dots_max, z.enum_zone"
            ", zs.ttl, zs.hostmaster, zs.serial, zs.refresh, zs.update_retr"
            ", zs.expiry, zs.minimum, zs.ns_fqdn";
            tmp->from() << "zone z JOIN zone_soa zs ON z.id = zs.zone";
            tmp->order_by() << "z.id";

            uf->addQuery(tmp);
            at_least_one = true;
        }

        if (!at_least_one)
        {
            LOGGER.error("wrong filter passed for reload ZoneList!");
            return EXIT_SUCCESS;
        }

        /* manually add query part to order result by id
        * need for findIDSequence() */
        //uf.serialize(info_query);
        //std::string info_query_str = str(boost::format("%1% ORDER BY id LIMIT %2%") % info_query.str() % m_limit);

        exec_and_print(info_query, *uf);
        return EXIT_SUCCESS;

      //Zone Test End
    /*
    //Registrar Test
    SelectQuery sq;
    Union uf;

    Registrar *r = new RegistrarImpl(true);
    Zone &z = r->addActiveZone();
    z.addFqdn().setValue("*.arpa");


    SelectQuery *sq1 = new SelectQuery;
    //r->addHandle() = "*";


    sq1->addSelect("id name handle url", r->joinRegistrarTable());

    uf.clear();
    uf.addFilter(r);
    //uf.addFilter(z);
    uf.addQuery(sq1);

    //sq.addSelect("id name handle url",r->joinRegistrarTable());

    exec_and_print(sq, uf);
    return 0;

    //Registrar Test End

    */

    //SelectQuery sq;
    //Union uf;
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


    //SelectQuery *sq1; //, *sq2;
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


/*
    Domain *df_test = new DomainHistoryImpl();
    df_test->addHandle().setValue("d*");
    df_test->addZoneId().setValue(Database::ID(3));
    df_test->addRegistrant().addHandle().setValue("CID:TOM");

    uf.addFilter(df_test);
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
*/
/*Req1
// logging
    RequestImpl *le = new RequestImpl(true);

    // DateTimeInterval di(Date(2008, 9, 17), DateTime("2008-09-18 12:41:13"));
    // DateTimeInterval di(DateTime("2008-10-23 11:00:30"), DateTime("2008-10-25 15:41:13"));


    DateTimeInterval di(DateTime("2008-10-23 13:16:30"), DateTime("2009-11-23 13:20:13"));
    le->addTimeBegin().setValue(di);

//  le->addComponent().setValue(LC_UNIX_WHOIS);

    RequestPropertyValue &pv = le->addRequestPropertyValue();

    pv.addName().setValue("registrarId");
    pv.addValue().setValue("REG-FRED_B");

    uf.addFilter(le);
    sq1 = new SelectQuery();
*/
/*
    pv.addRequestProperty().addName().setValue("search axis");
    pv.addValue().setValue("registrant");
*/



/*
    le->addJoin (new Join( Column("id", le->joinTable("request")),
			SQL_OP_EQ,
			Column("request_id", le->joinTable("property_value"))
			));
    le->addJoin (new Join( Column("property_id", le->joinTable("property_value")),
			SQL_OP_EQ,
			Column("id", le->joinTable("property"))
			));
*/
/*Req2
    sq1->addSelect("name", pv.joinRequestPropertyTable());
    sq1->addSelect("value", pv.joinRequestPropertyValueTable());

    sq1->addSelect("time_begin source_ip is_monitoring service_id", le->joinRequestTable());

    uf.addQuery(sq1);

    exec_and_print(sq, uf);

    return 0;
*/

// ------------- contact

/*
    Domain *d1 = new DomainHistoryImpl();
    // ObjectState &s = d1->addState();
    // s.addId().setValue(Database::Null<int>(14));

    DateTimeInterval di(Date(2008, 8, 01), Date(2008, 9, 13));

    d1->addCreateTime().setValue(di);
    // d1->addExpirationDate(DateInterval(PAST_MONTH, 1));
    // d1->addExpirationDate() << di;

    Contact &d1c = d1->addRegistrant();
    //d1c.addName().setValue("Ondřej*");
    d1c.addName().setValue("Jan*");
    // Filters::Value<std::string>& v2 = d1c->addCity(Database::Null<std::string>("Lysa nad Labem"));
    // v2.setValue("Praha 2");

    // NSSet *d1n = d1->addNSSet();
    // Filters::Value<std::string>& v3 = d1n->addHostFQDN("");
    // v3.setValue("ns3.domain.cz");

    uf.addFilter(d1);

    sq1 = new SelectQuery();
    // sq1->addSelect("*", d1->joinObjectRegistryTable());

    uf.addQuery(sq1);

    std::cout << "-------------- exec_and_print: " << std::endl;
    exec_and_print(sq, uf);


    return 1;
*/
/*ofstream

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
*/
    /*************************************************************************/
/*Contact
    Contact *c1 = new ContactImpl();
    c1->addName().setValue("Fred");

    uf.clear();
    uf.addFilter(c1);

    //boost::archive::xml_oarchive save_c(std::cout);
    //save_c << BOOST_SERIALIZATION_NVP(uf);

    sq.clear();

    // std::ifstream ifsc("test-c-filter.xml");
    // assert(ifsc.good());

    sq1 = new SelectQuery();
    sq1->addSelect("id crid name erdate", c1->joinObjectRegistryTable());

    uf.addQuery(sq1);

    exec_and_print(sq, uf);
*/
/*
    boost::archive::xml_iarchive load_c(ifsc);
    load_c >> BOOST_SERIALIZATION_NVP(uf);

    Union::iterator cfit = uf.begin();
    Contact *c2 = dynamic_cast<ContactImpl *>(*cfit);

    sq1 = new SelectQuery();
    sq1->addSelect("id crid name erdate", c2->joinObjectRegistryTable());

    uf.addQuery(sq1);
    exec_and_print(sq, uf);
*/

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

    // -------------------
    /*
    LogReader *lr = new LogReaderImpl();

    lr->addDateInterval(DateTimeInterval(LAST_MONTH, -1));
    lr->addComponent(Value<Comp>(MOD_WHOIS));
    lr->addIpAddr("127.0.*");
    lr->addProperties().addPair("inverseKey", "nsset");

    uf.addFilter(lr);

    sq1 = new SelectQuery();
    sq1->addSelect("sourceIP component Timestamp name value", lr->joinProperties());

    uf.addQuery(sq1);
    exec_and_print(sq, uf);
    */

//#endif // #if 0
    }
    catch (const Database::Exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown exception catched" << std::endl;
    }

    return EXIT_FAILURE;
}
