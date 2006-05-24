#!/usr/bin/env python 
# init test zatim pro funkce domeny 
# dodelat vsechny funkce od vytvareni kontaktu pres nsset az po domenu
import sys 
from omniORB import CORBA 
import ccReg
import time

orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID) 
file = open( "/tmp/ccReg.ref" );
ior = file.read(); 
# sys.argv[1] 

obj = orb.string_to_object(ior) 
epp = obj._narrow(ccReg.EPP) 

if epp is None:  
  print "Object reference is not an ccReg::EPP" 
  sys.exit(1) 
else : 
   print "OK ccReg::EPP"

result =  epp.GetTransaction( 1203 , "unknwo act" , 2104);

print "Get transaction  svrTRID '%s' err '%d'" % ( result.svTRID ,result.errCode )

result = epp.ClientLogin( "REG-WEB4U" , "123456789" , "", "python-login"  );
# nelze pouzivat out parametry vraci se jako navratova hodnota
loginID = result[1]

print "Login  svrTRID '%s' err '%d' loginID '%d'" % ( result[0].svTRID ,result[0].errCode , loginID  );

result = epp. ContactInfo(  'NECOCZ-PETR' , loginID , "pcontact-info" );
contact =  result[1]
print "Conact Info  '%s' err '%d' name  '%s' " % ( result[0].svTRID ,result[0].errCode , contact.Name  ) ;

contact.Name = "David Pospisilik"
contact.City = "Karlovy Vary"
result = epp.ContactCreate( 'NECOCZ-DAVID'  , contact ,  loginID , "david-create" );

print "ContactCreate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );

contact.Email = "david@neco.cz"
result = epp.ContactUpdate( 'NECOCZ-DAVID'  , contact , [""] ,  [""] ,  loginID , "david-update" );
print "ContactUpdate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );


result = epp.ContactDelete( 'NECOCZ-DAVID'  ,   loginID , "david-delete" );
print "ContactDelete  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );
 

 
result =epp.DomainInfo( 'neco.cz' , loginID , "python-dinfo" );
domain = result[1]

print "domain info: " , domain.name , domain.nsset,  domain.Registrant ,  domain.ClID,    domain.admin  , domain.stat
print domain.admin

print "DomainInfo  svrTRID '%s' err '%d' " % ( result[0].svTRID ,result[0].errCode  )


result =epp.NSSetInfo( domain.nsset , loginID , "nsset-info" );
nsset = result[1]

print "NSSetInfo  svrTRID '%s' err '%d' " % ( result[0].svTRID  ,result[0].errCode  )
print "nsset info: " , nsset.handle , nsset.dns ,  nsset.tech  


result =  epp.DomainCreate( 'temp.cz', 'NECOCZ-PETR' , 'NECOCZ' , "heslo1234" , 12 , [   'NECOCZ-ADMIN' ,   'NECOCZ-ROBERT' ] , loginID , "python-create" );

print "DomainCreate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );


result =  epp.DomainCheck( [  'temp.cz' , 'neco.cz' , 'xxx.cz' ], loginID , "python-domaincheck" )
av =  result[1]
print "DomainCheck  svrTRID '%s' err '%d' loginID '%d'" % ( result[0].svTRID ,result[0].errCode , loginID  );
print "avial " , av


result =  epp.DomainUpdate( 'temp.cz', "NECOCZ-ROBERT" ,  "heslo1234" , "" , "" , [   'NECOCZ-PETR' ] , [ 'NECOCZ-ADMIN'  ] , [ "clientTransferProhibited" ]  ,  [ "clientDeleteProhibited"  ] , loginID , "python-update" );
print "DomainUpdate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );

result =epp.DomainInfo( 'temp.cz' , loginID , "python-info" );
domain = result[1]

print "domain info: " 
print "name: " ,  domain.name  ,   "NSSET: "  , domain.nsset ,  "reg:" ,  domain.Registrant , "client:"  ,  domain.ClID 
print "status: " , domain.stat  , "ADMIN: " ,  domain.admin , "auth: "  , domain.AuthInfoPw
print "DomainInfo  svrTRID '%s' err '%d' " % ( result[0].svTRID ,result[0].errCode  )

result =  epp.DomainRenew( 'temp.cz' , domain.ExDate , 24 , loginID , "python-renew" );  
print "DomainReNew  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );

result =  epp.DomainDelete( 'temp.cz',  loginID , "python-delete" );
print "DomainDelete  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );
  


result =  epp.ClientLogout( loginID , "python-logout" );
print "Logout  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );
