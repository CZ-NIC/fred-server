#!/usr/bin/env python 
# init test zatim pro funkce domeny 
# dodelat vsechny funkce od vytvareni kontaktu pres nsset az po domenu
import sys 
from omniORB import CORBA 
import ccReg

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


result =epp.DomainInfo( 'neco.cz' , loginID , "python-dinfo" );
domain = result[1]

print "domain info: " , domain.name , domain.nsset,  domain.Registrant ,  domain.ClID 
print "DomainInfo  svrTRID '%s' err '%d' " % ( result[0].svTRID ,result[0].errCode  )

result =  epp.DomainCreate( 'temp.cz', 'NECOCZ-PETR' , 'NECOCZ' , "heslo1234" , 12 , [   'NECOCZ-ADMIN' ,   'NECOCZ-ROBERT' ] , loginID , "python-create" );

print "DomainCreate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );

result =  epp.DomainUpdate( 'temp.cz', "NECOCZ-ROBERT" ,  "heslo1234" , "" , "" , [   'NECOCZ-PETR' ] , [ 'NECOCZ-ADMIN'  ] , [ "clientTransferProhibited" ]  ,  [ ""  ] , loginID , "python-update" );
print "DomainUpdate  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );

result =epp.DomainInfo( 'temp.cz' , loginID , "python-dinfo" );
domain = result[1]

print "domain info: " , domain.name , domain.nsset,  domain.Registrant ,  domain.ClID
print "DomainInfo  svrTRID '%s' err '%d' " % ( result[0].svTRID ,result[0].errCode  )
  
result =  epp.DomainDelete( 'temp.cz',  loginID , "python-delete" );
print "DomainDelete  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );
  


result =  epp.ClientLogout( loginID , "python-logout" );
print "Logout  svrTRID '%s' err '%d' " % ( result.svTRID ,result.errCode );
