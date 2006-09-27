
#include <string.h>
#include <stdlib.h>
#include "examples-toolkit.h"

/** 
 * test @ev for exception 
 */
gboolean 
etk_raised_exception(CORBA_Environment *ev) 
{
	return ((ev)->_major != CORBA_NO_EXCEPTION);
}

/** 
 * test @ev for exception 
 */
gboolean 
etk_raised_exception_is_a (CORBA_Environment *ev, CORBA_char* id) 
{
	return ((ev)->_major != CORBA_NO_EXCEPTION)  &&                 
		strcmp(id, CORBA_exception_id(ev)) == 0;
}

/**
 *  
 */
void 
etk_abort_if_exception (CORBA_Environment *ev, const char* mesg) 
{
	if (etk_raised_exception (ev)) {
		g_error ("%s %s", mesg, CORBA_exception_id (ev));
		CORBA_exception_free (ev); 
		abort(); 
	}
}

/**
 *  
 */
void 
etk_ignore_if_exception (CORBA_Environment *ev, const char* mesg) 
{
	if (etk_raised_exception (ev)) {
		g_warning ("%s %s", mesg, CORBA_exception_id (ev));
		CORBA_exception_free (ev); 
	}
}


/**
 *
 */
void
etk_export_object_to_stream (CORBA_ORB          orb,
			     CORBA_Object       servant,
			     FILE              *stream,
			     CORBA_Environment *ev)
{
        CORBA_char *objref = NULL;
 
        /* write objref to file */
         
        objref = CORBA_ORB_object_to_string (orb, servant, ev);
        if (etk_raised_exception(ev)) return;
 
        /* print ior to terminal */
        fprintf (stream, "%s\n", objref);
        fflush (stream);
 
        CORBA_free (objref);
}


/* Writes stringified object reference of @servant to file
 * @filename. If error occures @ev points to exception object on
 * return.
 */
void 
etk_export_object_to_file (CORBA_ORB          orb,
			   CORBA_Object       servant,
			   char              *filename, 
			   CORBA_Environment *ev)
{
        CORBA_char *objref = NULL;
	FILE       *file   = NULL;

	/* write objref to file */
	
	if ((file=fopen(filename, "w"))==NULL) 
		g_error ("could not open %s\n", filename);
	
        /* print ior to stream */
	etk_export_object_to_stream (orb, servant, file, ev);

	fclose (file);
}


/**
 *
 */
static gchar*
etk_read_string_from_stream (FILE *stream)
{
        gulong length = 4*1024; /* should suffice ordinary IOR string */
        gchar *objref = g_malloc0 (length*sizeof(gchar)); /* empty string */
        int c = 0;
        int i = 0;

        /* skip leading white space */
        while((c=fgetc(stream))!=EOF && g_ascii_isspace(c));
        /* POST: c==EOF or c=first character */

        if (c!=EOF)
          /* PRE: c=first character */
          /* append c to string while more c exist and c not white space */
          do {
            /* check size */
            if (i>=(length-1)) {
              length*=2;
              objref=g_realloc (objref, length);
            }
            objref[i++] = c;
          } while ((c=fgetc(stream))!=EOF && !g_ascii_isspace(c));
          /* POST: first string read */

        /* terminate string with \0 */
        objref[i] = '\0';

        /* INV: objref valid string, #objref>=0 */

        return objref;
}

/**
 *
 */
CORBA_Object
etk_import_object_from_stream (CORBA_ORB          orb,
			       FILE              *stream,
			       CORBA_Environment *ev)
{
	CORBA_Object obj = CORBA_OBJECT_NIL;
	gchar *objref=etk_read_string_from_stream (stream);

	if (!objref || strlen (objref)==0) {
		g_warning ("empty object reference");
		if (objref) 
			g_free (objref);
		return CORBA_OBJECT_NIL;		
	}

	obj = (CORBA_Object) CORBA_ORB_string_to_object (orb,
							 objref, 
							 ev);
	free (objref);
	
	return obj;
}

/**
 *
 */
CORBA_Object
etk_import_object_from_file (CORBA_ORB          orb,
			      CORBA_char        *filename,
			      CORBA_Environment *ev)
{
        CORBA_Object  obj    = NULL;
        FILE         *file   = NULL;
  
        /* write objref to file */
          
        if ((file=fopen(filename, "r"))==NULL)
                g_error ("could not open %s\n", filename);
     
        obj= etk_import_object_from_stream (orb, file, ev);
         
	if (obj==CORBA_OBJECT_NIL) 
		g_warning ("object is NIL");

        fclose (file);
 
        return obj;
}
 
/**
 */
CosNaming_NamingContext 
etk_get_name_service (CORBA_ORB         orb,
		      CORBA_Environment *ev)
{
        CORBA_char   *str=NULL;
        CORBA_Object  ref
                = (CORBA_Object) CORBA_ORB_resolve_initial_references(orb,
								      "NameService",
                                                                      ev);
        if (etk_raised_exception(ev)) return CORBA_OBJECT_NIL;
	
        return (CosNaming_NamingContext) ref;
}

/** calculate length of NULL terminated string vector */
static
guint 
id_vec_len (char *id_vec[]) 
{ 
	gint i=0; 
	for (i = 0; id_vec[i]; ++i); 
	return i;
} 

/* binds @servant object reference to unique @name at
 * @name_service. If error occures @ev points to exception object on
 * return.
 */
void 
etk_name_service_bind (CosNaming_NamingContext  name_service,
		       CORBA_Object             servant,
		       gchar                   *id_vec[], 
		       CORBA_Environment       *ev)
{
	gint i = 0;
	gint len = id_vec_len (id_vec);

	/* Allocate a CosNaming::Name (sequence of CosNaming::NameComponent) */
	CosNaming_Name *name = CosNaming_Name__alloc();

	name->_buffer = CORBA_sequence_CosNaming_NameComponent_allocbuf(len);
	name->_maximum = len;
	name->_length  = 0;
	
	/* Relinquish ownership of the NameComponent to the
         * sequence. When CORBA_free is called on it later, the
         * NameComponent will be freed */	
	CORBA_sequence_set_release (name, TRUE);

	/* iterate components of name and create sub-context
	 * (directory) if needed */ 
	for (i = 0; i < len; ++i) {
		name->_length = i+1;
		name->_buffer[i].id   = CORBA_string_dup(id_vec[i]);
		name->_buffer[i].kind = CORBA_string_dup(""); 
		/* don't know what 'kind' shall be good for */ 

		if (name->_length < len) 
		{
			/* create a sub-context */ 
			CosNaming_NamingContext nc = 
				CosNaming_NamingContext_bind_new_context (name_service, 
									  name, 
									  ev);	
			if (etk_raised_exception_is_a (ev, 
						       ex_CosNaming_NamingContext_AlreadyBound))
			{
				/* ignore - ctx allread exists, this
				 * is not dramatic */
				CORBA_exception_free (ev);
			}
			else if (etk_raised_exception (ev)) 
			{
				/* critical - unexpected exception  */ 
				CORBA_free (name); 
				return;
			}
		}
		else
		{
			/* Bind object to last context - use 'rebind'
			 * operation in case the name has been
			 * registered allready in context - note, this
			 * might interfere with other service choosing
			 * same name */ 
			CosNaming_NamingContext_rebind (name_service, 
							name, 
							servant,
							ev);
			if (etk_raised_exception(ev)) {
				/* critical - can not bind object */ 
				CORBA_free (name);
				return;
			}
		}
	}

	CORBA_free (name);
	return;
}

CORBA_Object 
etk_name_service_resolve (CosNaming_NamingContext  name_service,
			  gchar                   *id_vec[], 
			  CORBA_Environment       *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	gint i = 0;
	gint len = id_vec_len (id_vec);

	/* Allocate a CosNaming::Name (sequence of CosNaming::NameComponent) */
	CosNaming_Name *name = CosNaming_Name__alloc();

	g_assert (id_vec_len (id_vec) > 0);

	name->_buffer = CORBA_sequence_CosNaming_NameComponent_allocbuf(len);
	name->_maximum = len;
	name->_length  = 0;
	
	/* Relinquish ownership of the NameComponent to the
         * sequence. When CORBA_free is called on it later, the
         * NameComponent will be freed */	
	CORBA_sequence_set_release (name, TRUE);

	/* iterate components of name and create sub-context
	 * (directory) if needed */ 
	for (i = 0; i < len; ++i) {
		name->_length = i+1;
		name->_buffer[i].id   = CORBA_string_dup(id_vec[i]);
		name->_buffer[i].kind = CORBA_string_dup(""); 
		/* don't know what 'kind' shall be good for */ 
	}
	
	retval = CosNaming_NamingContext_resolve (name_service, 
						  name, 
						  ev);
	
	if (etk_raised_exception (ev)) { 
		CORBA_free (name);
		return CORBA_OBJECT_NIL;
	}
	
	return retval;
}
