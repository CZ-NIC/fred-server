
/*
 * Echo client program.. Hacked by Ewan Birney <birney@sanger.ac.uk>
 * from echo test suite, update for ORBit2 by Frank Rehberger
 * <F.Rehberger@xtradyne.de>
 *
 * Client reads object reference (IOR) from local file 'echo.ior' and
 * forwards console input to echo-server. A dot . as single character
 * in input terminates the client.
 */


 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <orbit/orbit.h>

/*
 * This header file was generated from the idl
 */

#include "ccReg.h"

// #include "utf8hack.h"


static CORBA_ORB  global_orb = CORBA_OBJECT_NIL; /* global orb */
 
/* Is called in case of process signals. it invokes CORBA_ORB_shutdown()
 * function, which will terminate the processes main loop.
 */
static
void
client_shutdown (int sig)
{
        CORBA_Environment  local_ev[1];
        CORBA_exception_init(local_ev);
 
        if (global_orb != CORBA_OBJECT_NIL)
        {
                CORBA_ORB_shutdown (global_orb, FALSE, local_ev);
                etk_abort_if_exception (local_ev, "caught exception");
        }
}
 
        
/* Inits ORB @orb using @argv arguments for configuration. For each
 * ORBit options consumed from vector @argv the counter of @argc_ptr
 * will be decremented. Signal handler is set to call
 * echo_client_shutdown function in case of SIGINT and SIGTERM
 * signals.  If error occures @ev points to exception object on
 * return.
 */
static
void
client_init (int               *argc_ptr,
	     char              *argv[],
             CORBA_ORB         *orb,
             CORBA_Environment *ev)
{
        /* init signal handling */
 
        signal(SIGINT,  client_shutdown);
        signal(SIGTERM, client_shutdown);
         
        /* create Object Request Broker (ORB) */
         
        (*orb) = CORBA_ORB_init(argc_ptr, argv, "orbit-local-orb", ev);
        if (etk_raised_exception(ev)) return;
}

/* Releases @servant object and finally destroys @orb. If error
 * occures @ev points to exception object on return.
 */
static
void
client_cleanup (CORBA_ORB                 orb,
                CORBA_Object              service,
                CORBA_Environment        *ev)
{
        /* releasing managed object */
        CORBA_Object_release(service, ev);
        if (etk_raised_exception(ev)) return;
 
        /* tear down the ORB */
        if (orb != CORBA_OBJECT_NIL)
        {
                /* going to destroy orb.. */
                CORBA_ORB_destroy(orb, ev);
                if (etk_raised_exception(ev)) return;
        }
}

/**
 *
 */
/* static */
/* CORBA_Object */
/* client_import_service_from_stream (CORBA_ORB          orb, */
/* 				   FILE              *stream, */
/* 				   CORBA_Environment *ev) */
/* { */
/* 	CORBA_Object obj = CORBA_OBJECT_NIL; */
/* 	gchar *objref=NULL; */
    
/* 	fscanf (stream, "%as", &objref);  /\* FIXME, handle input error *\/  */
	
/* 	obj = (CORBA_Object) CORBA_ORB_string_to_object (global_orb, */
/* 							 objref,  */
/* 							 ev); */
/* 	free (objref); */
	
/* 	return obj; */
/* } */

/**
 *

 */
/* static */
/* CORBA_Object */
/* client_import_service_from_file (CORBA_ORB          orb, */
/* 				 char              *filename, */
/* 				 CORBA_Environment *ev) */
/* { */
/*         CORBA_Object  obj    = NULL; */
/*         FILE         *file   = NULL; */
 
/*         /\* write objref to file *\/ */
         
/*         if ((file=fopen(filename, "r"))==NULL) */
/*                 g_error ("could not open %s\n", filename); */
    
/* 	obj=client_import_service_from_stream (orb, file, ev); */
	
/* 	fclose (file); */

/* 	return obj; */
/* } */


/**
 *
 */


static void client_run (ccReg_EPP  service,   CORBA_Environment        *ev)
{
 //       ccReg_Domain *d;
        ccReg_Response *ret ;
        CORBA_long  loginID;
	      ccReg_timestamp c_crDate;
             ccReg_ContactChange     *ch;
         time_t t ;	     
        char handle[64] ;
        int i , num ;
      ret=  ccReg_EPP_ClientLogin( service , "REG-GENERAL-REGISTRY"  ,  "123456789" , "" ,  
	      "ortbi2-login" , "", &loginID , "AE:B3:5F:FA:38:80:DB:37:53:6A:3E:D4:55:E2:91:97" ,    ccReg_EN , ev);

      printf( "login loginID = %d ret %d mesg %s svTRID  %s\n" , loginID ,  ret->errCode ,  ret->errMsg  , ret->svTRID );
      

      if( loginID )
      {
      num = 1000;
        for( i = 0 ; i < num ; i ++ )
	  {
           sprintf( handle , "CID:ORBIT%d" , i );

           
	   ch = ccReg_ContactChange__alloc();
	   
     ch->Name  = CORBA_string_dup( handle );
     ch->CC  =  CORBA_string_dup( "CZ" );
     ch->City =  CORBA_string_dup( "mesto");
     ch->Organization = CORBA_string_dup( "" );
     ch->Street1 =  CORBA_string_dup( "ulice");
     ch->Street2 =  CORBA_string_dup( "ulice");
     ch->Street3 =  CORBA_string_dup( "ulice");
     ch->StateOrProvince = CORBA_string_dup( "" );
     ch->PostalCode = CORBA_string_dup( "252 62");
     ch->Telephone = CORBA_string_dup( "" );
     ch->Fax = CORBA_string_dup( "" );
     ch->Email = CORBA_string_dup( "neco@email.cz" );
     ch->NotifyEmail= CORBA_string_dup( "" );
     ch->VAT= CORBA_string_dup( "" );
     ch->SSN = CORBA_string_dup( "123456l/LN01" );
     ch->SSNtype =  ccReg_PASS;
      ch->AuthInfoPw = CORBA_string_dup( "heslo" );
	   ch->DiscloseName = ccReg_DISCL_DISPLAY;
	     ch->DiscloseOrganization = ccReg_DISCL_DISPLAY;
       ch->DiscloseAddress = ccReg_DISCL_DISPLAY;
         ch->DiscloseTelephone = ccReg_DISCL_DISPLAY;
	   ch->DiscloseFax = ccReg_DISCL_DISPLAY;
     ch->DiscloseEmail = ccReg_DISCL_DISPLAY;


             /* send new contact in repository */
       ret = ccReg_EPP_ContactCreate(service, handle , ch ,   &c_crDate, loginID , "orbit2-create-contact" ,  "<XML orbit>", ev );

        printf( "ContactCreate [%s]: ret %d mesg %s svTRID  %s\n" , handle  , ret->errCode ,  ret->errMsg  , ret->svTRID  );
	printf("Create date: %s\n" , ctime( &t ) ); 
      CORBA_free(ch);

      
     }      


       for( i = 0 ; i < num ; i ++ )
          {
	   sprintf( handle , "CID:ORBIT%d" , i );
	   ret=  ccReg_EPP_ContactDelete( service ,  handle ,   loginID ,  "orbit2-delete-contact"  , "", ev );
	   printf( "ContactDelete [%s]: ret %d mesg %s svTRID  %s\n" , handle  , ret->errCode ,  ret->errMsg  , ret->svTRID  );
         }



    }
      

      
        ret=  ccReg_EPP_ClientLogout( service ,  loginID , "orbit2-logout"  , "", ev );
         printf( "logout ret %d mesg %s svTRID  %s\n" ,   ret->errCode ,  ret->errMsg , ret->svTRID );

//         CORBA_free (d);
 
}

/*
 * main 
 */
int
main(int argc, char* argv[])
{
	CORBA_char filename[] = "/tmp/ccReg.ref";
        
	ccReg_EPP e_service = CORBA_OBJECT_NIL;


        CORBA_Environment ev[1];
        CORBA_exception_init(ev);


	client_init (&argc, argv, &global_orb, ev);
	etk_abort_if_exception(ev, "init failed");

	g_print ("Reading service reference from file \"%s\"\n", filename);

	e_service = (ccReg_EPP) etk_import_object_from_file (global_orb,
							   filename,
							   ev);
        etk_abort_if_exception(ev, "import service failed");

	client_run (e_service, ev);

        etk_abort_if_exception(ev, "service not reachable");
 
	client_cleanup (global_orb, e_service, ev);
        etk_abort_if_exception(ev, "cleanup failed");

 
        exit (0);
}
