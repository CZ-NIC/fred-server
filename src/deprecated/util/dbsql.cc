/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/util.hh"
#include "src/util/db/manager_tss.hh"
#include "util/log/logger.hh"
#include "src/bin/corba/epp/action.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <math.h>
#include <sstream>

#include <boost/date_time/gregorian/gregorian.hpp>

// for invoice  type
#define INVOICE_FA  1 // normal invoice
#define INVOICE_ZAL 0 // advance invoice

DB::DB()
    : memHandle(NULL),
      svrTRID(NULL),
      sqlBuffer(NULL),
      historyID(0),
      loginID(0),
      enum_action(0)
{
}

/* HACK! HACK! HACK! */
DB::DB(Database::Connection& _conn)
    : PQ(Database::TerribleHack::get_internal_psql_connection(_conn)),
      memHandle(NULL),
      svrTRID(NULL),
      sqlBuffer(NULL),
      historyID(0),
      loginID(0),
      enum_action(0)
{
}

// free memory buffers
DB::~DB()
{
    if (sqlBuffer != NULL)
    {
        try
        {
            LOG<Logging::Log::Severity::notice >( "delete sqlBuffer");
            delete[] sqlBuffer;
        }
        catch (...) { }
    }

    if (svrTRID != NULL)
    {
        try
        {
            LOG<Logging::Log::Severity::notice >( "delete svrTRID");
            delete[] svrTRID;
        }
        catch (...) { }
    }

    if (memHandle != NULL)
    {
        try
        {
            LOG<Logging::Log::Severity::notice >( "delete memHandle");
            delete[] memHandle;
        }
        catch (...) { }
    }
}

// action
bool DB::BeginAction(
  unsigned long long clientID, int action, const char *clTRID, const char *xml,
  unsigned long long requestID
)
{
  loginID = clientID; // id of corba client
  historyID = 0; // history ID

  if (svrTRID==NULL) {
      svrTRID= new char[MAX_SVTID];
      strncpy(svrTRID,
          Util::make_svtrid(requestID).c_str(),
          MAX_SVTID - 1
      );

      LOG<Logging::Log::Severity::debug>( "Make svrTRID: %s" , svrTRID );
  }

  enum_action = action;
  return true;
}

// end of EPP operation
const char * DB::EndAction(
  int response)
{
    LOG<Logging::Log::Severity::debug>( "EndAction svrTRID: %s" , svrTRID );
    return svrTRID;

}

const char * DB::GetObjectCrDateTime(
  int id)
{
  convert_rfc3339_timestamp(dtStr, MAX_DATE+1, GetValueFromTable("OBJECT_registry",
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
  convert_rfc3339_timestamp(dtStr, MAX_DATE+1, GetFieldValueName( (char * ) fname , row) ) ;
  return dtStr;
}

char * DB::GetFieldDateValueName(
  const char *fname, int row)
{
  convert_rfc3339_date(dtStr, MAX_DATE+1, GetFieldValueName( (char * ) fname , row) ) ;
  return dtStr;
}

// get number of dns host  associated to nsset
int DB::GetNSSetHosts(
  int nssetID)
{
  char sqlString[128];
  int num=0;

  snprintf(sqlString, sizeof(sqlString), "SELECT id FROM host  WHERE nssetID=%d;", nssetID);

  if (ExecSelect(sqlString) ) {
    num = GetSelectRows();
    LOG<Logging::Log::Severity::debug>( "nsset %d num %d" , nssetID , num );
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

    snprintf(sqlString, sizeof(sqlString), "SELECT id FROM dsrecord WHERE keysetid=%d;", keysetID);

    if (ExecSelect(sqlString)) {
        num = GetSelectRows();
        LOG<Logging::Log::Severity::debug>( "keyset id(%d) has %d dsrecord(s)", keysetID, num);
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
        << " AND digest='" << Escape2(digest) << "'";
    if (maxSigLife != -1)
        query << " AND maxsiglife=" << maxSigLife;

    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG<Logging::Log::Severity::debug>( "Found dsrecord id(%d) with same values", id);
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
        << " AND digest='" << Escape2(digest) << "'";
    if (maxSigLife != -1)
        query << " AND maxsiglife=" << maxSigLife;

    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG<Logging::Log::Severity::debug>( "Found dsrecord id(%d) with same values", id);
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
        LOG<Logging::Log::Severity::debug>( "Keyset id(%d) has %d dnskey(s)",
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
        << " AND key='" << Escape2(key) << "'";
    if (ExecSelect(query.str().c_str())) {
        if (GetSelectRows() > 0) {
            id = atoi(GetFieldValue(0, 0));
            if (id != 0) {
                LOG<Logging::Log::Severity::debug>( "Found dnskey id(%d) with same values", id);
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
        << " AND key='" << Escape2(key) << "'";
    if (ExecSelect(query.str().c_str())) {
        id = atoi(GetFieldValue(0, 0));
        if (id != 0) {
            LOG<Logging::Log::Severity::debug>( "Found dnskey id(%d) with same values", id);
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
    snprintf(sqlString, sizeof(sqlString), "SELECT id FROM  object WHERE id=%d and clID=%d ", id,
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

  snprintf( sqlString, sizeof(sqlString),
      "SELECT object.id FROM object_registry , object WHERE object_registry.type=%d AND object_registry.id=object.id AND object_registry.name=\'%s\';",
      type, name);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      id = atoi(GetFieldValue( 0, 0) );
      LOG<Logging::Log::Severity::debug>( "GetObjectID   name=\'%s\'  -> ID %d" , name , id );
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

  snprintf(sqlString, sizeof(sqlString), "SELECT * FROM nsset_contact_map  WHERE nssetID=%d;",
      nssetID);

  if (ExecSelect(sqlString) ) {
    num = GetSelectRows();
    LOG<Logging::Log::Severity::debug>( " nsset_contact_map  num %d" , num );
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
    snprintf(sqlString, sizeof(sqlString), "SELECT * FROM keyset_contact_map WHERE keysetid=%d;",
            keysetid);

    if (ExecSelect(sqlString)) {
        num = GetSelectRows();
        LOG<Logging::Log::Severity::debug>( " keyset_contact_map num %d", num);
        FreeSelect();
    }

    return num;
}

// save object as deleted
bool DB::SaveObjectDelete(
  int id)
{
  LOG<Logging::Log::Severity::debug>( "set delete objectID %d" , id );
  UPDATE("object_registry");
  SET("ErDate", "now");
  WHEREID(id);
  return EXEC();
}

bool DB::SaveObjectCreate(
  int id)
{
  LOG<Logging::Log::Severity::debug>( "set create histyoryID for object ID %d historyID %d" , id , historyID);
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
   snprintf( sqlString , "SELECT count( id ) FROM object_delete  WHERE name ILIKE \'%s\' and  deltime  > current_timestamp - interval\'%d days\';"  , name , days );

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

  snprintf(sqlString, sizeof(sqlString), "SELECT system FROM registrar where id=%d;", regID);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() == 1) {
      ret = GetFieldBooleanValueName("system", 0);

    }
    FreeSelect();
  }

  if (ret)
    LOG<Logging::Log::Severity::debug>( "GetRegistrarSystem TRUE" );
  else
    LOG<Logging::Log::Severity::debug>( "GetRegistrarSystem FALSE");

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

  std::string today = boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day());

  snprintf( sqlString, sizeof(sqlString),
      "SELECT  id  FROM  registrarinvoice  WHERE registrarid=%d and zone=%d and fromdate <= '%s' and (todate >= '%s' or todate is null);",
      regID, zone, today.c_str(), today.c_str());

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

  snprintf(sqlString, sizeof(sqlString), "INSERT INTO %s_contact_map VALUES ( %d , %d );", table,
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
  get_rfc3339_timestamp(time(NULL) , currentDate, MAX_DATE+1, true);

  if (id) // if ValExDate already exist and updated
  {
    // copy current Exdate during update
    strncpy(exDate, GetDomainValExDate(id), MAX_DATE) ;
    exDate[MAX_DATE] = '\0';

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
      "SELECT  ExDate+interval\'%d month\' FROM DOMAIN WHERE id=%d AND ExDate+interval\'%d month\' <= current_date+interval\'%d month\';",
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
  int hostID=0;
  std::stringstream sql;

  sql << "SELECT id FROM HOST WHERE fqdn = '" << Escape2(fqdn) << "' AND nssetid = " << nssetID <<  ";";

  if (ExecSelect(sql.str().c_str()) ) {
    if (GetSelectRows() == 1) {
      hostID = atoi(GetFieldValue( 0, 0) );
      LOG<Logging::Log::Severity::debug>( "CheckHost fqdn=\'%s\' nssetid=%d  -> hostID %d" , fqdn , nssetID , hostID );
    }

    FreeSelect();
  }

  if (hostID == 0)
    LOG<Logging::Log::Severity::debug>( "Host fqdn=\'%s\' not found" , fqdn );
  return hostID;
}

// test for nsset is linked to domain
bool DB::TestNSSetRelations(
  int id)
{
  bool ret = false;
  char sqlString[128];

  snprintf(sqlString, sizeof(sqlString), "SELECT id from DOMAIN WHERE nsset=%d;", id);
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

    snprintf(sqlString, sizeof(sqlString), "SELECT id FROM DOMAIN WHERE keyset=%d;", id);
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

  snprintf(sqlString, sizeof(sqlString), "SELECT count(id) from DOMAIN WHERE Registrant=%d;", id);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) );
    FreeSelect();
  }

  if (count > 0)
    return true;

  snprintf(sqlString, sizeof(sqlString),
      "SELECT count(nssetID ) from NSSET_CONTACT_MAP WHERE contactid=%d;", id);
  if (ExecSelect(sqlString) ) {
    count = atoi(GetFieldValue( 0, 0) );
    FreeSelect();
  }

  if (count > 0)
    return true;

  snprintf(sqlString, sizeof(sqlString),
          "SELECT count(keysetID) from KEYSET_CONTACT_MAP WHERE contactid=%d;", id);
  if (ExecSelect(sqlString)) {
      count = atoi(GetFieldValue(0, 0));
      FreeSelect();
  }

  if (count > 0)
    return true;

  snprintf(sqlString, sizeof(sqlString),
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

  snprintf(sqlString, sizeof(sqlString), "SELECT authinfopw from %s WHERE id=%d", table, id);

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



// vreturn id of the owner of domain
int DB::GetClientDomainRegistrant(
  int clID, int contactID)
{
  int regID=0;
  char sqlString[128];

  snprintf(sqlString, sizeof(sqlString),
      "SELECT  clID FROM DOMAIN WHERE Registrant=%d AND clID=%d", contactID,
      clID);
  if (ExecSelect(sqlString) ) {
    if (GetSelectRows() > 0) {
      regID = atoi(GetFieldValue( 0, 0) );
      LOG<Logging::Log::Severity::debug>( "Get ClientDomainRegistrant  contactID \'%d\' -> regID %d" , contactID , regID );
      FreeSelect();
    }
  }

  return regID;
}
// function for get one value from table
const char * DB::GetValueFromTable(
  const char *table, const char *vname, const char *fname, const char *value)
{
  int size;

  // snprintf(sqlString, sizeof(sqlString), "SELECT  %s FROM %s WHERE %s=\'%s\';", vname, table,
  //     fname, value);

  std::stringstream query;
  query << "SELECT " << vname << " FROM " << table
        << " WHERE " << fname << " = '" << Escape2(value) << "'";

  if (ExecSelect(query.str().c_str()) ) {
    if (GetSelectRows() == 1) // if selected only one record
    {
      size = GetValueLength( 0, 0);

      if (memHandle) {
        delete[] memHandle;
        LOG<Logging::Log::Severity::debug>( "re-alloc memHandle");
        memHandle = new char[size+1];
      } else {
        LOG<Logging::Log::Severity::debug>( "alloc memHandle");
        memHandle = new char[size+1];
      }

      strncpy(memHandle, GetFieldValue( 0, 0), size + 1);
      memHandle[size] = '\0';
      LOG<Logging::Log::Severity::debug>( "GetValueFromTable \'%s\' field %s  value  %s ->  %s" , table , fname , value , memHandle );
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

  snprintf(value, sizeof(value), "%d", numeric);

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

  snprintf(value, sizeof(value), "%d", numeric);

  return GetNumericFromTable(table, vname, fname, value);
}

// remove date from table
bool DB::DeleteFromTable(
  const char *table, const char *fname, int id)
{
  char sqlString[128];

  LOG<Logging::Log::Severity::debug>( "DeleteFromTable %s fname %s id -> %d" , table , fname , id );

  snprintf(sqlString, sizeof(sqlString), "DELETE FROM %s  WHERE %s=%d;", table, fname, id);
  return ExecSQL(sqlString);
}

// remove date from  contact map table
bool DB::DeleteFromTableMap(
  const char *map, int id, int contactid)
{
  char sqlString[128];

  LOG<Logging::Log::Severity::debug>( "DeleteFrom  %s_contact_map  id  %d contactID %d" , map ,id , contactid );

  snprintf(sqlString, sizeof(sqlString),
      "DELETE FROM %s_contact_map WHERE  %sid=%d AND contactid=%d;", map, map,
      id, contactid);

  return ExecSQL(sqlString);
}

int DB::GetSequenceID(
  const char *sequence)
{
  char sqlString[128];
  int id=0;

  snprintf(sqlString, sizeof(sqlString), "SELECT  NEXTVAL( \'%s_id_seq\'  );", sequence);

  if (ExecSelect(sqlString) ) {
    id = atoi(GetFieldValue( 0, 0) );
    LOG<Logging::Log::Severity::debug>( "GetSequence \'%s\' -> ID %d" , sequence , id );
    FreeSelect();
  }

  return id;
}

bool DB::SaveNSSetHistory(
  int id, unsigned long long request_id)
{

  //  save to history
  if (MakeHistory(id, request_id) ) {

    if (SaveHistory("NSSET", "id", id) )
      if (SaveHistory("HOST", "nssetid", id) )
        if (SaveHistory("HOST_IPADDR_map", "nssetid", id) )
          if (SaveHistory("nsset_contact_map", "nssetid", id) ) // history of tech-c
            return true;
  }

  return 0;
}

bool
DB::SaveKeySetHistory(int id, unsigned long long request_id)
{
    // save to history
    if (MakeHistory(id, request_id))
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
  int id, unsigned long long request_id)
{

  if (MakeHistory(id, request_id) ) {
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
  if (DeleteFromTable("domain_contact_map", "domainID", id) ) { // admin-c
    if (DeleteFromTable("enumval", "domainID", id) ) { // enumval extension
      if (DeleteFromTable("DOMAIN", "id", id) ) {
        if (DeleteFromTable("OBJECT", "id", id) ) {
          return true;
        }
      }
    }
  }
  return false;
}

bool DB::SaveContactHistory(
  int id, unsigned long long request_id)
{

  if (MakeHistory(id, request_id) ) {
    if (SaveHistory("Contact", "id", id) ) {
      if (SaveHistory("contact_address", "contactid", id)) {
        return true;
      }
    }
  }

  return false;
}

bool DB::DeleteContactObject(
  int id)
{
  if (DeleteFromTable("contact_address", "contactid", id)) {
    if (DeleteFromTable("CONTACT", "id", id) ) {
      if (DeleteFromTable("OBJECT", "id", id) ) {
        return true;
      }
    }
  }

  return false;
}

int DB::MakeHistory(
  int objectID, unsigned long long requestID) // write records of object to the history
{
  char sqlString[128];

    LOG<Logging::Log::Severity::debug>( "MakeHistory requestID -> %llu " ,
            requestID);
    historyID = GetSequenceID("HISTORY");
    if (historyID) {
      LOG<Logging::Log::Severity::debug>( "MakeHistory requestID -> %llu " , requestID);
      snprintf(sqlString, sizeof(sqlString),
          "INSERT INTO HISTORY ( id , request_id ) VALUES ( %d  , %llu );",
          historyID, requestID);
      if (ExecSQL(sqlString) ) {
        if (SaveHistory("OBJECT", "id", objectID) ) // save object table to history
        {
          LOG<Logging::Log::Severity::debug>( "Update objectID  %d -> historyID %d " , objectID , historyID );
          snprintf(sqlString, sizeof(sqlString),
              "UPDATE OBJECT_registry set historyID=%d WHERE id=%d;",
              historyID, objectID);
          if (ExecSQL(sqlString) )
            return historyID;
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
    LOG<Logging::Log::Severity::debug>( "SaveHistory historyID - > %d" , historyID );
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
    strncat(sqlBuffer, str, MAX_SQLBUFFER - len - 1);
  else
    // if sql buffer would be valid sql query at this place
    // and something fail to append it could have very bad consequences
    throw std::runtime_error("DB::SQLCat: string too long.");
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

  LOG<Logging::Log::Severity::debug>( "DB::SQLCatLW lw %d str[%s] sqlBuffer[%s]" , lw , str , sqlBuffer );

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
    // LOG<Logging::Log::Severity::debug>( "alloc escape string length  %d" , length*2 );
    str = new char[length*2];
    // DB escape funkce
    Escape(str, value, length) ;
    SQLCat(str);
    // LOG<Logging::Log::Severity::debug>( "free  escape string" );
    delete[] str;
  }

}

// set currency
void DB::SETPRICE(
  const char *fname, long price)
{
  char priceStr[16];

  snprintf(priceStr, sizeof(priceStr), "%ld.%02ld", price /100, price %100);
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
  snprintf(numStr, sizeof(numStr), "%ld", value);
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
  snprintf(numStr, sizeof(numStr), "%d", value);
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
  if (SQLTestEnd(',') || SQLTestEnd(';'))
  {
    SQLDelEnd(); //  delete last char

    SQLCat(" WHERE ");
    SQLCat(fname);
    SQLCat("='");
    SQLCatEscape(value);
    SQLCat("' ;");
  }
  else
  {
    sqlBuffer[0] = '\0'; // empty  SQL buffer
  }
}

void DB::OPERATOR(
  const char *op)
{
  if (SQLTestEnd(',') || SQLTestEnd(';'))
  {
    SQLDelEnd(); //   delete last char
    SQLCat("  ");
    SQLCat(op); // op  AND OR LIKE
    SQLCat(" ");
  }
  else
  {
    sqlBuffer[0] = '\0'; // empty  SQL buffer
  }

}

void DB::WHEREOPP(
  const char *op, const char *fname, const char *p, const char * value)
{
  if (SQLTestEnd(',') || SQLTestEnd(';'))
  {
    SQLDelEnd();
  }
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
  snprintf(numStr, sizeof(numStr), "%d", value);
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
  strncat(sqlBuffer, " );", MAX_SQLBUFFER - len - 1); // vmake on the end

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

  snprintf(str, sizeof(str), "current_timestamp + interval\'%d month\' ", period);
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
  snprintf(numStr, sizeof(numStr), "%d", value);
  VALUES(numStr, false, false, 0); // without ESC
}

void DB::VALUE(
  long value)
{
  char numStr[100];
  snprintf(numStr, sizeof(numStr), "%ld", value);
  VALUES(numStr, false, false, 0); // without ESC
}

void DB::VALUE(
  unsigned long long value)
{
  char numStr[100];
  snprintf(numStr, sizeof(numStr), "%llu", value);
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
  snprintf(priceStr, sizeof(priceStr), "%ld.%02ld", price /100, price %100);
  VALUES(priceStr, false, false, 0); // without ESC
}
bool DB::EXEC()
{
  bool ret;
  // run SQL
  ret = ExecSQL(sqlBuffer);

  // free mem
  delete[] sqlBuffer;
  sqlBuffer = NULL;
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
  sqlBuffer = NULL;
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
  LOG<Logging::Log::Severity::debug>( "SELECTONE  table %s fname %s values %s" , table , fname , value );
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
  snprintf(numStr, sizeof(numStr), "%d", id);
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

