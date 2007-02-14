BEGIN{   id = 1;  }
{
# pro create domain enum_operation 1
printf "INSERT INTO  invoice_object_registry ( id , objectID  , operation , crDate , zone , registrarID ) values   ( %d ,  %d , 1 , '%s' , 1 , %d ); \n", id , $3  , $2 , $4; 
printf "INSERT INTO  invoice_object_registry_price_map( id , invoiceID , price )  values   ( %d ,  %d , 0 );\n" , id , $1 ;   

id = id + 1 ;
# pro renew-domain enum_operation 2  predpoklada se prodlouzeni o 12 mesicu u vsech domen POZOR!!! pokud to tak neni je nutne nasledne nastavit period
printf "INSERT INTO  invoice_object_registry ( id , objectID  , operation , crDate , zone , registrarID  , ExDate , period )  values   ( %d ,  %d , 2 , '%s' , 1 , %d ,  '%s'  , 12 ); \n", id , $3  , $2 , $4 , $5 ;
printf "INSERT INTO  invoice_object_registry_price_map( id , invoiceID , price )  values   ( %d ,  %d , 1.0 );\n" , id , $1 ;
id = id + 1 ;
};
END{ print "SELECT SETVAL( 'invoice_object_registry_id_seq' , id); -- nastaveni sequence ID "}
