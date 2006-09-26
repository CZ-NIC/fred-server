#include "admin.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"
#include "register/register.h"


ccReg_Admin_i::ccReg_Admin_i(const std::string _database) : database(_database)
{}

ccReg_Admin_i::~ccReg_Admin_i() 
{}

ccReg::RegistrarList* ccReg_Admin_i::getRegistrars()
{
int rows = 0 , i ;
DB DBsql;
ccReg::RegistrarList *reglist;

reglist = new ccReg::RegistrarList;

 if( DBsql.OpenDatabase( database.c_str() ) )
  {
   if( DBsql.ExecSelect( "SELECT * FROM REGISTRAR;"  ) )
     {
       rows = DBsql.GetSelectRows();
       LOG( NOTICE_LOG, "getRegistrars: num -> %d",  rows );
       reglist->length( rows );
       for( i = 0 ; i < rows ; i ++ )
          {    
             //  *(reglist[i]) = new ccReg::Registrar;   
             (*reglist)[i].id = atoi( DBsql.GetFieldValueName("ID" , i ) );
             (*reglist)[i].handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , i ) ); // handle
             (*reglist)[i].name=CORBA::string_dup( DBsql.GetFieldValueName("name" , i ) ); 
             (*reglist)[i].organization=CORBA::string_dup( DBsql.GetFieldValueName("organization" , i ) ); 
             (*reglist)[i].street1=CORBA::string_dup( DBsql.GetFieldValueName("street1" , i ) );
             (*reglist)[i].street2=CORBA::string_dup( DBsql.GetFieldValueName("street2" , i ) );
             (*reglist)[i].street3=CORBA::string_dup( DBsql.GetFieldValueName("street3" , i ) );
             (*reglist)[i].city=CORBA::string_dup( DBsql.GetFieldValueName("city" , i ) );
             (*reglist)[i].stateorprovince=CORBA::string_dup( DBsql.GetFieldValueName("stateorprovince" , i ) );
             (*reglist)[i].postalcode=CORBA::string_dup( DBsql.GetFieldValueName("postalcode" , i ) );
             (*reglist)[i].country=CORBA::string_dup( DBsql.GetFieldValueName("country" , i ) );
             (*reglist)[i].telephone=CORBA::string_dup( DBsql.GetFieldValueName("telephone" , i ) );
             (*reglist)[i].fax=CORBA::string_dup( DBsql.GetFieldValueName("fax" , i ) );
             (*reglist)[i].email=CORBA::string_dup( DBsql.GetFieldValueName("email" , i ) );
             (*reglist)[i].url=CORBA::string_dup( DBsql.GetFieldValueName("url" , i ) );
             (*reglist)[i].credit=get_price( DBsql.GetFieldValueName("credit" , i ) );
  
          }

       DBsql.FreeSelect();
     }

    DBsql.Disconnect();
  }

if( rows == 0 ) reglist->length( 0 ); // nulova delka

return  reglist;
}

ccReg::Registrar* ccReg_Admin_i::getRegistrarByHandle(const char* handle)
  throw (ccReg::Admin::ObjectNotFound)
{
DB DBsql;
ccReg::Registrar *reg;
bool find=false;

reg = new ccReg::Registrar ;

 if( DBsql.OpenDatabase( database.c_str() ) )
  { 
    LOG( NOTICE_LOG, "getRegistrByHandle: handle -> %s", handle );

   if( DBsql.SELECTONE( "REGISTRAR" , "handle" , handle  ) )
   {
     if( DBsql.GetSelectRows() != 1 ) throw ccReg::Admin::ObjectNotFound();
     else
         {
 
             reg->id =  atoi( DBsql.GetFieldValueName("ID" , 0 ) );
             reg->handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , 0 ) ); // handle
             reg->name=CORBA::string_dup( DBsql.GetFieldValueName("name" , 0 ) );
             reg->organization=CORBA::string_dup( DBsql.GetFieldValueName("organization" , 0 ) );
             reg->street1=CORBA::string_dup( DBsql.GetFieldValueName("street1" , 0 ) );
             reg->street2=CORBA::string_dup( DBsql.GetFieldValueName("street2" , 0 ) );
             reg->street3=CORBA::string_dup( DBsql.GetFieldValueName("street3" , 0 ) );
             reg->city=CORBA::string_dup( DBsql.GetFieldValueName("city" , 0 ) );
             reg->stateorprovince=CORBA::string_dup( DBsql.GetFieldValueName("stateorprovince" , 0 ) );
             reg->postalcode=CORBA::string_dup( DBsql.GetFieldValueName("postalcode" , 0 ) );
             reg->country=CORBA::string_dup( DBsql.GetFieldValueName("country" , 0 ) );
             reg->telephone=CORBA::string_dup( DBsql.GetFieldValueName("telephone" , 0 ) );
             reg->fax=CORBA::string_dup( DBsql.GetFieldValueName("fax" , 0 ) );
             reg->email=CORBA::string_dup( DBsql.GetFieldValueName("email" , 0 ) );
             reg->url=CORBA::string_dup( DBsql.GetFieldValueName("url" , 0 ) );
             reg->credit=get_price( DBsql.GetFieldValueName("credit" , 0 ) );
            find = true;
         }
     
    DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  }

 
if( find == false )
{
reg->id =  0;
reg->handle=CORBA::string_dup( ""  ); // handle
reg->name=CORBA::string_dup( ""  );
reg->organization=CORBA::string_dup( "" );
reg->street1=CORBA::string_dup( "" );
reg->street2=CORBA::string_dup( "" );
reg->street3=CORBA::string_dup( "" );
reg->city=CORBA::string_dup( "" );
reg->stateorprovince=CORBA::string_dup( "" );
reg->postalcode=CORBA::string_dup( "" );
reg->country=CORBA::string_dup( "" );
reg->telephone=CORBA::string_dup( "" );
reg->fax=CORBA::string_dup( "" );
reg->email=CORBA::string_dup( "" );
reg->url=CORBA::string_dup( "" );
reg->credit=0;
}

return reg;
}



// primitivni vypis
ccReg::Lists*  ccReg_Admin_i::ObjectList( char* table , char *fname )
{
DB DBsql;
ccReg::Lists *list;
char sqlString[128];
int rows =0, i;

list = new ccReg::Lists;


 if( DBsql.OpenDatabase( database.c_str() ) )
  {
   sprintf( sqlString , "SELECT %s FROM %s;" , fname , table );

   if( DBsql.ExecSelect( sqlString ) )
     {
       rows = DBsql.GetSelectRows();
       LOG( NOTICE_LOG, "List: %s  num -> %d",  table , rows );
       list->length( rows );

       for( i = 0 ; i < rows ; i ++ )
          {
             (*list)[i]=CORBA::string_dup( DBsql.GetFieldValue(  i , 0 )  ); 
          }

      DBsql.FreeSelect();
     }
    DBsql.Disconnect();

  }

if( rows == 0 ) list->length( 0 ); // nulova delka

return list;
}

ccReg::Lists* ccReg_Admin_i::ListRegistrar()
{
return ObjectList( "REGISTRAR" , "handle" );
}
ccReg::Lists* ccReg_Admin_i::ListDomain()
{
return ObjectList( "DOMAIN" , "fqdn" );
}
ccReg::Lists* ccReg_Admin_i::ListContact()
{
return ObjectList( "CONTACT" , "handle" );
}
ccReg::Lists* ccReg_Admin_i::ListNSSet()
{
return ObjectList( "NSSET" , "handle" );
}

#define SWITCH_CONVERT(x) case Register::x : ch->handleClass = ccReg::x; break
void
ccReg_Admin_i::checkHandle(const char* handle, ccReg::CheckHandleType_out ch)
{
  std::auto_ptr<Register::Manager> r(Register::Manager::create(NULL));
  Register::CheckHandle chd;
  r->checkHandle(handle,chd);
  ch = new ccReg::CheckHandleType;
  ch->newHandle = CORBA::string_dup(chd.newHandle.c_str());
  switch (chd.handleClass) {
    SWITCH_CONVERT(CH_ENUM_BAD_ZONE);
    SWITCH_CONVERT(CH_ENUM); 
    SWITCH_CONVERT(CH_DOMAIN_PART); 
    SWITCH_CONVERT(CH_DOMAIN_BAD_ZONE); 
    SWITCH_CONVERT(CH_DOMAIN_LONG); 
    SWITCH_CONVERT(CH_DOMAIN);
    SWITCH_CONVERT(CH_NSSET);
    SWITCH_CONVERT(CH_CONTACT);
    SWITCH_CONVERT(CH_INVALID);
  } 
}
 
char* 
ccReg_Admin_i::login(const char* username, const char* password)
{
  return CORBA::string_dup("XXX");
}

ccReg::Session_ptr 
ccReg_Admin_i::getSession(const char* sessionID)
{
  return NULL;
}
