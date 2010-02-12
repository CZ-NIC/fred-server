/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <sstream>

#include "dbsql.h"
#include "util.h"
#include "log.h"
#include "log/logger.h"
#include "corba/epp/action.h"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

void
ParsedAction::add(unsigned id, const std::string& value)
{
  elements[id] = value;
}

bool 
ParsedAction::executeSQL(Register::TID actionid, DB* db)
{
  std::map<unsigned, std::string>::const_iterator i;
  for (i=elements.begin();i!=elements.end();i++) {
    std::stringstream sql;
    sql << "INSERT INTO action_elements (actionid,elementid,value) VALUES ("
        << actionid << "," << i->first << ", LOWER('" << db->Escape2(i->second) << "'))";
    if (!db->ExecSQL(sql.str().c_str()))
        return false;
  }
  return true;
}

// for invoice  type 
#define INVOICE_FA  1 // normal invoice
#define INVOICE_ZAL 0 // advance invoice 

// constructor 
DB::DB()
{
  // set mem buffers 
  svrTRID = NULL;
  memHandle=NULL;
  actionID = 0;
  enum_action=0;
  loginID = 0;
}

// free memory buffers
DB::~DB()
{
  if (svrTRID) {
    LOG( NOTICE_LOG , "delete svrTRID");
    delete[] svrTRID;
  }

  if (memHandle) {
    LOG( NOTICE_LOG , "delete memHandle");
    delete[] memHandle;
  }

}

long DB::GetRegistrarCredit(
  int regID, int zoneID)
{
  long price=0;
  char sqlString[128];

  sprintf(sqlString,
      "SELECT sum( credit) FROM invoice  WHERE  registrarid=%d and zone=%d; ",
      regID, zoneID);

  if (ExecSelect(sqlString) ) {

    if (GetSelectRows() == 1) {
      price = (long) rint( 100.0 * atof(GetFieldValue( 0, 0) ) );
    }

    FreeSelect();
  }

  return price;
}

// test for login
bool DB::TestRegistrarACL(
  int regID, const char * pass, const char * cert)
{
  char sqlString[512];
  bool ret =false;

  sprintf(
      sqlString,
      "SELECT  registrarid FROM registraracl WHERE registrarid=%d and cert=\'%s\' and password=\'%s\'; ",
      regID, cert, pass);
  if (ExecSelect(sqlString) ) {

    if (GetSelectRows() > 0)
      ret = true;

    FreeSelect();
  }

  return ret;
}
// save EPP message about transfered object
bool DB::SaveEPPTransferMessage(
  int oldregID, int regID, int objectID, int type)
{
  char xmlString[1024];
  char regHandle[65];
  char
      schema_nsset[] =
          " xmlns:nsset=\"http://www.nic.cz/xml/epp/nsset-1.2\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/nsset-1.2 nsset-1.2.xsd\" ";
  char
      schema_domain[] =
          " xmlns:domain=\"http://www.nic.cz/xml/epp/domain-1.3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/domain-1.3 domain-1.3.xsd\" ";
  char
      schema_contact[] =
          " xmlns:contact=\"http://www.nic.cz/xml/epp/contact-1.4\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/contact-1.4 contact-1.4.xsd\" ";

  char
      schema_keyset[] = 
      " xmlns:keyset=\"http://www.nic.cz/xml/epp/keyset-1.4\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/keyset-1.4 keyset-1.4.xsd\" ";

  LOG( NOTICE_LOG , "EPPTransferMessage to registrar : %d trasfer objectID %d new registar %d" , oldregID , objectID , regID );

  xmlString[0] = 0; // empty string

  // get registrar handle 
  strncpy(regHandle, GetRegistrarHandle(regID), sizeof(regHandle) - 1);

  switch (type) {
    case 1: // contact
      if (SELECTOBJECTID("CONTACT", "handle", objectID) ) {

        snprintf(
            xmlString,
            sizeof(xmlString),
            "<contact:trnData %s ><contact:id>%s</contact:id><contact:trDate>%s</contact:trDate><contact:clID>%s</contact:clID></contact:trnData>",
            schema_contact, GetFieldValueName("Name", 0),
            GetFieldDateTimeValueName("TrDate", 0) , regHandle);
        FreeSelect();
      }
      break;
    case 2: // nsset
      if (SELECTOBJECTID("NSSET", "handle", objectID)) {
        snprintf(
            xmlString,
            sizeof(xmlString),
            "<nsset:trnData %s > <nsset:id>%s</nsset:id><nsset:trDate>%s</nsset:trDate><nsset:clID>%s</nsset:clID></nsset:trnData>",
            schema_nsset, GetFieldValueName("Name", 0),
            GetFieldDateTimeValueName("TrDate", 0) , regHandle);
        FreeSelect();
      }

      break;
    case 3: // domain
      if (SELECTOBJECTID("DOMAIN", "fqdn", objectID) ) {
        snprintf(
            xmlString,
            sizeof(xmlString),
            "<domain:trnData %s ><domain:name>%s</domain:name><domain:trDate>%s</domain:trDate><domain:clID>%s</domain:clID></domain:trnData>",
            schema_domain, GetFieldValueName("Name", 0) ,
            GetFieldDateTimeValueName("TrDate", 0) , regHandle);
        FreeSelect();
      }
      break;
    case 4: // keyset
      if (SELECTOBJECTID("KEYSET", "handle", objectID)) {
          snprintf(
                  xmlString,
                  sizeof(xmlString),
                  "<keyset:trnData %s > <keyset:id>%s</keyset:id><keyset:trDate>%s</keyset:trDate><keyset:clID>%s</keyset:clID></keyset:trnData> ",
                  schema_keyset, GetFieldValueName("Name", 0),
                  GetFieldDateTimeValueName("TrDate", 0), regHandle);
          FreeSelect();
      }
      break;
    default:
      xmlString[0] = 0; // empty string  
      break;
  }

  LOG(DEBUG_LOG , "EPPTransferMessage xmlsString: %s" , xmlString );
  //  insert message into table message default ExDate + 1 month

  INSERT("message");
  INTO("Clid");
  INTO("crdate");
  INTO("exdate");
  INTO("seen");
  INTO("message");
  VALUE(oldregID); // id of the old registrar 
  VALUENOW();
  VALUEPERIOD( 1); // actual datetime plus one month 
  VALUE( false);

  if (strlen(xmlString) ) {
    VALUE(xmlString);
    return EXEC();
  } else
    return false;

}

// return price per operation depend on the period in months
long DB::GetPrice(
  int operation, int zone, int period)
{
  char sqlString[256];
  long p, price=0; // default price=0
  int per; // price per period


  // specialy for renew operation depend on period
  if (period > 0)
    snprintf(
        sqlString,
        sizeof(sqlString),
        "SELECT price , period FROM price_list WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  and operation=%d and zone=%d;",
        operation, zone);
  // price for operation not depend on the time interval period
  else
    snprintf(
        sqlString,
        sizeof(sqlString),
        "SELECT price  FROM price_list WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  AND  operation=%d and zone=%d;",
        operation, zone);

  if (ExecSelect(sqlString) ) {

    if (GetSelectRows() == 1) {
      // get price from database convert numeric  1.00 to 100
      p = get_price(GetFieldValue( 0, 0) );

      if (period > 0) {
        per = atoi(GetFieldValue( 0, 1) );
        // count price 
        price = period * p / per;
      } else
        price = p;

      LOG( NOTICE_LOG , "GetPrice operation %d zone %d period %d   -> price %ld (units) " , operation , zone , period , price);
    } else
      price=-1;

    FreeSelect();
    return price;
  } else
    return -2; // ERROR
}

// save credit to invoice
bool DB::SaveInvoiceCredit(
  int regID, int objectID, int operation, int zone, int period,
  const char *ExDate, long price, long price2, int invoiceID, int invoiceID2)
{
  int id;

  LOG( DEBUG_LOG , "SaveInvoiceCredit: uctovani creditu objectID %d ExDate [%s] regID %d" , objectID , ExDate , regID );

  id = GetSequenceID("invoice_object_registry");

  // insert record about billing objects 
  INSERT("invoice_object_registry");
  INTO("id");
  INTO("objectid");
  INTO("registrarid");
  INTO("operation");
  INTO("zone");
  INTO("period");
  INTOVAL("ExDate", ExDate); // if is set ExDate for renew 
  VALUE(id);
  VALUE(objectID);
  VALUE(regID);
  VALUE(operation);
  VALUE(zone);
  VALUE(period);
  VAL(ExDate);
  if (EXEC() ) {

    LOG( DEBUG_LOG , "SaveInvoiceCredit:   price %ld   invoiceID %d" , price , invoiceID );

    INSERT("invoice_object_registry_price_map");
    INTO("id");
    INTO("invoiceID");
    INTO("price");
    VALUE(id);
    VALUE(invoiceID);
    VALPRICE(price);
    if ( !EXEC() ) {
      LOG( ERROR_LOG , "ERROR insert invoice_object_registry_price_map" );
      return false;
    }

    if (price2) // save next price credit came from next advance invoice
    {
      LOG( DEBUG_LOG , "uctovani creditu  price2 %ld   invoiceID2 %d" , price2 , invoiceID2 );

      INSERT("invoice_object_registry_price_map");
      INTO("id");
      INTO("invoiceID");
      INTO("price");
      VALUE(id);
      VALUE(invoiceID2);
      VALPRICE(price2);
      if ( !EXEC() ) {
        LOG( ERROR_LOG , "ERROR insert invoice_object_registry_price_map price2" );
        return false;
      }
    }

    return true;
  } else {
    LOG( ERROR_LOG , "ERROR SaveInvoiceCredit invoiceID %d objectid %d " , invoiceID , objectID );
    return false;
  }

}

bool DB::InvoiceCountCredit(
  long price, int invoiceID)
{
  char sqlString[256];

  // if the zero price not update credit 
  if (price == 0) {
    LOG( DEBUG_LOG , "nulova castka nemenim credit u invoiceID %d" , invoiceID );
    return true;
  } else {
    // count credit on the advance invoice
    sprintf(sqlString,
        "UPDATE invoice SET  credit=credit-%ld%c%02ld  WHERE id=%d;", price
            /100, '.', price %100, invoiceID);
    if (ExecSQL(sqlString) )
      return true;
    else {
      LOG( ERROR_LOG , "error InvoiceCountCredit invoice  %d price %ld" , invoiceID , price );
      return false;
    }
  }

}

// billing operation   CREATE domain
bool DB::BillingCreateDomain(
  int regID, int zone, int objectID)
{
  return UpdateInvoiceCredit(regID, OPERATION_DomainCreate, zone, 0, "",
      objectID);
}
// billing operation RENEW domain
bool DB::BillingRenewDomain(
  int regID, int zone, int objectID, int period, const char *ExDate)
{
  return UpdateInvoiceCredit(regID, OPERATION_DomainRenew, zone, period,
      ExDate, objectID);
}

// count credit from one or two  advance invoice 
bool DB::UpdateInvoiceCredit(
  int regID, int operation, int zone, int period, const char *ExDate,
  int objectID)
{
  char sqlString[256];
  long price, credit;
  long price1, price2;
  int invoiceID;
  int invID[2];
  long cr[2];
  int i;
  int num = 0;

  LOG( DEBUG_LOG , "UpdateInvoiceCredit operation %d objectID %d ExDate [%s]  period %d regID %d" , operation , objectID , ExDate , period , regID );

  // system registrar work free without invoicing 
  if (GetRegistrarSystem(regID) == true)
    return true;

  // get price per operation  for zone and interval period
  price = GetPrice(operation, zone, period);

  if (price == -2)
    return false; // SQL error get price

  // if the price not set  the operation is allowed and not billing
  if (price == -1)
    return true;

  // query where is a credit on the advance invoice get maximal two 
  snprintf(
      sqlString,
      sizeof(sqlString),
      "SELECT id, credit FROM invoice WHERE registrarid=%d and zone=%d and credit > 0 order by id limit 2 FOR UPDATE;",
      regID, zone);

  invoiceID=0;

  if (ExecSelect(sqlString) ) {
    num = GetSelectRows();

    for (i = 0; i < num; i ++) {

      invID[i] = atoi(GetFieldValue(i, 0) );
      // credit 
      cr[i] = (long) rint( 100.0 * atof(GetFieldValue(i, 1) ) );
    }
    FreeSelect();
  }

  // not any advance invoice can not billing credit
  if (num == 0)
    return false;

  credit= cr[0];
  invoiceID=invID[0];

  if (credit - price > 0) {

    if (SaveInvoiceCredit(regID, objectID, operation, zone, period, ExDate,
        price, 0, invoiceID, 0) ) {
      return InvoiceCountCredit(price, invoiceID);
    }

  } else {

    if (num == 2) // if exist next advance invoice 
    {

      price1= cr[0]; // balance on the first invoice 
      price2= price - cr[0]; // next credit get from second  invoice

      // biling 
      if (SaveInvoiceCredit(regID, objectID, operation, zone, period, ExDate,
          price1, price2, invID[0], invID[1]) ) {
        if (InvoiceCountCredit(cr[0], invID[0]) ) // count to zero on the first invoice 
        {
          invoiceID=invID[1];
          LOG( DEBUG_LOG , "UpdateInvoiceCredit: price  credit0 %ld credit1 %ld price %ld second price %ld" , cr[0] , cr[1] , price1 , price2 );

          if (cr[1] - price2 >= 0) { 
            // second invoice 
            return InvoiceCountCredit(price2, invID[1]);

          } else
            LOG( WARNING_LOG , "UpdateInvoiceCredit: not enough credit on second invoice id %d: price left after charging previous invoice: %ld, credit on this invoice: %ld", invoiceID, price2, cr[1]);

        }

      }

    } else
      LOG( WARNING_LOG , "UpdateInvoiceCredit: not next invoice to count ");

  }

  return false;
}

int DB::SaveXMLout(
  const char *svTRID, const char *xml)
{
  int actionID;

  actionID = GetNumericFromTable("action", "id", "serverTRID", svTRID);

  if (actionID > 0) {

    if (strlen(xml) ) {
      UPDATE("Action_XML");
      SET("xml_out", xml);
      WHERE("actionID", actionID);
      if (EXEC() )
        return true;
    }

  }

  // default
  return false;
}

// action 
bool DB::BeginAction(
  int clientID, int action, const char *clTRID, const char *xml,
  ParsedAction* paction
)
{

  bool ret = false;

  if (!BeginTransaction())
      return false;

  // actionID for loging all action
  actionID = GetSequenceID("action");
  loginID = clientID; // id of corba client
  historyID = 0; // history ID 


  if (actionID) {

    //    make  server ticket  svrTRID

    if (svrTRID==NULL) {
      svrTRID= new char[MAX_SVTID];

      // create  server ticket
      sprintf(svrTRID, "ccReg-%010d", actionID);
      LOG( SQL_LOG , "Make svrTRID: %s" , svrTRID );
    }

    // EPP operation
    enum_action=action;

    // write to action table
    INSERT("ACTION");
    INTO("id");
    if (clientID > 0)
      INTO("clientID");
    INTO("action");
    INTO("clienttrid");

    VALUE(actionID);
    if (clientID > 0)
      VALUE(clientID);
    VALUE(action);
    VALUE(clTRID);

    if (EXEC() ) {
      // write XML from epp-client
      if (strlen(xml) ) {
        INSERT("Action_XML");
        VALUE(actionID);
        VALUE(xml);
        if (EXEC() )
          ret = true;
      } else
        ret = true;
    }

    if (ret == true) {
      if (paction)
        ret = paction->executeSQL(actionID,this);
    }
  }

  QuitTransaction(ret ? CMD_OK : 0);

  return ret;
}

// end of EPP operation
const char * DB::EndAction(
  int response)
{
  int id;

  if (actionID == 0)
    return "no action";
  else {

    UPDATE("ACTION");
    if (response > 0)
      SET("response", response);
    SET("enddate", "now");
    SSET("servertrid", svrTRID); // without escape
    WHEREID(actionID);

    // update table
    id = actionID;
    actionID = 0;
    LOG( SQL_LOG , "EndAction svrTRID: %s" , svrTRID );

    if (EXEC() )
      return svrTRID;
    else {
      LOG( ERROR_LOG , "End action DATABASE_ERROR" );
      return "";
    }

  }

}

const char * DB::GetObjectCrDateTime(
  int id)
{
  convert_rfc3339_timestamp(dtStr, GetValueFromTable("OBJECT_registry",
      "CrDate", "id", id) );
  return dtStr;
}

const char * DB::GetDomainExDate(
  int id)
{
  // now it is just a date - no conversion needed
  return GetValueFromTable("DOMAIN", "ExDate", "id", id);
  // convert_rfc3339_date(dtStr, GetValueFromTable("DOMAIN", "ExDate", "id", id) );
  // return dtStr;
}

const char * DB::GetDomainValExDate(
  int id)
{
  // now it is just a date - no conversion needed
  return GetValueFromTable("enumval", "ExDate", "domainid", id);
  // convert_rfc3339_date(dtStr, GetValueFromTable("enumval", "ExDate",
  // "domainid", id) );
  // return dtStr;
}

char * DB::GetFieldDateTimeValueName(
  const char *fname, int row)
{
  convert_rfc3339_timestamp(dtStr, GetFieldValueName( (char * ) fname , row) ) ;
  return dtStr;
}

char * DB::GetFieldDateValueName(
  const char *fname, int row)
{
  convert_rfc3339_date(dtStr, GetFieldValueName( (char * ) fname , row) ) ;
  return dtStr;
}

// get number of dns host  associated to nsset
int DB::GetNSSetHosts(
  int nssetID)
{
  char sqlString[128];
  int num=0;

  sprintf(sqlString, "SELECT id FROM host  WHERE nssetID=%d;", nssetID);

  if (ExecSelect(sqlString) ) {
    num = GetSelectRows();
    LOG( SQL_LOG , "nsset %d num %d" , nssetID , num );
    FreeSelect();
  }

  return num;
}

// get number of dsrecords associated to keyset
int 
DB::GetKeySetDSRecords(int keysetID)
{
    char sqlString[128];
    int num = 0;

    sprintf(sqlString, "SELECT id FROM dsrecord WHERE keysetid=%d;", keysetID);

    if (ExecSelect(sqlString)) {
        num = GetSelectRows();
        LOG(SQL_LOG, "keyset id(%d) has %d dsrecord(s)", keysetID, num);
        FreeSelect();
    }

    return num;
}
// get id of dsrecord
int 
DB::GetDSRecordId(
        int keysetId,
        int keyTag,
        int alg,
        int digestType,
        const char *digest,
        int maxSigLife)
{
    std::stringstream query;
    int id = 0;
    query
        << "SELECT id"
        << " FROM dsrecord"
        << " WHERE keysetid=" << keysetId
        << " AND keytag=" << keyTag
        << " AND alg=" << alg
        << " AND digest='" << digest << "'";
    if (maxSigLife != -1)
        query << " AND maxsiglife=" << maxSigLife;

    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG(SQL_LOG, "Found dsrecord id(%d) with same values", id);
        }
        FreeSelect();
    }
    return id;
}

// get id of dsrecord, dont care about keyset id
int 
DB::GetDSRecordId(
        int keyTag,
        int alg,
        int digestType,
        const char *digest,
        int maxSigLife)
{
    std::stringstream query;
    int id = 0;
    query
        << "SELECT id"
        << " FROM dsrecord"
        << " WHERE keytag=" << keyTag
        << " AND alg=" << alg
        << " AND digest='" << digest << "'";
    if (maxSigLife != -1)
        query << " AND maxsiglife=" << maxSigLife;

    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG(SQL_LOG, "Found dsrecord id(%d) with same values", id);
        }
        FreeSelect();
    }
    return id;
}

// returns number of dnskey records associated to keyset
int
DB::GetKeySetDNSKeys(int keysetId)
{
    std::stringstream query;
    int ret = 0;

    query << "SELECT id FROM dnskey WHERE keysetid=" << keysetId << ";";
    if (ExecSelect(query.str().c_str())) {
        ret = GetSelectRows();
        LOG(SQL_LOG, "Keyset id(%d) has %d dnskey(s)",
                keysetId, ret);
        FreeSelect();
    }
    return ret;
}

// get id of dnskey
int
DB::GetDNSKeyId(
        int keysetId,
        int flags,
        int protocol,
        int alg,
        const char *key)
{
    std::stringstream query;
    int id = 0;
    query
        << "SELECT id"
        << " FROM dnskey"
        << " WHERE keysetid=" << keysetId
        << " AND flags=" << flags
        << " AND protocol=" << protocol
        << " AND alg=" << alg
        << " AND key='" << key << "'";
    if (ExecSelect(query.str().c_str())) {
        if (GetSelectRows() > 0) {
            id = atoi(GetFieldValue(0, 0));
            if (id != 0) {
                LOG(SQL_LOG, "Found dnskey id(%d) with same values", id);
            }
        }
        FreeSelect();
    }
    return id;
}
// get id of dnskey, don't care about keyset id
int
DB::GetDNSKeyId(
        int flags,
        int protocol,
        int alg,
        const char *key)
{
    std::stringstream query;
    int id = 0;
    query
        << "SELECT id"
        << " FROM dnskey"
        << " WHERE flags=" << flags
        << " AND protocol=" << protocol
        << " AND alg=" << alg
        << " AND key='" << key << "'";
    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG(SQL_LOG, "Found dnskey id(%d) with same values", id);
        }
        FreeSelect();
    }
    return id;
}


// if the registrar is client of the object
bool DB::TestObjectClientID(
  int id, int regID)
{
  char sqlString[128];
  bool ret=false;

  // system registrator o
  if (GetRegistrarSystem(regID) == true)
    return true; // has rights for all object
  else {
    sprintf(sqlString, "SELECT id FROM  object WHERE id=%d and clID=%d ", id,
        regID);
    if (ExecSelect(sqlString) ) {
      if (GetSelectRows() == 1)
        ret = true;
      FreeSelect();
    }

    return ret;
  }

}

const char * DB::GetObjectName(
  int id)
{
  return GetValueFromTable("object_registry", "name", "id", id);
}

int DB::GetContactID(
  const char *handle)
{
  char HANDLE[64];
  // to upper case and test
  if (get_CONTACTHANDLE(HANDLE, handle) == false)
    return -1;
  else
    return GetObjectID( 1, HANDLE);
}

int DB::GetNSSetID(
  const char *handle)
{
  char HANDLE[64];
  // to upper case and test 
  if (get_NSSETHANDLE(HANDLE, handle) == false)
    return -1;
  else
    return GetObjectID( 2, HANDLE);
}

int
DB::GetKeySetID(const char *handle)
{
    char HANDLE[64];
    // to upper case and test
    if (get_KEYSETHANDLE(HANDLE, handle) == false)
        return -1;
    else
        return GetObjectID(4, HANDLE);
}

int DB::GetObjectID(
  int type, const char *name)
{
  char sqlString[512];
  int id=0;

  sprintf(
      sqlString,
      "SELECT object.id FROM object_registry , object WHERE object_registry.type=%d AND object_registry.id=object.id AND object_registry.name=\'%s\';",
      type, name);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      id = atoi(GetFieldValue( 0, 0) );
      LOG( SQL_LOG , "GetObjectID   name=\'%s\'  -> ID %d" , name , id );
    }

    FreeSelect();
  }

  return id;
}

// get number of tech-c associated to nsset
int DB::GetNSSetContacts(
  int nssetID)
{
  char sqlString[128];
  int num=0;

  sprintf(sqlString, "SELECT * FROM nsset_contact_map  WHERE nssetID=%d;",
      nssetID);

  if (ExecSelect(sqlString) ) {
    num = GetSelectRows();
    LOG( SQL_LOG , " nsset_contact_map  num %d" , num );
    FreeSelect();
  }

  return num;
}

// get number of tech contact associated to keyset
int
DB::GetKeySetContacts(int keysetid)
{
    char sqlString[128];
    int num = 0;
    sprintf(sqlString, "SELECT * FROM keyset_contact_map WHERE keysetid=%d;",
            keysetid);

    if (ExecSelect(sqlString)) {
        num = GetSelectRows();
        LOG(SQL_LOG, " keyset_contact_map num %d", num);
        FreeSelect();
    }

    return num;
}

// save object as deleted
bool DB::SaveObjectDelete(
  int id)
{
  LOG( SQL_LOG , "set delete objectID %d" , id );
  UPDATE("object_registry");
  SET("ErDate", "now");
  WHEREID(id);
  return EXEC();
}

bool DB::SaveObjectCreate(
  int id)
{
  LOG( SQL_LOG , "set create histyoryID for object ID %d historyID %d" , id , historyID);
  if (historyID) {
    UPDATE("object_registry");
    SET("crhistoryid", historyID);
    WHEREID(id);
    return EXEC();
  } else
    return false;
}

bool DB::TestContactHandleHistory(
  const char * handle, int days)
{
  return TestObjectHistory(handle, days);
}

bool DB::TestNSSetHandleHistory(
  const char * handle, int days)
{
  return TestObjectHistory(handle, days);
}

bool DB::TestDomainFQDNHistory(
  const char * fqdn, int days)
{
  return TestObjectHistory(fqdn, days);
}

bool
DB::TestKeySetHandleHistory(const char *handle, int days)
{
    return TestObjectHistory(handle, days);
}

// test protected period 
bool DB::TestObjectHistory(
  const char * name, int days)
{
  /* TODO rework
   char sqlString[512];
   bool ret=false;
   int count;

   if( days > 0 )
   {
   // it doesn't depend if lowercase or uppercase 
   sprintf( sqlString , "SELECT count( id ) FROM object_delete  WHERE name ILIKE \'%s\' and  deltime  > current_timestamp - interval\'%d days\';"  , name , days );

   if( ExecSelect( sqlString ) )
   {
   count = atoi(  GetFieldValue( 0 , 0 ) ) ;
   if( count > 0 )  ret=true ;
   FreeSelect();
   }

   return ret;
   } else 
   */
  return false;

}

int DB::CreateObject(
  const char *type, int regID, const char *name, const char *authInfoPw)
{
  if (!type)
    return 0;
  unsigned itype;
  switch (type[0]) {
    case 'C':
      itype = 1;
      break;
    case 'N':
      itype = 2;
      break;
    case 'D':
      itype = 3;
      break;
    case 'K':
      itype = 4;
      break;
    default:
      return 0;
  }
  std::stringstream sql;
  // TODO: name should be escaped but is called after handle check so it's safe
  sql << "SELECT create_object(" << regID << ",'" << name << "'," << itype
      << ")";
  if (!ExecSelect(sql.str().c_str()))
    return -1;
  unsigned long long id = atoll(GetFieldValue(0, 0));
  FreeSelect();
  if (!id)
    return 0;
  INSERT("OBJECT");
  INTO("id");
  INTO("ClID");
  INTO("AuthInfoPw");

  VALUE(id);
  VALUE(regID);

  char pass[PASS_LEN+1];

  if (strlen(authInfoPw) == 0) {
    random_pass(pass); // autogenerate password if not set 
    VVALUE(pass);
  } else
    VALUE(authInfoPw);

  if (EXEC() )
    return id;
  else
    return 0;
}

// return  true if is system registrar 
bool DB::GetRegistrarSystem(
  int regID)
{
  char sqlString[128];
  bool ret=false;

  sprintf(sqlString, "SELECT system FROM registrar where id=%d;", regID);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      ret = GetFieldBooleanValueName("system", 0);

    }
    FreeSelect();
  }

  if (ret)
    LOG( SQL_LOG , "GetRegistrarSystem TRUE" );
  else
    LOG( SQL_LOG , "GetRegistrarSystem FALSE");

  return ret;
}

// TODO test and of  invoicing
// test registrar access to zone depend on the begin  of invoicing  table registrarinvoice
bool DB::TestRegistrarZone(
  int regID, int zone)
{
  bool ret = false;
  char sqlString[256];

  // system registrar has rigths to all zone
  if (GetRegistrarSystem(regID) == true)
    return true;

  sprintf(
      sqlString,
      "SELECT  id  FROM  registrarinvoice  WHERE registrarid=%d and zone=%d and fromdate <= CURRENT_DATE and (todate >= CURRENT_DATE or todate is null);",
      regID, zone);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() > 0) {
      ret=true;
    }
    FreeSelect();
  }

  return ret;
}

// test contact in the contact map
bool DB::CheckContactMap(
  const char * table, int id, int contactid, int role)
{
  bool ret = false;
  std::stringstream sql;
  sql << "SELECT * FROM " << table << "_contact_map " << "WHERE " << table
      << "id=" << id << " AND contactid=" << contactid;
  if (role)
    sql << " AND role=" << role;
  if (ExecSelect(sql.str().c_str())) {
    if (GetSelectRows() >= 1)
      ret = true; // contact exist
    FreeSelect();
  }
  return ret;
}

// add contact to contact table
bool DB::AddContactMap(
  const char * table, int id, int contactid)
{
  char sqlString[128];

  sprintf(sqlString, "INSERT INTO %s_contact_map VALUES ( %d , %d );", table,
      id, contactid);

  return ExecSQL(sqlString);
}

bool DB::TestValExDate(
  const char *valexDate, int period, int interval, int id)
{
  char sqlString[512];
  char exDate[MAX_DATE+1]; //  ExDate old value  in the table enumval
  bool ret =false;
  char currentDate[MAX_DATE+1];
  bool use_interval=false; // default value for using protected interval
  // std::stringstream sql;

  // actual local date based on the timezone
  get_rfc3339_timestamp(time(NULL) , currentDate, true);

  if (id) // if ValExDate already exist and updated 
  {
    // copy current Exdate during update 
    strncpy(exDate, GetDomainValExDate(id), MAX_DATE) ;

    // USE SQL for calculate
    // test if the ExDate is lager then actual date and less or equal to protected period (interval days)

    // OS:
    //    sql << "SELECT date_gt(date('" << exDate << "'), date('" << currentDate "')) AND " 
    //        << "date_le(date('" << exDate << "'), date(date('" << currentDate << "') + interval '"
    //        << interval << " days')) as test;";

    snprintf(
        sqlString,
        sizeof(sqlString),
        "SELECT   date_gt( date(\'%s\') , date(\'%s\') ) AND \
           date_le ( date(\'%s\') ,  date ( date(\'%s') + interval'%d days' ) ) as test; ",
        exDate, currentDate, exDate, currentDate, interval);

    // As a test value
    //    if (ExecSelect(sql.str().c_str())) {
    if (ExecSelect(sqlString)) {
      if (GetSelectRows() == 1)
        use_interval = GetFieldBooleanValueName("test", 0);
      FreeSelect();
    }
  }

  if (use_interval) // use current exDate as max value is int the protected interval
    snprintf(
        sqlString,
        sizeof(sqlString),
        "SELECT   date_gt( date(\'%s\' ) , date(\'%s') ) AND \
             date_le( date(\'%s\') , date( date(\'%s\') + interval'%d  months' ) ) as test; ",
        valexDate, currentDate, valexDate, exDate, period);
  else
    // use current date
    snprintf(
        sqlString,
        sizeof(sqlString),
        "SELECT   date_gt( date(\'%s\' ) , date(\'%s') ) AND \
             date_le( date(\'%s\') , date( date(\'%s\') + interval'%d  months' ) ) as test; ",
        valexDate, currentDate, valexDate, currentDate, period);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1)
      ret =GetFieldBooleanValueName("test", 0); // return value true if is OK

    FreeSelect();
  }

  return ret;
}

// test for  renew domain
bool DB::CountExDate(
  int domainID, int period, int max_period)
{
  char sqlString[256];
  bool ret=false;

  snprintf(
      sqlString,
      256,
      "SELECT  ExDate+interval\'%d month\' FROM DOMAIN WHERE id=%d AND ExDate+interval\'%d month\' < current_date+interval\'%d month\';",
      period, domainID, period, max_period);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1)
      ret = true;
    FreeSelect();
  }

  return ret;
}

// make and count new ExDate for renew domain
bool DB::RenewExDate(
  int domainID, int period)
{
  char sqlString[256];

  snprintf(sqlString, 256,
      "UPDATE domain SET  ExDate=ExDate+interval\'%d month\' WHERE id=%d ;",
      period, domainID);

  return ExecSQL(sqlString);
}

int DB::GetDomainID(
  const char *fqdn, bool enum_zone)
{
  return GetObjectID( 3, fqdn);
}

// return ID of host
int DB::GetHostID(
  const char *fqdn, int nssetID)
{
  char sqlString[128];
  int hostID=0;

  sprintf(sqlString, "SELECT id FROM HOST WHERE fqdn=\'%s\' AND nssetid=%d;",
      fqdn, nssetID);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      hostID = atoi(GetFieldValue( 0, 0) );
      LOG( SQL_LOG , "CheckHost fqdn=\'%s\' nssetid=%d  -> hostID %d" , fqdn , nssetID , hostID );
    }

    FreeSelect();
  }

  if (hostID == 0)
    LOG( SQL_LOG , "Host fqdn=\'%s\' not found" , fqdn );
  return hostID;
}

// test for nsset is linked to domain
bool DB::TestNSSetRelations(
  int id)
{
  bool ret = false;
  char sqlString[128];

  sprintf(sqlString, "SELECT id from DOMAIN WHERE nsset=%d;", id);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() > 0)
      ret=true;
    FreeSelect();
  }

  return ret;
}

// test for keyset is linked to domain
bool
DB::TestKeySetRelations(int id)
{
    bool ret = false;
    char sqlString[128];

    sprintf(sqlString, "SELECT id FROM DOMAIN WHERE keyset=%d;", id);
    if (ExecSelect(sqlString)) {
        if (GetSelectRows() > 0)
            ret = true;
        FreeSelect();
    }
    return ret;
}

bool DB::TestContactRelations(
  int id)
{
  int count=0;
  char sqlString[128];

  sprintf(sqlString, "SELECT count(id) from DOMAIN WHERE Registrant=%d;", id);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) );
    FreeSelect();
  }

  if (count > 0)
    return true;

  sprintf(sqlString,
      "SELECT count(nssetID ) from NSSET_CONTACT_MAP WHERE contactid=%d;", id);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) );
    FreeSelect();
  }

  if (count > 0)
    return true;

  sprintf(sqlString,
          "SELECT count(keysetID) from KEYSET_CONTACT_MAP WHERE contactid=%d;", id);
  if (ExecSelect(sqlString)) {
      count = atoi(GetFieldValue(0, 0));
      FreeSelect();
  }

  if (count > 0)
    return true;

  sprintf(sqlString,
      "SELECT count( domainID)  from DOMAIN_CONTACT_MAP WHERE contactid=%d;",
      id);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) );

    FreeSelect();
  }

  if (count > 0)
    return true;
  else
    return false;
}

// compare authinfopw 
bool DB::AuthTable(
  const char *table, const char *auth, int id)
{
  bool ret=false;
  const char *pass;
  char sqlString[128];

  sprintf(sqlString, "SELECT authinfopw from %s WHERE id=%d", table, id);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      pass = GetFieldValue( 0, 0);
      if (strcmp(pass, auth) == 0)
        ret = true;
    }

    FreeSelect();
  }

  return ret;
}

int DB::GetSystemVAT() // return VAT for invoicing depend on the time
{
  char sqlString[128] =
      "select vat from price_vat where valid_to > now() or valid_to is null;";
  int dph=0;

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      dph = atoi(GetFieldValue( 0, 0) );
    }
    FreeSelect();
  }

  return dph;
}

double DB::GetSystemKOEF() // return VAT count parametr for count price without VAT
{
  char sqlString[128] =
      "select koef   from price_vat where valid_to > now() or valid_to is null;";
  double koef = 0;

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      koef = atof(GetFieldValue( 0, 0) );
    }
    FreeSelect();
  }

  return koef;
}

// get ID of back account
int DB::GetBankAccount(
  const char *accountStr, const char *codeStr)
{
  std::stringstream sqlString;
  int accountID=0;

  LOG( LOG_DEBUG ,"GetBankAccount account %s , code %s" , accountStr ,codeStr );
  sqlString
      << "SELECT id FROM bank_account WHERE trim(leading '0' from account_number)="
      << "trim(leading '0' from '"
      << accountStr
      << "') AND bank_code='"
      << codeStr
      << "';";
  if (ExecSelect(sqlString.str().c_str()) ) {
    if (GetSelectRows() == 1) {
      accountID=atoi(GetFieldValue( 0, 0) );
      LOG( LOG_DEBUG ,"get accountId %d" , accountID );
    }
    FreeSelect();
  }
  return accountID;
}

// get zone for bank account
int DB::GetBankAccountZone(
  int accountID)
{
  char sqlString[128];
  int zone=0;

  LOG( LOG_DEBUG ,"GetBankAccountZone accountID %d" , accountID );
  sprintf(sqlString, "SELECT  zone  FROM bank_account WHERE id=%d", accountID);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      zone = atoi(GetFieldValue( 0, 0) );
      LOG( LOG_DEBUG ,"get zone %d" , zone );
    }
    FreeSelect();
  }

  return zone;
}

// test balance on the account for importing bank statement
int DB::TestBankAccount(
  const char *accountStr, int num, long oldBalance, char *bank)
{
  int accountID=0;
  int lastNum=0;
  long lastBalance=0;

  if (SELECTONE("bank_account", "account_number", accountStr) ) {
    if (GetSelectRows() == 1) {
      accountID = atoi(GetFieldValueName("id", 0) );
      lastNum = atoi(GetFieldValueName("last_num", 0) );
      lastBalance
          = (long) rint( 100.0 * atof(GetFieldValueName("balance", 0) ) );
      strncpy(bank, GetFieldValueName("bank_code", 0), 4);
    }

    FreeSelect();
  }

  if (accountID) {
    LOG( LOG_DEBUG ,"posledni vypis ucetID %d cislo %d zustatek na uctu %ld" , accountID , lastNum , lastBalance);

    // test for numer of statement not for the first statement
    if (num > 1) {
      if (lastNum + 1 != num) {
        LOG( ERROR_LOG , "chyba nesedi cislo  vypisu %d  posledni nacteny je %d" , num , lastNum );
      }
    }

    // next test if balance is ok
    if (oldBalance == lastBalance)
      return accountID;
    else {
      LOG( ERROR_LOG , "chyba nesedi zustatek na uctu poslednu zustatek %ld nacitany stav %ld" , lastBalance , oldBalance );
    }

  } else {
    LOG( ERROR_LOG , "nelze najit ucet na vypisu cislo %s" , accountStr );
  }

  return 0;
}

// update on the bank account
bool DB::UpdateBankAccount(
  int accountID, char *date, int num, long newBalance)
{

  UPDATE("bank_account");
  SET("last_date", date);
  SET("last_num", num);
  SETPRICE("balance", newBalance);
  WHEREID(accountID);

  return EXEC();
}

// save bank statement 
int DB::SaveBankHead(
  int accountID, int num, char *date, char *oldDate, long oldBalance,
  long newBalance, long credit, long debet)
{
  int statemetID;

  statemetID = GetSequenceID("bank_statement_head");

  INSERT("bank_statement_head");
  INTO("id");
  INTO("account_id");
  INTO("num");
  INTO("create_date");
  INTO("balance_old_date");
  INTO("balance_old");
  INTO("balance_new");
  INTO("balance_credit");
  INTO("balance_debet");
  VALUE(statemetID);
  VALUE(accountID);
  VALUE(num);
  VALUE(date);
  VALUE(oldDate);
  VALPRICE(oldBalance);
  VALPRICE(newBalance);
  VALPRICE(credit);
  VALPRICE(debet);

  if (EXEC() )
    return statemetID;
  else
    return 0;

}
//  save items of bank statement
int DB::SaveBankItem(
  int statemetID, char *account, char *bank, char *evidNum, char *date,
  char *memo, int code, char *konstSymb, char *varSymb, char *specsymb,
  long price)
{
  int itemID;

  itemID = GetSequenceID("bank_statement_item");
  if (itemID <= 0)
    return 0;

  INSERT("bank_statement_item");
  INTO("id");
  INTO("statement_id");
  INTO("account_number");
  INTO("bank_code");
  INTO("account_evid");
  INTO("account_date");
  INTO("account_memo");
  INTO("code");
  INTO("konstsym");
  INTO("varsymb");
  INTO("specsymb");
  INTO("price");
  VALUE(itemID);
  VALUE(statemetID);
  VALUE(account);
  VALUE(bank);
  VALUE(evidNum);
  VALUE(date);
  VALUE(memo);
  VALUE(code);
  VALUE(konstSymb);
  VALUE(varSymb);
  VALUE(specsymb);
  VALPRICE(price);

  if (!EXEC())
    return 0;
  return itemID;
}

// mark bank statement as imported
bool DB::UpdateBankStatementItem(
  int id, int invoiceID)
{
  UPDATE("bank_statement_item");
  SET("invoice_id", invoiceID);
  WHEREID(id);
  return EXEC();
}

// for E-Banka https bank statement
int DB::TestEBankaList(
  const char *ident)
{
  char sqlString[128];
  int id=0;

  sprintf(sqlString, "SELECT  id    from BANK_EBANKA_LIST where ident=\'%s\'",
      ident);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      id= atoi(GetFieldValue( 0, 0) );

    }
    FreeSelect();
  }

  return id;
}

int DB::SaveEBankaList(
  int account_id, const char *ident, long price, const char *datetimeStr,
  const char *accountStr, const char *codeStr, const char *varsymb,
  const char *konstsymb, const char *nameStr, const char *memoStr)
{
  int ID;

  // if not readed
  if (TestEBankaList(ident) == false) {

    ID = GetSequenceID("bank_ebanka_list");

    INSERT("BANK_EBANKA_LIST");
    INTO("id");
    INTO("account_id");
    INTO("account_number");
    INTO("bank_code");
    INTO("konstsym");
    INTO("varsymb");
    INTO("memo");
    INTO("name");
    INTO("ident");
    INTO("crdate");
    INTO("price");
    VALUE(ID);
    VALUE(account_id);
    VALUE(accountStr);
    VALUE(codeStr);
    VALUE(konstsymb);
    VALUE(varsymb);
    VALUE(memoStr);
    VALUE(nameStr);
    VALUE(ident);
    VALUE(datetimeStr);
    VALPRICE(price);

    if (EXEC() )
      return ID;
    else
      return -1; // error
  } else
    return 0;
}

// mark e-banka statement as imported on advance invoice
bool DB::UpdateEBankaListInvoice(
  int id, int invoiceID)
{
  UPDATE("BANK_EBANKA_LIST");
  SET("invoice_id", invoiceID);
  WHEREID(id);
  return EXEC();

}

// count new balance on advance invoice from total and all usages of that invoice 
long DB::GetInvoiceBalance(
  int aID, long credit)
{
  char sqlString[512];
  long total, suma;
  long price=-1; // err value


  LOG( NOTICE_LOG , "GetInvoiceBalance: zalohova FA %d" , aID );

  sprintf(sqlString, "select total from invoice where id=%d", aID);

  if (ExecSelect(sqlString) ) {
    total = (long) rint( 100.0 * atof(GetFieldValue( 0, 0) ) );
    LOG( NOTICE_LOG , "celkovy zaklad faktury %ld" , total );
    FreeSelect();

    sprintf(
        sqlString,
        "SELECT sum( credit ) FROM invoice_credit_payment_map where ainvoiceid=%d;",
        aID);
    if (ExecSelect(sqlString) ) {
      suma = (long) rint( 100.0 * atof(GetFieldValue( 0, 0) ) );
      LOG( NOTICE_LOG , "sectweny credit %ld  pro zal FA" , suma );
      FreeSelect();
    } else
      return -2; // err

    price = total - suma - credit;
    LOG( NOTICE_LOG , "celkovy zustatek pri  uzavreni Fa %ld" , price );

  } else
    return -1; // err

  return price;
}
// return accounted price for invoice  iID from advance invoice  FA aID
long DB::GetInvoiceSumaPrice(
  int iID, int aID)
{
  char sqlString[512];
  long price=-1; // err value
  LOG( NOTICE_LOG , "GetInvoiceSumaPrice invoiceID %d zalohova FA %d" , iID , aID );

  sprintf(
      sqlString,
      "SELECT  sum( invoice_object_registry_price_map.price ) FROM invoice_object_registry ,  invoice_object_registry_price_map\
                 WHERE invoice_object_registry.id=invoice_object_registry_price_map.id AND invoice_object_registry.invoiceid=%d AND \
                   invoice_object_registry_price_map.invoiceid=%d; ",
      iID, aID);

  if (ExecSelect(sqlString) ) {
    if (IsNotNull( 0, 0) ) {
      price = (long) rint( 100.0 * atof(GetFieldValue( 0, 0) ) );
      LOG( NOTICE_LOG , "celkovy strezeny credit z dane zal  Fa %ld" , price );
    }

    FreeSelect();
  } else
    return -1; // error

  return price;
}

// invoicing make new invoice
int DB::MakeFactoring(
  int regID, int zone, const char *timestampStr, const char *taxDateStr)
{
  char sqlString[512];
  int invoiceID=-1;
  int *aID;
  int i, num;
  char fromdateStr[MAX_DATE+1];
  char todateStr[MAX_DATE+1];
  int count=-1;
  long price = 0, credit, balance;

  LOG( NOTICE_LOG , "MakeFactoring regID %d zone %d" , regID , zone );

  // First step look into table invoice_generation till when invoicing was realised and this value 
  // todate take as from date
  fromdateStr[0]=0;

  // last record todate plus one day
  snprintf(
      sqlString,
      sizeof(sqlString),
      "SELECT date( todate + interval'1 day')  from invoice_generation  WHERE zone=%d  AND registrarid =%d  order by id desc limit 1;",
      zone, regID);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      strncpy(fromdateStr, GetFieldValue( 0, 0), MAX_DATE);
    }
    FreeSelect();
  } else
    return -1; // error


  // else STEP 2 

  if (fromdateStr[0]== 0) {
    // find out fromdate from tabel registrarinvoice from when invoicing 
    sprintf(
        sqlString,
        "SELECT  fromdate  from registrarinvoice  WHERE zone=%d and registrarid=%d;",
        zone, regID);
    if (ExecSelect(sqlString) ) {

      if (IsNotNull( 0, 0) ) {
        strncpy(fromdateStr, GetFieldValue( 0, 0), MAX_DATE);
      }

      FreeSelect();
    } else
      return -1; // error

  }

  // 
  strncpy(todateStr, timestampStr, 10);
  todateStr[10] = 0;
  LOG( NOTICE_LOG , "Fakturace od %s do %s timestamp [%s] " , fromdateStr , todateStr , timestampStr );

  // find out amount of item for invoicing
  sprintf(
      sqlString,
      "SELECT count( id)  from invoice_object_registry  where crdate < \'%s\' AND  zone=%d AND registrarid=%d AND invoiceid IS NULL;",
      timestampStr, zone, regID);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) ) ;
    FreeSelect();
  } else
    return -2; // error


  // find out total invoiced price if it exists al least one record
  if (count > 0) {
    sprintf(
        sqlString,
        "SELECT sum( price ) FROM invoice_object_registry , invoice_object_registry_price_map  WHERE   invoice_object_registry_price_map.id=invoice_object_registry.id AND  crdate < \'%s\' AND zone=%d and registrarid=%d AND  invoice_object_registry.invoiceid is null ;",
        timestampStr, zone, regID);
    if (ExecSelect(sqlString) ) {
      price = (long) rint( 100.0 * atof(GetFieldValue( 0, 0) ) );
      LOG( NOTICE_LOG , "Celkova castka na fakture %ld" , price );
      FreeSelect();
    }
  } else
    price = 0; // else null price


  // empty invoice invoicing record
  // returns invoiceID or null if nothings was invoiced, on error returns negative number of error
  if ( (invoiceID = MakeNewInvoice(taxDateStr, fromdateStr, todateStr, zone,
      regID, price, count) ) >= 0) {

    if (count > 0) // mark item of invoice
    {
      sprintf(
          sqlString,
          "UPDATE invoice_object_registry set invoiceid=%d  WHERE crdate < \'%s\' AND zone=%d and registrarid=%d AND invoiceid IS NULL;",
          invoiceID, timestampStr, zone, regID);
      if (ExecSQL(sqlString) == false)
        return -3; // error

    }

    // set last date into tabel registrarinvoice
    sprintf(
        sqlString,
        "UPDATE registrarinvoice SET lastdate=\'%s\' WHERE zone=%d and registrarid=%d;",
        todateStr, zone, regID);
    if (ExecSQL(sqlString) == false)
      return -4; // error
    // if invoice was created  
    if (invoiceID > 0) {

      // query for all advance invoices, from which were gathered for taxes FA 
      snprintf(
          sqlString,
          sizeof(sqlString),
          "select invoice_object_registry_price_map.invoiceid from  invoice_object_registry ,  invoice_object_registry_price_map  where invoice_object_registry.id=invoice_object_registry_price_map.id and invoice_object_registry.invoiceid=%d  GROUP BY invoice_object_registry_price_map.invoiceid ; ",
          invoiceID);
      // EXEC SQL a insert  invoice_credit_payment_map

      if (ExecSelect(sqlString) ) {
        num = GetSelectRows();
        aID = new int[num];

        // OS: to je stejně čuňárna, proč se to nedělá v jedné smyčce?

        for (i = 0; i < num; i ++) {
          aID[i] = atoi(GetFieldValue(i, 0) );
          LOG( LOG_DEBUG ,"zalohova FA -> %d" , aID[i]);
        }
        FreeSelect();

        // insert into table invoice_credit_payment_map;
        for (i = 0; i < num; i ++) {
          credit = GetInvoiceSumaPrice(invoiceID, aID[i]);
          balance = GetInvoiceBalance(aID[i], credit); // actual available balance
          if (balance >=0) {
            LOG( LOG_DEBUG ,"zalohova FA  %d credit %ld balance %ld" , aID[i] , credit , balance );
            INSERT("invoice_credit_payment_map");
            INTO("invoiceid");
            INTO("ainvoiceid");
            INTO("credit");
            INTO("balance");
            VALUE(invoiceID);
            VALUE(aID[i]);
            VALPRICE(credit);
            VALPRICE(balance);
            if ( !EXEC() ) {
              delete[] aID;
              return -7;
            } // error
          } else {
            delete[] aID;
            return -8;
          } // error

        }

        delete[] aID;
      } else
        return -6; // error
    }
  } else
    return -5; // invoice creation wasn't successful

  return invoiceID;
}

int DB::MakeNewInvoice(
  const char *taxDateStr, const char *fromdateStr, const char *todateStr,
  int zone, int regID, long price, unsigned int count)
{
  int invoiceID;
  long prefix;
  int type;
  int dph;

  LOG( LOG_DEBUG ,"MakeNewInvoice taxdate[%s]  fromdateStr [%s] todateStr[%s]  zone %d regID %d , price %ld  count %d" ,
      taxDateStr , fromdateStr , todateStr , zone , regID , price , count);
  if ( (type = GetPrefixType(taxDateStr, INVOICE_FA, zone) )) // usable prefix id of invoice  
  {

    if (count) // create invoice 
    {
      if ( (prefix = GetInvoicePrefix(taxDateStr, INVOICE_FA, zone) )) // number of invoice accord to taxable period
      {
        // find out VAT height 
        dph =GetSystemVAT();
        LOG( LOG_DEBUG ,"Make Invoice prefix %ld type %d DPH=%d\n" , prefix , type , dph);

        invoiceID = GetSequenceID("invoice");

        INSERT("invoice");
        INTO("id");
        INTO("prefix");
        INTO("zone");
        INTO("prefix_type");
        INTO("registrarid");
        INTO("taxDate");
        INTO("price");
        INTO("vat");
        INTO("total");
        INTO("totalVAT");
        INTO("credit");
        VALUE(invoiceID);
        VALUE(prefix);
        VALUE(zone);
        VALUE(type); // link into prefix
        VALUE(regID);
        VALUE(taxDateStr);
        VALPRICE(price); // total price
        VALUE(dph); // VAT is not null 
        VALUE( 0); // base without is zero amount  
        VALUE( 0);
        VALUENULL(); // only credit is NULL


        if ( !EXEC() )
          return -1; // SQL insert error 
      } else
        return -4;

    } else
      invoiceID=0; // empty invoicing


    // record of invoicing
    INSERT("invoice_generation");

    INTO("fromdate");
    INTO("todate");
    INTO("registrarid");
    INTO("zone");

    INTO("invoiceID");
    VALUE(fromdateStr);
    VALUE(todateStr);
    VALUE(regID);
    VALUE(zone);
    if (invoiceID)
      VALUE(invoiceID);
    else
      VALUENULL();

    if (EXEC() )
      return invoiceID; // insert error
    else
      return -3; // error 


  } else
    return -2; // prefix creation error  


}
// description in english
// creation of advance invoice for registrar for price amount with height VAT vatNum paid VAT tax and amount without VAT credit
// taxDateStr date of taxable fulfilment date when value came into our account
int DB::MakeNewInvoiceAdvance(
  const char *taxDateStr, int zone, int regID, long price)
{
  int invoiceID;
  long prefix;
  int dph;
  long total; // amount without VAT == credit
  long credit;
  long totalVAT; // paid VAT
  double koef; // conversion coefficient for VAT
  int type; // type of advance invoice (ZAL FA) from front 

  // patch by JT
  // making VAT parametr useless and taking it from DB
  std::stringstream sql;
  sql << "SELECT vat FROM registrar WHERE id=" << regID;
  if (!ExecSelect(sql.str().c_str()))
    return -3;
  if (GetSelectRows() != 1)
    return -4;
  bool VAT = *GetFieldValue(0, 0) == 't';
  FreeSelect();
  // end of patch by JT


  if (VAT) {
    // find out VAT height
    dph =GetSystemVAT(); // VAT height 19 %
    koef =GetSystemKOEF();// coefficient height
    // count paid VAT math rounded off at dimes 

    totalVAT = count_dph(price, koef);
    total = price - totalVAT;
    credit = total;

  } else // create advance invoice without paid VAT 
  {
    dph=0;
    totalVAT=0;
    total = price;
    credit = price;
  }

  type = GetPrefixType(taxDateStr, INVOICE_ZAL, zone);

  prefix = GetInvoicePrefix(taxDateStr, INVOICE_ZAL, zone);

  LOG( LOG_DEBUG ,"MakeNewInvoiceAdvance taxdate[%s]   zone %d regID %d , price %ld dph %d credit %ld" , taxDateStr , zone , regID , price , dph , credit );

  if (prefix > 0) {
    invoiceID = GetSequenceID("invoice");

    INSERT("invoice");
    INTO("id");
    INTO("prefix");
    INTO("zone");
    INTO("prefix_type");
    INTO("registrarid");
    INTO("taxDate");
    INTO("price");
    INTO("vat");
    INTO("total");
    INTO("totalVAT");
    INTO("credit");
    VALUE(invoiceID);
    VALUE(prefix);
    VALUE(zone);
    VALUE(type); // number of line from tabel invoice_prefix
    VALUE(regID);
    VALUE(taxDateStr);
    VALPRICE(price); // total price
    VALUE(dph); // VAT height 19 %
    VALPRICE(total); // price without VAT
    VALPRICE(totalVAT); // price without VAT
    VALPRICE(credit); // added credit

    if (EXEC() )
      return invoiceID;
    else
      return -1; // SQL insert error
  } else
    return -2; // prefix error 

}

int DB::GetPrefixType(
  const char *dateStr, int typ, int zone)
{
  char sqlString[512];
  int year;
  char yearStr[5];
  int id=0;

  // year
  strncpy(yearStr, dateStr, 4);
  yearStr[4] = 0;
  year= atoi(yearStr);
  LOG( LOG_DEBUG ,"GetPrefixType  date[%s]  year %d typ %d zone %d\n" , dateStr , year , typ , zone );

  sprintf(
      sqlString,
      "SELECT id  FROM invoice_prefix WHERE zone=%d AND  typ=%d AND year=\'%s\';",
      zone, typ, yearStr);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      id = atoi(GetFieldValue( 0, 0) );
      LOG( LOG_DEBUG ,"invoice_id type-> %d" , id );
    } else if (GetSelectRows() > 1) {
      LOG( ERROR_LOG , "Multiple rows selected from invoice_prefix. Using ID of the first record." );
      id = atoi(GetFieldValue( 0, 0));
    } else {
      LOG( ERROR_LOG , "Correct invoice prefix not found.");
      FreeSelect();
      return -3;
    }

    FreeSelect();
  }

  return id;
}

long DB::GetInvoicePrefix(
  const char *dateStr, int typ, int zone)
{
  char sqlString[512];
  int year;
  char yearStr[5];
  long prefix=0, id=0;

  // year
  strncpy(yearStr, dateStr, 4);
  yearStr[4] = 0;
  year= atoi(yearStr);

  LOG( LOG_DEBUG ,"GetInvoicePrefix date[%s]  year %d typ %d zone %d\n" , dateStr , year , typ , zone );

  sprintf(
      sqlString,
      "SELECT id , prefix   FROM invoice_prefix WHERE zone=%d AND  typ=%d AND year=\'%s\';",
      zone, typ, yearStr);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      id = atol(GetFieldValueName("id", 0) );
      prefix = atol(GetFieldValueName("prefix", 0) );
      LOG( LOG_DEBUG ,"invoice_prefix id %d -> %ld" , id , prefix );
    } else {
      LOG( ERROR_LOG ,"Requested invoice_prefix not found " );
      return -3; // error
    }

    FreeSelect();

    UPDATE("invoice_prefix");
    SET("prefix", prefix +1);
    WHEREID(id);
    if (EXEC() )
      return prefix;
    else
      return -2; // error


  } else
    return -1;

}

// vreturn id of the owner of domain
int DB::GetClientDomainRegistrant(
  int clID, int contactID)
{
  int regID=0;
  char sqlString[128];

  sprintf(sqlString,
      "SELECT  clID FROM DOMAIN WHERE Registrant=%d AND clID=%d", contactID,
      clID);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() > 0) {
      regID = atoi(GetFieldValue( 0, 0) );
      LOG( SQL_LOG , "Get ClientDomainRegistrant  contactID \'%d\' -> regID %d" , contactID , regID );
      FreeSelect();
    }
  }

  return regID;
}
// function for get one value from table
const char * DB::GetValueFromTable(
  const char *table, const char *vname, const char *fname, const char *value)
{
  char sqlString[512];
  int size;

  sprintf(sqlString, "SELECT  %s FROM %s WHERE %s=\'%s\';", vname, table,
      fname, value);

  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) // if selected only one record 
    {
      size = GetValueLength( 0, 0);

      if (memHandle) {
        delete[] memHandle;
        LOG( SQL_LOG , "re-alloc memHandle");
        memHandle = new char[size+1];
      } else {
        LOG( SQL_LOG , "alloc memHandle");
        memHandle = new char[size+1];
      }

      strncpy(memHandle, GetFieldValue( 0, 0), size + 1);
      LOG( SQL_LOG , "GetValueFromTable \'%s\' field %s  value  %s ->  %s" , table , fname , value , memHandle );
      FreeSelect();
      return memHandle;
    } else {
      FreeSelect();
      return "";
    }
  } else
    return "";

}

const char * DB::GetValueFromTable(
  const char *table, const char *vname, const char *fname, int numeric)
{
  char value[16];

  sprintf(value, "%d", numeric);

  return GetValueFromTable(table, vname, fname, value);
}

int DB::GetNumericFromTable(
  const char *table, const char *vname, const char *fname, const char *value)
{
  return atoi(GetValueFromTable(table, vname, fname, value) );
}

int DB::GetNumericFromTable(
  const char *table, const char *vname, const char *fname, int numeric)
{
  char value[16];

  sprintf(value, "%d", numeric);

  return GetNumericFromTable(table, vname, fname, value);
}

// remove date from table
bool DB::DeleteFromTable(
  const char *table, const char *fname, int id)
{
  char sqlString[128];

  LOG( SQL_LOG , "DeleteFromTable %s fname %s id -> %d" , table , fname , id );

  sprintf(sqlString, "DELETE FROM %s  WHERE %s=%d;", table, fname, id);
  return ExecSQL(sqlString);
}

// remove date from  contact map table
bool DB::DeleteFromTableMap(
  const char *map, int id, int contactid)
{
  char sqlString[128];

  LOG( SQL_LOG , "DeleteFrom  %s_contact_map  id  %d contactID %d" , map ,id , contactid );

  sprintf(sqlString,
      "DELETE FROM %s_contact_map WHERE  %sid=%d AND contactid=%d;", map, map,
      id, contactid);

  return ExecSQL(sqlString);
}

int DB::GetSequenceID(
  const char *sequence)
{
  char sqlString[128];
  int id=0;

  sprintf(sqlString, "SELECT  NEXTVAL( \'%s_id_seq\'  );", sequence);

  if (ExecSelect(sqlString) ) {
    id = atoi(GetFieldValue( 0, 0) );
    LOG( SQL_LOG , "GetSequence \'%s\' -> ID %d" , sequence , id );
    FreeSelect();
  }

  return id;
}

bool DB::SaveNSSetHistory(
  int id)
{

  //  save to history 
  if (MakeHistory(id) ) {

    if (SaveHistory("NSSET", "id", id) )
      if (SaveHistory("HOST", "nssetid", id) )
        if (SaveHistory("HOST_IPADDR_map", "nssetid", id) )
          if (SaveHistory("nsset_contact_map", "nssetid", id) ) // history of tech-c
            return true;
  }

  return 0;
}

bool
DB::SaveKeySetHistory(int id)
{
    // save to history
    if (MakeHistory(id))
        if (SaveHistory("KEYSET", "id", id))
            if (SaveHistory("DSRECORD", "keysetid", id))
                if (SaveHistory("dnskey", "keysetid", id))
                    if (SaveHistory("keyset_contact_map", "keysetid", id))
                        return true;
    return false;
}


bool DB::DeleteNSSetObject(
  int id)
{

  // first delete tech-c
  if (DeleteFromTable("nsset_contact_map", "nssetid", id) )
    if (DeleteFromTable("HOST_IPADDR_map", "nssetid", id) ) // delete  ip address
      if (DeleteFromTable("HOST", "nssetid", id) ) // delete dns hosts
        if (DeleteFromTable("NSSET", "id", id) )
          if (DeleteFromTable("OBJECT", "id", id) )
            return true;

  return false;
}

bool
DB::DeleteKeySetObject(int id)
{
    // first delete tech contact
    if (DeleteFromTable("keyset_contact_map", "keysetid", id))
        if (DeleteFromTable("dnskey", "keysetid", id))
            if (DeleteFromTable("dsrecord", "keysetid", id))
                if (DeleteFromTable("keyset", "id", id))
                    if (DeleteFromTable("object", "id", id))
                        return true;
    return false;
}

bool DB::SaveDomainHistory(
  int id)
{

  if (MakeHistory(id) ) {
    if (SaveHistory("DOMAIN", "id", id) )
      if (SaveHistory("domain_contact_map", "domainID", id) ) // save admin-c
        if (SaveHistory("enumval", "domainID", id) ) // save enum extension
          return true;
  }

  return 0;
}

bool DB::DeleteDomainObject(
  int id)
{
  if (DeleteFromTable("domain_contact_map", "domainID", id) ) // admin-c
    if (DeleteFromTable("enumval", "domainID", id) ) // enumval extension 
      if (DeleteFromTable("DOMAIN", "id", id) )
        if (DeleteFromTable("OBJECT", "id", id) )
          return true;

  return false;
}

bool DB::SaveContactHistory(
  int id)
{

  if (MakeHistory(id) ) {
    if (SaveHistory("Contact", "id", id) )
      return true;
  }

  return 0;
}

bool DB::DeleteContactObject(
  int id)
{

  if (DeleteFromTable("CONTACT", "id", id) ) {
    if (DeleteFromTable("OBJECT", "id", id) )
      return true;
  }

  return false;
}

int DB::MakeHistory(
  int objectID) // write records of object to the history
{
  char sqlString[128];

  if (actionID) {
    LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID);
    historyID = GetSequenceID("HISTORY");
    if (historyID) {
      LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID);
      sprintf(sqlString,
          "INSERT INTO HISTORY ( id , action ) VALUES ( %d  , %d );",
          historyID, actionID);
      if (ExecSQL(sqlString) ) {
        if (SaveHistory("OBJECT", "id", objectID) ) // save object table to history 
        {
          LOG( SQL_LOG , "Update objectID  %d -> historyID %d " , objectID , historyID );
          sprintf(sqlString,
              "UPDATE OBJECT_registry set historyID=%d WHERE id=%d;",
              historyID, objectID);
          if (ExecSQL(sqlString) )
            return historyID;
        }
      }
    }
  }

  // default
  return 0;
}

// save row of tabel with id
bool DB::SaveHistory(
  const char *table, const char *fname, int id)
{
  int i, row;
  bool ret= true; // default


  if (historyID) {
    LOG( SQL_LOG , "SaveHistory historyID - > %d" , historyID );
    if (SELECTONE(table, fname, id) ) {
      for (row = 0; row < GetSelectRows() ; row ++) {
        // insert to the history table
        INSERTHISTORY(table);

        for (i = 0; i < GetSelectCols() ; i ++) {
          // if not  historyID from object table do not save
          if (IsNotNull(row, i) && strcasecmp("historyID", GetFieldName(i) )
              != 0)
            INTO(GetFieldName(i) );
        }

        VALUE(historyID); // first write  historyID
        for (i = 0; i < GetSelectCols() ; i ++) {
          // save not null value
          if (IsNotNull(row, i) && strcasecmp("historyID", GetFieldName(i) )
              != 0)
            VALUE(GetFieldValue(row, i) );
        }

        if (EXEC() == false) {
          ret = false;
          break;
        } // if error 

      }

      FreeSelect();
    }

  }

  // default 

  return ret;
}

// update object table client of object (registrar) and authInfo 
bool DB::ObjectUpdate(
  int id, int regID, const char *authInfo)
{
  UPDATE("OBJECT");
  SSET("UpDate", "now"); // now 
  SET("UpID", regID);
  SET("AuthInfoPw", authInfo);
  WHEREID(id);
  return EXEC();
}

// UPDATE fce
// SQL UPDATE fce
void DB::UPDATE(
  const char * table)
{
  sqlBuffer = new char[MAX_SQLBUFFER];
  memset(sqlBuffer, 0, MAX_SQLBUFFER);
  SQLCat("UPDATE ");
  SQLCat(table);
  SQLCat(" SET ");
}

void DB::SQLDelEnd()
{
  int len;
  len = strlen(sqlBuffer);
  // del on the end
  if (len > 0)
    sqlBuffer[len-1] = 0;
}

bool DB::SQLTestEnd(
  char c)
{
  int len;
  len = strlen(sqlBuffer);

  // test
  if (sqlBuffer[len-1] == c)
    return true;
  else
    return false;
}

void DB::SQLCat(
  const char *str)
{
  int len, length;
  len = strlen(str);
  length = strlen(sqlBuffer);

  //  test for length buffer
  if (len + length < MAX_SQLBUFFER)
    strcat(sqlBuffer, str);
  else
    // if sql buffer would be valid sql query at this place
    // and something fail to append it could have very bad consequences
    throw;
}

void DB::SQLCatLower(
  const char *str)
{
  SQLCatLW(str, false);
}

void DB::SQLCatUpper(
  const char *str)
{
  SQLCatLW(str, true);
}

// convert to lower or upper 
void DB::SQLCatLW(
  const char *str, bool lw)
{
  int len, length, i;
  len = strlen(str);
  length = strlen(sqlBuffer);

  //  test the lenght
  if (len + length < MAX_SQLBUFFER) {
    for (i = 0; i < len; i ++) {
      if (lw)
        sqlBuffer[length+i] = toupper(str[i]);
      else
        sqlBuffer[length+i] = tolower(str[i]);
    }
    // end of buffer
    sqlBuffer[length+len] = 0;
  }

  LOG( SQL_LOG , "DB::SQLCatLW lw %d str[%s] sqlBuffer[%s]" , lw , str , sqlBuffer );

}

// escape string 
void DB::SQLCatEscape(
  const char * value)
{
  int length;
  char *str;

  length = strlen(value);

  if (length) {
    // increase buffer for escape string 
    // LOG( SQL_LOG , "alloc escape string length  %d" , length*2 );
    str = new char[length*2];
    // DB escape funkce
    Escape(str, value, length) ;
    SQLCat(str);
    // LOG( SQL_LOG , "free  escape string" );
    delete[] str;
  }

}

// set currency
void DB::SETPRICE(
  const char *fname, long price)
{
  char priceStr[16];

  sprintf(priceStr, "%ld.%02ld", price /100, price %100);
  SETS(fname, priceStr, false); //without ESC
}

// without escape
void DB::SSET(
  const char *fname, const char * value)
{
  SETS(fname, value, false);
}
// with escape 
void DB::SET(
  const char *fname, const char * value)
{
  SETS(fname, value, true);
}

void DB::SETS(
  const char *fname, const char * value, bool esc)
{

  if (strlen(value) ) {

    SQLCat(" ");
    SQLCat(fname);
    SQLCat("=");
    // ERASE meanwhile it use also hack at sign \b
    //   if( strcmp( value  , NULL_STRING ) ==  0 || value[0] == 0x8 ) // set up NULL value for NULL string from IDL

    // if( null )
    // NULL back based on backslash 
    if (value[0] == 0x8) // meanwhile use with mod_eppd hack at sign \b
    {
      SQLCat("NULL");
    } else {
      SQLCat("'");
      if (esc)
        SQLCatEscape(value);
      else
        SQLCat(value);
      SQLCat("'");
    }

    SQLCat(" ,"); // comma on the end
  }

}

void DB::SET(
  const char *fname, long value)
{
  char numStr[100];

  SQLCat("  ");
  SQLCat(fname);
  SQLCat("=");
  sprintf(numStr, "%ld", value);
  SQLCat(numStr);
  SQLCat(" ,");
}

void DB::SET(
  const char *fname, int value)
{
  char numStr[16];

  SQLCat("  ");
  SQLCat(fname);
  SQLCat("=");
  sprintf(numStr, "%d", value);
  SQLCat(numStr);
  SQLCat(" ,");
}

void DB::SETNULL(
  const char *fname)
{
  SQLCat("  ");
  SQLCat(fname);
  SQLCat("=NULL");
  SQLCat(" ,");
}

void DB::SET(
  const char *fname, bool value)
{

  SQLCat("  ");
  SQLCat(fname);
  SQLCat("=");

  if (value)
    SQLCat("\'t\'");
  else
    SQLCat("\'f\'");

  SQLCat(" ,");
}

// set  t or f 
void DB::SETBOOL(
  const char *fname, char c)
{
  // one as  true zero like false  false -1 not change 
  if (c == 't' || c == 'f' || c == 'T' || c == 'F') {
    SQLCat("  ");
    SQLCat(fname);
    SQLCat("=");

    if (c == 't' || c == 'T')
      SQLCat("\'t\'");
    else
      SQLCat("\'f\'");

    SQLCat(" ,");
  }

}

void DB::WHERE(
  const char *fname, const char * value)
{
  if (SQLTestEnd( ',') || SQLTestEnd( ';') ) {
    SQLDelEnd(); //  delete last char 

    SQLCat("  WHERE ");
    SQLCat(fname);
    SQLCat("='");
    SQLCatEscape(value);
    SQLCat("' ;");
  } else
    sqlBuffer[0] =0; // empty  SQL buffer
}

void DB::OPERATOR(
  const char *op)
{
  if (SQLTestEnd( ',') || SQLTestEnd( ';') ) {
    SQLDelEnd(); //   delete last char 
    SQLCat("  ");
    SQLCat(op); // op  AND OR LIKE
    SQLCat(" ");
  } else
    sqlBuffer[0] =0; // empty  SQL buffer

}

void DB::WHEREOPP(
  const char *op, const char *fname, const char *p, const char * value)
{
  if (SQLTestEnd( ',') || SQLTestEnd( ';') )
    SQLDelEnd();

  SQLCat("  ");
  SQLCat(op); // op AND OR LIKE
  SQLCat(" ");
  SQLCat(fname);
  SQLCat(p);
  SQLCat("'");
  SQLCatEscape(value);
  SQLCat("' ;"); // end

}

void DB::WHERE(
  const char *fname, int value)
{
  char numStr[16];
  sprintf(numStr, "%d", value);
  WHERE(fname, numStr);
}

// INSERT INTO history 
void DB::INSERTHISTORY(
  const char * table)
{
  sqlBuffer = new char[MAX_SQLBUFFER];
  memset(sqlBuffer, 0, MAX_SQLBUFFER);
  SQLCat("INSERT INTO ");
  SQLCat(table);
  SQLCat("_history ");
  INTO("HISTORYID");
}

// INSERT INTO function

void DB::INSERT(
  const char * table)
{
  sqlBuffer = new char[MAX_SQLBUFFER];
  memset(sqlBuffer, 0, MAX_SQLBUFFER);
  SQLCat("INSERT INTO ");
  SQLCat(table);
  SQLCat("  ");
}

void DB::INTO(
  const char *fname)
{

  // begin by (
  if (SQLTestEnd(' ') )
    SQLCat(" ( ");

  SQLCat(fname);
  SQLCat(" ,");

}

void DB::INTOVAL(
  const char *fname, const char * value)
{
  // if some value
  if (strlen(value) )
    INTO(fname);
}

void DB::VAL(
  const char * value)
{
  if (strlen(value) )
    VALUE(value);
}

void DB::VALUEUPPER(
  const char * value)
{
  VALUES(value, false, true, 1);
}

void DB::VALUELOWER(
  const char * value)
{
  VALUES(value, false, true, -1);
}

void DB::VALUES(
  const char * value, bool esc, bool amp, int uplo)
{
  int len;

  len = strlen(sqlBuffer);

  if (SQLTestEnd( ';') ) // end
  {
    SQLDelEnd();
    SQLDelEnd();
    SQLCat(",");
  } else {

    if (SQLTestEnd( ',') ) {
      SQLDelEnd();
      SQLCat(" ) "); // ont the end  )
    }

    SQLCat(" VALUES ( ");

  }

  if (amp)
    SQLCat(" '");
  else
    SQLCat(" ");

  // escape
  if (esc)
    SQLCatEscape(value);
  else {

    if (uplo == 0)
      SQLCat(value);
    else if (uplo > 0)
      SQLCatUpper(value);
    else
      SQLCatLower(value);
  }

  if (amp)
    SQLCat("'");
  strcat(sqlBuffer, " );"); // vmake on the end

}

// set current_timestamp
void DB::VALUENOW()
{
  VALUES("current_timestamp", false, false, 0);
}

// set null value
void DB::VALUENULL()
{
  VALUES("NULL", false, false, 0);
}
// actual timestamp plus period in months
void DB::VALUEPERIOD(
  int period)
{
  char str[80];

  sprintf(str, "current_timestamp + interval\'%d month\' ", period);
  VALUES(str, false, false, 0);
}

void DB::VVALUE(
  const char * value)
{
  // do not use ESCAPE
  VALUES(value, false, true, 0);
}

void DB::VALUE(
  const char * value)
{
  // use ESCAPE 
  VALUES(value, true, true, 0);
}

void DB::VALUE(
  int value)
{
  char numStr[16];
  sprintf(numStr, "%d", value);
  VALUES(numStr, false, false, 0); // without ESC
}

void DB::VALUE(
  long value)
{
  char numStr[100];
  sprintf(numStr, "%ld", value);
  VALUES(numStr, false, false, 0); // without ESC
}

void DB::VALUE(
  unsigned long long value)
{
  char numStr[100];
  sprintf(numStr, "%llu", value);
  VALUES(numStr, false, false, 0); // without ESC
}

void DB::VALUE(
  bool value)
{
  // without ESC
  if (value)
    VALUES("t", false, true, 0);
  else
    VALUES("f", false, true, 0);
}

void DB::VALPRICE(
  long price)
{
  char priceStr[16];
  // currency in penny 
  sprintf(priceStr, "%ld.%02ld", price /100, price %100);
  VALUES(priceStr, false, false, 0); // without ESC
}
bool DB::EXEC()
{
  bool ret;
  // run SQL 
  ret = ExecSQL(sqlBuffer);

  // free mem 
  delete[] sqlBuffer;
  // return if success of failed
  return ret;
}

// select functions 
bool DB::SELECT()
{
  bool ret;

  // run SQL query
  ret = ExecSelect(sqlBuffer);

  // free mem 
  delete[] sqlBuffer;
  //  return if success of failed
  return ret;
}

// SQL SELECT functions 
void DB::SELECTFROM(
  const char *fname, const char * table)
{
  sqlBuffer = new char[MAX_SQLBUFFER];
  memset(sqlBuffer, 0, MAX_SQLBUFFER);
  SQLCat("SELECT ");
  if (strlen(fname) )
    SQLCat(fname);
  else
    SQLCat(" * ") ;
  SQLCat(" FROM ");
  SQLCat(table);
  SQLCat(" ;"); // end
}

bool DB::SELECTONE(
  const char * table, const char *fname, const char *value)
{
  LOG( SQL_LOG , "SELECTONE  table %s fname %s values %s" , table , fname , value );
  SELECTFROM("", table);
  WHERE(fname, value);
  return SELECT();
}

bool DB::SELECTONE(
  const char * table, const char *fname, int value)
{
  SELECTFROM("", table);
  WHERE(fname, value);
  return SELECT();
}

// special select to the object
bool DB::SELECTOBJECTID(
  const char *table, const char *fname, int id)
{
  char numStr[16];

  sqlBuffer = new char[MAX_SQLBUFFER];
  memset(sqlBuffer, 0, MAX_SQLBUFFER);
  SQLCat("SELECT * ");
  SQLCat(" FROM ");
  SQLCat(" object_registry ,  object  , ");
  SQLCat(table); // table
  SQLCat(" WHERE Object.id= object_registry.id");
  SQLCat(" AND Object.id");
  SQLCat("=");
  sprintf(numStr, "%d", id);
  SQLCat(numStr);
  SQLCat(" AND ");
  SQLCat("Object.id=");
  SQLCat(table);
  SQLCat(".id");
  SQLCat(" ;"); // end
  return SELECT();
}

// for contact mam table get handles of admin-c or tech-c
bool DB::SELECTCONTACTMAP(
  char *map, int id, unsigned role)
{
  std::stringstream sql;
  sql << "SELECT name  FROM object_registry o, " << map
      << "_contact_map c WHERE c.contactid=o.id AND c." << map << "id=" << id;
  if (role)
    sql << " AND role=" << role;
  return ExecSelect(sql.str().c_str());
}

