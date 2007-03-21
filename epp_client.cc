//                      This is the client.118
//
// Usage: client <object reference>
//
#include <fstream.h>
#include <iostream.h>
#include <ccReg.hh>

#include <nameservice.h>

#include <time.h>

//////////////////////////////////////////////////////////////////////



void random_string(char *str , int len)
{
int i , j ;
for( i = 0 ; i < len ; i ++ )
{
     j=1+(int) (25.0*rand()/(RAND_MAX+1.0));
    str[i] = 'a' + j ;
}
 str[i] = 0 ; // ukocit
}

int main(int argc, char** argv)
{
  try {
//     CORBA::String_var clID, pass , cc ; 
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);    
    CORBA::Object_var  epp_obj , admin_obj;
     CORBA::Short errCode ; 
    ccReg::Contact *contact , *cc;
    ccReg::ContactChange *ch;
    ccReg::Response *ret;
//    ccReg::NSSet *nsset;
    ccReg::Domain *domain;
    ccReg::Check_var check;
    ccReg::Check dcheck(2);
    ccReg::ZoneCredit_var credit;
    CORBA::Any an; 
//    CORBA::Short err , sh;
    ccReg::RegistrarList *rl;
    ccReg::Lists *lists ;
    ccReg::Registrar *reg;
    CORBA::String_var errMsg , svTR , mesg ;
    ccReg::Error errors;
//    char *msg;
    unsigned long  i  ,  j , num ;
//    int len , max = 512, d , j , a , num;
   CORBA::Long loginID;
   
    ccReg::CheckResp_var cr;
    ccReg::DNSHost_var dns_chg ,  dns_add , dns_rem;
    ccReg::TechContact_var tech_add , tech_rem;
    ccReg::AdminContact_var admin;
    ccReg::Status_var add , rem;
    CORBA::Short count;
    ccReg::timestamp crDate , qDate , exDate , exvalDate ;
    CORBA::String_var msgID , newmsgID ;
//    ccReg::ENUMValidationExtension *enumVal;
    ccReg::ENUMValidationExtension_var ev;
    ccReg::ExtensionList_var ext;
     ccReg::Period_str period;
//    struct tm dt;
//    time_t t;
//    filebuf *pbuf;
//    char *buffer;
    char name[64] , handle[64] ,  fqdn[64] , email[32] , clTRID[32];
//    long size  ;
    NameService ns(orb);

    epp_obj =  ns.resolve("EPP");

    if (CORBA::is_nil (epp_obj))
      {
        return 1;
      }
    ccReg::EPP_var EPP = ccReg::EPP::_narrow (epp_obj);

/*
    admin_obj =  ns.resolve("Admin");

    if (CORBA::is_nil (admin_obj)) 
      {
        return 1;
      }

    ccReg::Admin_var Admin = ccReg::Admin::_narrow (admin_obj);
*/
    ccReg::timestamp_var ts;
    cerr << "EPP version: "  <<  EPP->version(ts)   << endl;
   

    loginID = 0; 
/*
//    errors = new ccReg::Error_var;
    errors.length(1);
    errors[0].code=ccReg::transfer_op;
    errors[0].value= CORBA::string_dup( "XX" );

    errCode=1000;
    cout << "get Transaction code input " << errCode << endl;
    ret =  EPP->GetTransaction( errCode ,  errors , loginID ,  "client-CLTRID" );
*/
/*
    cout << "get Transaction code " << ret->errCode << ret->errMsg  <<  ret->svTRID  << endl;
    for(  i = 0 ; i < ret->errors.length() ; i ++ )
       cout << "errors code: " <<  ret->errors[i].code << " value: "  <<  ret->errors[i].value << " raeson: "  << ret->errors[i].reason  << endl;
 */
    // vrati kompletni seznam registratoru
/*
    rl = Admin->getRegistrars();



    cerr << "Registrar total:" << rl->length() << endl ;

    // najde jednoho registratora
    for( i = 0 ; i < rl->length() ; i ++ )
      {
 //          cout << "registrar:" << (*rl)[i].id  << (*rl)[i].handle   << (*rl)[i].name << (*rl)[i].organization << (*rl)[i].url << endl ;
 	   reg =  Admin->getRegistrarByHandle(    (*rl)[i].handle  );
             cout << "registrar:" << reg->name << reg->url << endl ; 

      }
*/

/*
    ret =  EPP->ContactInfo( "jouda",  cc , 0 , "jouda_zero" , "<XML></XML>");
    cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

    cout << "info "  << cc->Name  <<  endl;
 
    cout <<  cc->Name << cc->Email << cc->DiscloseEmail <<  cc->CountryCode << endl;
*/


    cout << "loginIDClientLogin " << endl;

    ret =  EPP->ClientLogin(  "REG-LRR"  ,  "123456789"  , "" ,     "LRR-12345" , "" , loginID , "AE:B3:5F:FA:38:80:DB:37:53:6A:3E:D4:55:E2:91:97" ,  ccReg::EN  );


    cout << "loginID" << loginID  << endl;


    cout << "err code " <<  ret->code << ret->msg << " svTRID " <<  ret->svTRID  << endl;

    strcpy(  handle , "CID:ID01"  );

  
    cout << " ContactInfo " <<  handle <<   endl;

    ret =  EPP->ContactInfo( handle ,  cc , loginID , "test-contact-info" , "<XML>debil</XML>");
    cout << "ContactInfo err code " << ret->code << ret->msg << " serverTRID " <<  ret->svTRID  << endl;
    cout << "ContactInfo " <<  cc->Name << endl;

    admin = new ccReg::AdminContact;
    admin->length(0);
/*
    admin[0] = CORBA::string_dup("CID:JOUDA" );
    admin[1] = CORBA::string_dup("CID:DEBIL" );
    admin[2] = CORBA::string_dup("CID:KOKOT" );
*/

//    ext.length(0);
    ext = new ccReg::ExtensionList;
    ext->length(0);

    period.count=1;
    period.unit=ccReg::unit_year;


    strcpy(  fqdn , "neco.cz" );
    cout << "DomainCreate " << fqdn << endl;

try {
    ret =  EPP->DomainCreate( "neco.cz" , "cid:id01" , "NSSID:bazmek" , "heslicko" , period , admin , crDate , exDate ,   loginID , "domain-neco.cz" , "<XML>domaincreator</XML>" , ext );
    cout << "domain create err code " << ret->code  <<  ret->msg <<  ret->svTRID << "crDate: " << crDate << "exDate" << exDate << endl;

  }
  catch(ccReg::EPP::EppError& ex) 
    {
    cerr << "EPP Error: errCode: "  <<  ex.errCode  << " msg: " << ex.errMsg << " svTRID: "   <<   ex.svTRID << endl;

     for( unsigned int  i = 0 ; i < ex.errorList.length() ; i ++ )
        {
          cout << "error list  param: " << ex.errorList[i].code << " reason: "  <<   ex.errorList[i].reason << " position " <<  ex.errorList[i].position << endl ;
        }
  }


 

   cout << "DomainInfo: " << fqdn << endl;

   ret =  EPP->DomainInfo(  fqdn ,  domain , loginID , "info-neco" , "<XML>"  );
   cout << "doman inf err code " << ret->code  <<  ret->msg <<  ret->svTRID  << endl;

   for( i = 0 ; i < domain->admin.length() ; i ++ ) cout << "admin: "  << domain->admin[i] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  domain->nsset <<  endl;

   cout << "CrDate: " <<  domain->CrDate << endl;
   cout << "UpDate: " <<  domain->UpDate << endl;
   cout << "TrDate: " <<  domain->TrDate << endl;
   cout << "ExDate: " <<  domain->ExDate << endl;

   delete domain;

/*
    ret =  EPP->domainSendAuthInfo("nic.edu" , loginID, "EPP_DomainSendAuthInfo" , "<XML>" );
    cout << "SendAuthInfo err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

    ret =  EPP->nssetSendAuthInfo("SID:EXAMPLE" , loginID, "EPP_nssetSendAuthInfo" , "<XML>" );
    cout << "SendAuthInfo err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

    ret =  EPP->nssetSendAuthInfo("NSSID:NEWNSSET" , loginID, "EPP_nssetSendAuthInfo" , "<XML>" );
    cout << "SendAuthInfo err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

    ret =  EPP->contactSendAuthInfo("debil" , loginID, "EPP_contactSendAuthInfo" , "<XML>" );
    cout << "SendAuthInfo err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;
   
*/
/*
    ret =  EPP->ContactInfo( "CID:DEBIL" ,  cc , loginID , "test-contact-info" , "<XML>debil</XML>");
    cout << "ContactInfo err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;
      cout << "ContactInfo " <<  cc->Name << endl;

  for( i = 0 ; i < cc->stat.length() ; i ++ )
       cout << "status " <<  cc->stat[i].value <<  cc->stat[i].text << endl;
*/
/*
    ret =  EPP->ClientCredit( credit ,   loginID  , "client-credit-ZC" , "<XML>get_rededit</XML>" );
    cout << "ClientCredit " << ret->ode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

   for( i = 0 ; i < credit->length() ; i ++  )
    {
     cout << "credit " << credit[i].price << " zone " << credit[i].zone_fqdn << endl;
    }
*/

/*
  ret =  EPP->PollRequest( msgID ,  count ,  qDate , mesg ,  loginID ,  "poll-request"  ,  "<XML>");
  cout << "PollRequest: "  << "msgID " << msgID << endl ;
  cout << "err code " << ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

  cout << "count " << count << " qDate: " <<  qDate  <<  endl;
  cout << "message " << mesg << endl ;

 msgID = CORBA::string_dup( "2" );
  ret =  EPP->PollAcknowledgement( msgID ,  count , newmsgID , loginID ,  "poll-acknowledgement" , "<XML>"  );
  cout << "PollRAcknowledgement: "  << ret->errCode  << endl;
  cout << "count " << count << "newmsgID: " << newmsgID <<  " count " << count <<  endl;
  cout << "err code " << ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;
*/

/*
   ret =  EPP->DomainInfo(  "beta.cz" ,  domain , loginID , "info-enum-4.4.4" , "<XML>"  );
   cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

   for( i = 0 ; i < domain->admin.length() ; i ++ ) cout << "admin: "  << domain->admin[i] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  domain->nsset <<  endl;
  
   cout << "CrDate: " <<  domain->CrDate << endl;
   cout << "UpDate: " <<  domain->UpDate << endl;
   cout << "TrDate: " <<  domain->TrDate << endl;
   cout << "ExDate: " <<  domain->ExDate << endl;
   cout << "err code " << ret->errCode   << endl;
   cout << "ext length " <<   domain->ext.length() << endl ;
   enumVal = new  ccReg::ENUMValidationExtension;


    if(  domain->ext.length() == 1 )
     {
       if(  domain->ext[0] >>= enumVal   )
        {
         cout << "valExpDate" <<    enumVal->valExDate  << endl;
        }
       else cout << "Unknown value extension" << endl;
     }

   delete domain;

*/

//     ret =  EPP->ClientCredit( credit ,  loginID , "clTRID-credit" , "<XML>credit</XML>" );
//     cout << "credit " << credit <<  "err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;
/*
    ret =  EPP->ContactList ( lists , loginID , "contact-list" , "<XML>contact-list</XML>" );
    cerr << "ContactList err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;
 
    cerr << "Contacts total:" << lists->length() << endl ;

     for( i = 0 ; i < lists->length() ; i ++ )
        cout << "contact: " << (*lists)[i] << endl;
    delete lists;


    ret =  EPP->NSSetList ( lists , loginID , "nsset-list" , "<XML>nsset-list</XML>" );
    cerr << "NSsetList err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

    cerr << "NSSET total:" << lists->length() << endl ;

     for( i = 0 ; i < lists->length() ; i ++ )
        cout << "nsset: " << (*lists)[i] << endl ;


    delete lists;
    ret =  EPP->DomainList ( lists , loginID , "domain-list" , "<XML>domain-list</XML>" );
    cerr << "domainList err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;

    cerr << "domain total:" << lists->length() << endl ;

     for( i = 0 ; i < lists->length() ; i ++ )
        cout << "domain: " << (*lists)[i] << endl ;


    delete lists;
*/


/*

if( loginID )
{
num = 0;



  for( i = 0 ; i < num ; i ++ )
  {

    random_string( name ,  13 );

    strcpy(  handle , "CID:XX"  );
    strcat(  handle , name );




  ch = new ccReg::ContactChange;
  ch->Name  = CORBA::string_dup( "Jmeno Prijmeni" );
  ch->CC  =  CORBA::string_dup( "CZ" );
  ch->City =  CORBA::string_dup( "mesto");
  ch->Organization = CORBA::string_dup( "" );
  ch->Streets.length(0); // zadna ulice

  ch->StateOrProvince = CORBA::string_dup( "" );
  ch->PostalCode = CORBA::string_dup( "252 62");
  ch->Telephone = CORBA::string_dup( "" );
  ch->Fax = CORBA::string_dup( "" );
  ch->Email = CORBA::string_dup( "neco@email.cz" );
  ch->NotifyEmail= CORBA::string_dup( "" );
  ch->VAT= CORBA::string_dup( "" );
  ch->ident = CORBA::string_dup( "123456l/LN01" ); 
  ch->identtype =  ccReg::PASS;
  ch->AuthInfoPw = CORBA::string_dup( "heslo" );
  ch->DiscloseFlag = ccReg::DISCL_DISPLAY;
  ch->DiscloseName = ccReg::DISCL_DISPLAY;
  ch->DiscloseOrganization = ccReg::DISCL_DISPLAY;
  ch->DiscloseAddress = ccReg::DISCL_DISPLAY;
  ch->DiscloseTelephone = ccReg::DISCL_DISPLAY;
  ch->DiscloseFax = ccReg::DISCL_DISPLAY;
  ch->DiscloseEmail = ccReg::DISCL_DISPLAY;

 
   cout << "contact create  " << handle << endl;     
   

   ret =  EPP->ContactCreate( handle , *ch ,  crDate ,  loginID , "test-contact-create"  , "<XML omniORB>"   );

   cout << "contact create  " << handle << "  "  << ret->errCode  <<  ret->errMsg <<  ret->svTRID << endl;
   cout << "Create date: " << crDate  << endl; 
     for( j  = 0 ; j <  ret->errors.length() ; j ++ )
        {
           cout << "errorsreason:  " <<  ret->errors[j].code <<  ret->errors[j].reason  << ret->errors[j].value << endl;
        }
   delete ch;
//   delete ret;

   ret =  EPP->ContactInfo( handle ,  cc , loginID , "test-contact-info" , "<XML>debil</XML>");
   cout << "ContactInfo err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;
   cout << "ContactInfo " <<  handle << endl;
   delete cc;
//   delete ret;



//    sprintf( fqdn , "d-%d-ZZ.cz" ,  (int ) i );

    admin = new ccReg::AdminContact;
    admin->length(3);
    admin[0] = CORBA::string_dup("CID:JOUDA" );
    admin[1] = CORBA::string_dup("CID:DEBIL" );
    admin[2] = CORBA::string_dup("CID:KOKOT" );


//    ext.length(0);
    ext = new ccReg::ExtensionList;
    ext->length(0);

    period.count=1;
    period.unit=ccReg::unit_year;


    random_string( name ,  10 );

    strcpy(  fqdn , name  );
    strcat(  fqdn , "-tt.cz" );
 
   cout << "domain " << fqdn << endl;
   ret =  EPP->DomainCreate( fqdn , handle , "NSSID:BAZMEK" , "heslicko" , period , admin , crDate , exDate ,   loginID , "domain-create-epp-client" , "<XML>domaincreator</XML>" , ext );
   cout << "domain create err code " << ret->errCode  <<  ret->errMsg <<  ret->svTRID << "crDate: " << crDate << "exDate" << exDate << endl;
  

   ret =  EPP->DomainInfo(  fqdn ,  domain , loginID , "info-cpp-temp"  , "<XML>");
   cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

   for( j = 0 ; j < domain->admin.length() ; j ++ ) cout << "admin: "  << domain->admin[j] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  endl;
   cout << "ExpDate: " << domain->ExDate  << endl;
   cout << "err code " << ret->errCode   << endl;
   cout << "ext length " <<   domain->ext.length() << endl ;


   ret =  EPP->ContactDelete(  handle ,  loginID , "test-contact-delete" , "" );
   cout << "contact delete " << handle << "  "  << ret->errCode  <<  ret->errMsg <<  ret->svTRID << endl;
   delete ret;

  }



}
*/
/*
    ret =  EPP->ClientLogout( loginID , "XXXX-logout" , "");
    cout << "err code " <<  ret->errCode  << " svTRID " <<  ret->svTRID  << endl;



    ret =  EPP->ClientLogin(  "REG-LRR2"  ,  "123456789"  , "" ,     "LRR-login-now" , "" , loginID , "AE:B3:5F:FA:38:80:DB:37:53:6A:3E:D4:55:E2:91:97" ,  ccReg::EN  );

    cout << "loginID" << loginID  << endl;


    cout << "err code " <<  ret->errCode << ret->errMsg << " svTRID " <<  ret->svTRID  << endl;


if( loginID )
{

num = 1000;
  for( i = 0 ; i < num ; i ++ )
  {
   sprintf( handle , "CID:ORBA%d" , i );

    ret =  EPP->ContactTransfer( handle, "heslo" ,  loginID, "test-contact-transfer" , "<XML></XML>"  );
    cout << "contact transfer err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;


    ret =  EPP->ContactInfo( handle ,  cc , loginID , "test-contact-info" , "<XML></XML>");
    cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;


    cout << "info: "  <<  cc->Name << cc->Email << cc->DiscloseEmail <<  cc->CountryCode << endl;
    t = cc->TrDate;
    cout << "transfer date: " << ctime( &t )  << endl;

    delete cc;

    ret =  EPP->ContactDelete(  handle ,  loginID , "test-contact-delete" , "" );
    cout << "contact delete " << handle << "  "  << ret->errCode  <<  ret->errMsg <<  ret->svTRID << endl;
  }



}
*/

/*
   ret =  EPP->DomainInfo(  "exp.cz" ,  domain , loginID , "info-cpp-temp" );
   cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

   for( i = 0 ; i < domain->admin.length() ; i ++ ) cout << "admin: "  << domain->admin[i] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  endl;
   t = domain->ExDate;
   cout << "ExpDate: " << ctime( &t )  << endl;
   cout << "err code " << ret->errCode   << endl;
   cout << "ext length " <<   domain->ext.length() << endl ;
//   enumVal = new  ccReg::ENUMValidationExtension;
  
  
    if(  domain->ext.length() == 1 )
     {
       if(  domain->ext[0] >>= enumVal   ) 
        {
          exvalDate  = enumVal->valExDate ; 

         cout << "valExpDate" <<   exvalDate  << endl;
        }
       else cout << "Unknown value extension" << endl;
     }

   delete domain;

    ext = new  ccReg::ExtensionList;
//    ext.length(0);
   
 

  cout << "login ID" <<  loginID << endl;
//  ret =  EPP->DomainRenew(  "e$$xp.cz" ,  t  , 18 , exDate ,  loginID , "renew-ex" ,  ext ); 
//  cout << "Domain renew" << "err code " << ret->errCode   << endl;     
ret =  EPP->ContactDelete( "e$$xp." ,   loginID ,  "omniorb-delete-contact"   );
cout << "Contact delete" << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

     for( i = 0 ; i <  ret->errors.length() ; i ++ )
        {
           cout << "errors " <<  ret->errors[i].code <<  ret->errors[i].reason << endl;
            an = ret->errors[i].value ;
           if( an >>= msg) cout << "msg value " << msg << endl;
           if( an >>= sh) cout << "int value " << sh << endl;
       }
*/

/*
    ret =  EPP->ContactInfo( "jouda",  cc , loginID , "jouda_info" );
    cout << "err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

    cout << "info "  << cc->Name  <<  endl;
 
    cout <<  cc->Name << cc->Email << cc->DiscloseEmail <<  cc->Country << endl;
*/
/*
      ch = new ccReg::ContactChange;

<<<<<<< .mine

   ret =  EPP->DomainInfo(  "nic.cz" ,  domain , loginID , "info-cpp-nic" );
=======

     add = new ccReg::Status;
     add->length(2);
     add[0] = CORBA::string_dup(  "clientDeleteProhibited" );
     add[1] = CORBA::string_dup(  "serverDeleteProhibited" );
 
     rem = new ccReg::Status;
     rem->length(1);
     rem[0] = CORBA::string_dup(  "clientUpdateProhibited" );

     ret =  EPP->ContactUpdate( "NECOCZ-PETR",  *ch , add , rem  , loginID , "NECOCZ-ROBERT-update status" );
     cout << "err code " << ret->errCode << " serverTRID " <<  ret->svTRID  << endl;

*/
/*
*/
/*
   ret =  EPP->DomainDelete(  "super.cz"  , loginID , "delete-cpp-super" );
   cout << "Delete err code " << ret->errCode << ret->errMsg << " serverTRID " <<  ret->svTRID  << endl;

   t = time(NULL); 
   enumVal = new  ccReg::ENUMValidationExtension;  
   enumVal->valExDate = t ; 
   cout << " valExDate " <<  enumVal->valExDate << endl;

    ext.length(1);
    ext[0] <<= enumVal;

    admin = new ccReg::AdminContact;
    admin->length(1);
    admin[0] = CORBA::string_dup( "NECOCZ-ADMIN" );
    

   ret =  EPP->DomainCreate( "super.cz" , "NECOCZ-PETR" , "NECOCZ" , "heslicko" , 12 , admin , crDate , exDate ,    loginID , "domain-create"  , ext );
   cout << "domain create err code " << ret->errCode  <<  ret->errMsg <<  ret->svTRID << "crDate: " << crDate << "exDate" << exDate << endl;

    add = new ccReg::Status;
    add->length(0);
    rem = new ccReg::Status;
    rem->length(0);
 
  admin[0] = CORBA::string_dup( "NECOCZ-ROBERT" );
 ret =  EPP->DomainUpdate(  "super.cz" , "NECOCZ-ROBERT" , "" , "" ,  admin  , admin ,   add , rem ,  loginID , "domain-update" , ext );

   cout << "domain update err code " << ret->errCode  <<  ret->errMsg <<  ret->svTRID <<  endl;
*/

/*
  ret =  EPP->PollRequest( msgID ,  count ,  qDate , mesg ,  loginID ,  "poll-request" );
  cout << "PollRequest: "  << ret->errCode  << endl ; 
  t = qDate;
  cout << "count " << count << " qDate: " <<  ctime( &t )  <<  endl;
  cout << msgID << mesg << endl ;

  ret =  EPP->PollAcknowledgement( msgID ,  count , newmsgID , loginID ,  "poll-acknowledgement" );
  cout << "PollRAcknowledgement: "  << ret->errCode  << endl;
  cout << "count " << count << "newmsgID: " << newmsgID << endl;


*/
/*
   add = new ccReg::Status;
     add->length(0);

    tech_rem = new ccReg::TechContact;
    tech_rem->length(0);

     dns_rem = new ccReg::DNSHost;
     dns_rem->length(1);
     dns_rem[0].inet.length(0);
     dns_rem[0].fqdn = CORBA::string_dup( "ns1.bazmek.net" );

     dns_add = new ccReg::DNSHost;
     dns_add->length(1);
     dns_add[0].inet.length(1);
     dns_add[0].inet[0] = CORBA::string_dup( "194.23.54.1" );
     dns_add[0].fqdn = CORBA::string_dup( "ns99.bazmek.net" );
    

   //dns_chg[0].inet.length(2);
    //dns_chg[0].fqdn = CORBA::string_dup( "zeleny.neco.cz" );      

     
    ret =  EPP->NSSetUpdate(  "exampleNsset" , "" ,  dns_add ,  dns_rem , tech_rem , tech_rem , add , add ,  loginID , "nsset-update-xxx" );

    cout << "err code " << ret->errCode   << endl;
*/
/*


   ret =  EPP->DomainInfo(  "temp.cz" ,  domain , loginID , "info-cpp-temp" );
   for( i = 0 ; i < domain->admin.length() ; i ++ ) cout << "admin: "  << domain->admin[i] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  endl;
   cout << "err code " << ret->errCode   << endl;
   delete domain;

   ret =  EPP->DomainInfo(  "neco.cz" ,  domain , loginID , "info-cpp-neco" );
   for( i = 0 ; i < domain->admin.length() ; i ++ ) cout << "admin: "  << domain->admin[i] << endl;
   cout << "Domain info"  << domain->name << domain->Registrant <<  endl;
   cout << "err code " << ret->errCode   << endl;
   delete domain;
 */
/*
    ret =  EPP->HostInfo( "modry.neco.cz",  host  , loginID ,  "XX-host"  );
    cout << "err code " << ret->errCode  <<  ret->svTRID  << endl;

     cout << "host "  << host->fqdn << " nsset "  << host->nsset << endl;

     for( i = 0 ; i < host->inet.length() ; i ++ ) cout << "InetAddress: " <<  host->inet[i]  << endl ; 


    delete host;
*/
//     ret =  EPP->GetTransaction( loginID, "unknwo act" , 2104);

  //  cout << "get Transaction code " << ret->errCode  <<  ret->svTRID  << endl;


/*
     check = new ccReg::Check;
     check->length(4);
     check[0] = CORBA::string_dup(  "NECOCZ%PETR" );
     check[1] = CORBA::string_dup(  "NECOCZ-Petr" );
     check[2] = CORBA::string_dup(  "NECOCZ-ADMIN" );
     check[3] = CORBA::string_dup(  "NECOCZ-NIKDO" );
 
    ret =  EPP->ContactCheck( check , cr,   loginID ,  "check-neco"  , "XML" );
    cout <<  " err code " << ret->errCode  <<  ret->svTRID  << endl;
 
    for( i = 0 ; i <  cr->length() ; i ++ )
      {
         cout << i << check[i] << "contact avail: " << cr[i].avail << " reason: "  <<  cr[i].reason << endl;
      }


     dcheck.length(4);
     dcheck[0] = CORBA::string_dup(  "www.example.cz" );
     dcheck[1] = CORBA::string_dup(  "example.sk" );
     dcheck[2] = CORBA::string_dup(  "Example.cz" );
     dcheck[3] = CORBA::string_dup(  "EXAMPLE.CZ" );

     ret =  EPP->DomainCheck( dcheck , cr,   loginID ,  "domain-neco" , "XML" );
     cout <<  " err code " << ret->errCode  <<  ret->svTRID  << endl;
 
    for( i = 0 ; i <  cr->length() ; i ++ )
      {
         cout << i << dcheck[i] << "domain avail: " << cr[i].avail << " reason: "  <<  cr[i].reason << endl;
      }
 

 dcheck.length(3);
 dcheck[0] = CORBA::string_dup(  "NECO$CZ" );
 dcheck[1] = CORBA::string_dup(  "necocz" );
 dcheck[2] = CORBA::string_dup(  "NSSET" );

    ret =  EPP->NSSetCheck(  dcheck , cr,   loginID ,  "XX-nsset-check" , "XML" );

    for( i = 0 ; i <  cr->length() ; i ++ )
      {
         cout << i << dcheck[i] << "nsset avail: " << cr[i].avail << " reason: "  <<  cr[i].reason << endl;
      }

*/
 /*

    ret =  EPP->NSSetInfo( "NECOCZ" , nsset ,  loginID ,  "XX-nsset-info-necocz" );
    cout << "err code " << ret->errCode  <<  ret->svTRID  << endl;

     t = (time_t )  nsset->CrDate;
 
     cout << "nsset "  <<  nsset->handle << " created " <<  asctime( gmtime( &t) ) << endl;
     cout << "clID "  <<  nsset->ClID << " crID " << nsset->CrID<< " upID " << nsset->UpID  << endl;

      for( i = 0 ; i < nsset->tech.length() ; i ++ ) cout << "tech contact: " <<  nsset->tech[i] << endl ;

      for( i = 0 ; i < nsset->dns.length() ; i ++ ) 
         {
             cout << "dns: " <<  nsset->dns[i].fqdn << endl;
             for( j = 0 ; j < nsset->dns[i].inet.length() ; j ++ ) cout << "ipadres : "  <<  nsset->dns[i].inet[j] <<  endl ; 
         }



    ret =  EPP->NSSetCreate( "NEWNSSET" , "heslo" ,  nsset->tech ,  nsset->dns , crDate ,   loginID , "new-nsset-create" );
    cout << "NSSetCreate err code " << ret->errCode  <<  ret->svTRID << "crDate: " << crDate << endl;


//    ret =  EPP->NSSetDelete( "NEWNSSET" ,  loginID ,  "new-nsset-delete" );
//    cout << "NSSetDelete err code " << ret->errCode  <<  ret->svTRID  << endl;
//   delete nsset;



//    ret =  EPP->ContactDelete( "TEST-USER" ,  loginID ,  "XX-nsset-delete" );
  //   cout << "err code " << ret->errCode  <<  ret->svTRID  << endl;






    ret =  EPP->ContactInfo( "jouda",  cc , loginID , "jouda_info" );
    cout << "err code " << ret->errCode << " serverTRID " <<  ret->svTRID  << endl;

    cout << "info "  << cc->Name  <<  endl;
 
    cout <<  cc->Name << cc->Email << cc->DiscloseEmail <<  cc->Country << endl;

      for( i = 0 ; i < cc->stat.length() ; i ++ )
         {
            cout << "status: " <<  cc->stat[i] << endl;
         }
*/     



/*
  //  t = (time_t )  contact->CrDate;
   // cout << asctime( gmtime( &t) ) << endl;


   ret =  EPP->DomainInfo(  "neco.cz" ,  domain , loginID , clTRID );
   cout << "err code " << ret->errCode   << endl;

   cout << "domain "  << domain->name << "client: " << domain->ClID << endl;

   ret =  EPP->DomainRenew(  "neco.cz" ,  *domain , 15 ,  loginID , clTRID );
   cout << "err code " << ret->errCode   << endl;


   domain->nsset = CORBA::string_dup( "NSSET" );


   cout << "domain update nsset:" <<  domain->nsset   << endl;

 
//   ret =  EPP->DomainUpdate(  "neco.cz" ,  *domain , loginID , clTRID );

  // cout << "err code " << ret->errCode   << endl;
 
 


  domain = new ccReg::Domain;


for( i = 0 ; i < max ; i ++ )
{


     len =2+(int) (18.0*rand()/(RAND_MAX+1.0));



     random_string( name , len);

     domain->ROID =  CORBA::string_dup( name );
     strcat( name , ".cz" );
     domain->name =  CORBA::string_dup( name );
     domain->Registrant =  CORBA::string_dup( "XPET" );
     domain->CrID =  CORBA::string_dup( "REG-IPEX" );
     domain->ClID =  CORBA::string_dup( "REG-IPEX" );

    cout << "domain " << i  <<  name    << endl;

    ret =  EPP->DomainCreate(  *domain , loginID , "XX-create" );

    cout << "err code " << ret->errCode   << endl;

} 
delete domain;
*/

/*
  domain = new ccReg::Domain;
 
dt.tm_sec = 0;
dt.tm_hour = 0;
dt.tm_min = 0;
dt.tm_mday = 31;        
dt.tm_mon = 0;        
tm_year = 108;       

strcpy( name , "superweb" );

     domain->ROID =  CORBA::string_dup( name );
     strcat( name , ".cz" );
     domain->name =  CORBA::string_dup( name );
     domain->Registrant =  CORBA::string_dup( "XPET" );
     domain->CrID =  CORBA::string_dup( "REG-CT" );
     domain->ClID =  CORBA::string_dup( "REG-CT" );
     domain->ExDate = mktime( &dt );
     domain->ns.length( 3 );
     domain->ns[0] = CORBA::string_dup( "dns.web.net" );
     domain->ns[1] = CORBA::string_dup( "dns.searsch.net" );
     domain->ns[2] = CORBA::string_dup( "dns.superWWW.net" );

     cout << "domain " << i  <<  name    << endl;

    ret =  EPP->DomainCreate( name ,  *domain , loginID , "XX-create" );

    cout << "err code " << ret->errCode   << endl;


max =1000;
 for( d = 0 ; d < max ; d ++ )
 {
     random_string( name , 2 );
     strcat( name , ".cz" );

  //  t = (time_t )  contact->CrDate;
   // cout << asctime( gmtime( &t) ) << endl;
   t = time(NULL);
   cout << "info: " <<  d <<  " " <<   asctime( gmtime( &t) ) << name  << endl;

   sprintf( clTRID , "%06d-%06d" , loginID , d ); 
   ret =  EPP->DomainInfo(  name ,  domain , loginID , clTRID );
   cout << "err code " << ret->errCode   << endl;

   cout << "domain "  << domain->name << "client: " << domain->ClID << endl;

    for( i = 0 ; i < domain->ns.length() ; i ++ ) cout << "DNS: " <<  domain->ns[i]  << endl ; 

 delete domain;
}




cc = new ccReg::Contact;
 
    cc->ROID =  CORBA::string_dup( "XPET" );
    cc->Fax  =  CORBA::string_dup( "1234567890" );

    cout << "ContactUpdate" <<  cc->ROID   << endl;
    
    EPP->ContactUpdate( *cc , "XX-1223" ,  errMsg , svTR );


delete cc;
    EPP->ContactDelete( "XPET",  "ZZ-aaaa" ,  errMsg , svTR );
*/

/*
    cc = new ccReg::Contact;
     max = 10;
    for( i = 0 ; i < max ; i ++) 
       {
        random_string( name , 10 );

           sprintf( roid , "ID-%s" , name );

           cc->ROID = CORBA::string_dup(  roid );  
//           cc->handle = CORBA::string_dup(  roid );  
           cc->Name = CORBA::string_dup( name );
           cc->Country = CORBA::string_dup( "CZ" );
           cc->stat.length(1);
           cc->stat[0] = CORBA::string_dup( "ok" );  
           cc->CrID = CORBA::string_dup( "REG-WEB4U" );
           cc->ClID = CORBA::string_dup( "REG-WEB4U" );
           cout << "Create: "  << i  << cc->ROID <<   cc->Name << endl;  
           EPP->ContactCreate( roid ,  *cc ,loginID ,  "CREATE-contact" );
       } 


    for( i = 0 ; i < max ; i ++)
       {
           sprintf( roid , "ID-%06d" , i+1 );
 
           cout << "Info: " << roid  << endl;

           EPP->ContactInfo(  roid , "XY-1234" , contact , errMsg , svTR );

           sprintf( email, "%06d@neco.cz" , i+1 );
           contact->Email =  CORBA::string_dup( email );
           cout << "Update: " << cc->ROID <<   cc->Email << endl;

           EPP->ContactUpdate( *contact ,      "XY-1234" ,  errMsg , svTR );
       }



    for( i = 0 ; i < max ; i ++)
       {
               sprintf( roid , "ID-%06d" , i+1 );


               cout << "Delete: " << roid  << endl;
               EPP->ContactDelete( roid ,  "ZZ-aaaa" ,  errMsg , svTR );
       }

   delete cc;
*/
//   printf("zeme %c%c [%s] \n" , contact->Country[0] ,  contact->Country[1] , contact->AuthInfoPw);
/*
    add = new ccReg::Status;
    add->length(0);
    rem = new ccReg::Status;
    rem->length(0);

//    dns_chg = new ccReg::DNSHost;
  //  dns_chg->length(1);
    //dns_chg[0].inet.length(2);
    //dns_chg[0].fqdn = CORBA::string_dup( "zeleny.neco.cz" );      
    //dns_chg[0].inet[0] = CORBA::string_dup( "216.31.200.1" );
    //dns_chg[0].inet[1] = CORBA::string_dup( "216.31.200.10" );
    dns_add = new ccReg::DNSHost;
    dns_add->length(0);
    dns_rem = new ccReg::DNSHost;
    dns_rem->length(0);

    tech_rem = new ccReg::TechContact;
    tech_rem->length(1);
    tech_rem[0] = CORBA::string_dup( "NECOCZ-ADMIN" );
    

    tech_add = new ccReg::TechContact;
    tech_add->length(1);
    tech_add[0] = CORBA::string_dup( "NECOCZ-SUPERUSER" );
     
    ret =  EPP->NSSetUpdate(  "NECOCZ" , "" ,  dns_rem ,  dns_add , dns_add , tech_add , tech_rem , add , rem ,  loginID , "nsset-update" );

    cout << "err code " << ret->errCode   << endl;



//    ret = EPP->DomainTrade("neco.cz", "NECOCZ-ROBERT"  , "NECOCZ-PETR" , "" ,  loginID , "domain-trade" );
    cout  << "nsset transfer" << endl;
//    ret = EPP->DomainTransfer("neco.cz",   "NECOCZ-PETR" , "heslo" ,  loginID , "domain-transfer" );
    ret = EPP->NSSetTransfer( "NECOCZ",   "heslo" ,  loginID , "nsset-transfer" );
*/
    cout  << "client logout "<< endl;

    ret =  EPP->ClientLogout( loginID , "XXXX-logout" , "");
  cout << "err code " <<  ret->code  << " svTRID " <<  ret->svTRID  << endl;
     
    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
   cerr << "Caught system exception TRANSIENT -- unable to contact the server." << endl ;
  }





  catch(CORBA::SystemException& ex) {
    cerr << "Caught a SystemException  CORBA::" << ex._name() << endl;
  }



  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }


  catch(omniORB::fatalException& fe) {
   cerr << "Caught omniORB::fatalException:" << endl;
   cerr << "  file: " << fe.file() << endl;
   cerr << "  line: " << fe.line() << endl;
   cerr << "  mesg: " << fe.errmsg() << endl;
  }
  return 0;
}


