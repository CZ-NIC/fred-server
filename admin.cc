#include "admin.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"

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
char sqlString[128];
bool find=false;

reg = new ccReg::Registrar ;

 if( DBsql.OpenDatabase( database.c_str() ) )
  { 
    sprintf( sqlString , "SELECT * FROM REGISTRAR WHERE handle=\'%s\';" , handle );
    LOG( NOTICE_LOG, "getRegistrar: num -> %s", handle );

   if( DBsql.ExecSelect( sqlString ) )
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


ccReg::RegObjectType ccReg_Admin_i::getRegObjectType(const char* objectName)
{
DB DBsql;
char sqlString[128];
int zone , id ;

 if( DBsql.OpenDatabase( database.c_str() ) )
  {
  if( ( id =  DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", (char *) objectName ) )  > 0  )
    {
      zone =  DBsql.GetNumericFromTable( "DOMAIN", "zone" , "id" ,  id ) ;
      switch( zone )
            {
               case ZONE_CZ:
                              return ccReg::CZ_DOMAIN;
               case ZONE_ENUM:
                              return ccReg::ENUM_DOMAIN;
               
            }      
    
    }
   else
   {
   if(  DBsql.GetNumericFromTable( "CONTACT", "id", "handle", (char *) objectName ) ) return ccReg::CONTACT_HANDLE;
   else 
        if(  DBsql.GetNumericFromTable( "NSSET", "id", "handle", (char *) objectName ) ) return ccReg::NSSET_HANDLE;
   }

    DBsql.Disconnect();  
 }


// deafult
return  ccReg::NONE;
}
 
