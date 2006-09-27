
#ifndef __EXAMPLES_TOOLKIT__
#define __EXAMPLES_TOOLKIT__

#include <stdio.h>
#include <orbit/orbit.h>
#include <ORBitservices/CosNaming.h>

/* extracts type of exception: Three return Values are possible:
 * CORBA_NO_EXCEPTION, CORBA_USER_EXCEPTION, CORBA_SYSTEM_EXCEPTION:/
*/
#define etk_exception_type(ev) (ev->_major)

/** 
 * test @ev for any exception 
 */
gboolean 
etk_raised_exception (CORBA_Environment *ev);

/** 
 * test @ev for specific exception  @ex
 */
gboolean 
etk_raised_exception_is_a (CORBA_Environment *ev, CORBA_char* ex);

/**
 * in case of any exception this operation will abort the process  
 */
void 
etk_abort_if_exception(CORBA_Environment *ev, const char* mesg); 

/**
 * in case of any exception this operation will only free allocated resources
 */
void 
etk_ignore_if_exception(CORBA_Environment *ev, const char* mesg); 

/**
 *
 */
void
etk_export_object_to_stream (CORBA_ORB          orb,
			     CORBA_Object       servant,
			     FILE              *stream,
			     CORBA_Environment *ev);


/* Writes stringified object reference of @servant to file
 * @filename. If error occures @ev points to exception object on
 * return.
 */
void 
etk_export_object_to_file (CORBA_ORB          orb,
			   CORBA_Object       servant,
			   char              *filename, 
			   CORBA_Environment *ev);

/**
 *
 */
CORBA_Object
etk_import_object_from_stream (CORBA_ORB          orb,
			       FILE              *stream,
			       CORBA_Environment *ev);

/**
 *
 */
CORBA_Object
etk_import_object_from_file (CORBA_ORB          orb,
			     CORBA_char        *filename,
			     CORBA_Environment *ev);


/** resolves default name-service, usually given to application as
 * command line argument "-ORBInitRef NameService=IOR:0100000028..",
 * or since release 2.8.0 corbalocs in form of URL can be used, eg:
 * "-ORBInitRef NameService=corbaloc:iiop:HOSTNAME:PORT/NameService%00"
 */
CosNaming_NamingContext 
etk_get_name_service (CORBA_ORB         orb,
		      CORBA_Environment *ev);


/* binds @servant object reference to unique @name at
 * @name_service. @name is a NULL terminated list of strings
 * (CORBA_char*). If error occures @ev points to exception object on
 * return.
 */
void 
etk_name_service_bind (CosNaming_NamingContext  name_service,
		       CORBA_Object             servant,
		       gchar                   *id_vec[], 
		       CORBA_Environment       *ev);

/* resolves object reference @return with unique @name at
 *  @name_service. @name is a NULL terminated list of strings
 *  (CORBA_char*).  If error occures @ev points to * exception object
 *  on return.
 */
CORBA_Object 
etk_name_service_resolve (CosNaming_NamingContext  name_service,
			  gchar                   *id_vec[], 
			  CORBA_Environment       *ev);

#endif
