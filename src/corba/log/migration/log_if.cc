/*
 * log_if.cc
 *
 *  Created on: Jan 27, 2009
 *      Author: jvicenik
 */
#include <string>
#include "m_epp_parser.h"
#include "types.h"

/* strlen, strcpy */ 
#include <string.h> 

using namespace std;

char * wrap_str(const char *str)
{
	char *ret;
	int len = strlen(str) + 1;

	ret = (char*)malloc(len);
	if (ret == NULL) return NULL;

	strcpy(ret, str);
	return ret;
}

// this would be much nicer
// typedef list<ccLogProperty> ccProperties;

#define MAX_ERROR_MSG_LEN 1024
/**
 * Add a name, value pair to the properties. Allocate memory and the property list itself
 * on demand
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param name		property name
 * @param value		property value
 * @param output	whether the property is related to output
 * @param child 	true if the property is child to the last property with child = false
 *
 * @returns			NULL in case of an allocation error, modified c_props otherwise
 */
ccProperties *epp_property_push(ccProperties *c_props, const  char *name, const char *value, bool output, bool child)
{
	if(c_props == NULL) {
		c_props = new ccProperties;
		if (c_props == NULL) {
				return NULL;
		}
		c_props->_maximum = ALLOC_STEP;
		c_props->_buffer = (ccLogProperty*)malloc(c_props->_maximum * sizeof(ccLogProperty));
		if (c_props->_buffer == NULL) {
				delete c_props;
				return NULL;
		}
		c_props->_length = 0;
	}

	if (value != NULL) {
		if(c_props->_length >= c_props->_maximum) {
			c_props->_maximum += ALLOC_STEP;
			c_props->_buffer = (ccLogProperty*)realloc(c_props->_buffer, c_props->_maximum*sizeof(ccLogProperty));
			if (c_props->_buffer == NULL) {
				delete c_props;
				return NULL;
			}
		}
		c_props->_buffer[c_props->_length].name = wrap_str(name);
		if(c_props->_buffer[c_props->_length].name == NULL) {
			delete c_props;
			return NULL;
		}
		c_props->_buffer[c_props->_length].value = wrap_str(value);
		if(c_props->_buffer[c_props->_length].value == NULL) {
			delete c_props;
			delete c_props->_buffer[c_props->_length].name;
			return NULL;
		}
		c_props->_buffer[c_props->_length].output = output;   // TODO true if it's output
		c_props->_buffer[c_props->_length].child = child;
		c_props->_length++;
	}
	return c_props;
}


/**
 * Add the content of a qhead linked list to the properties.
 * The list should contain only strings
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param list		list of strings
 * @param list_name	base name for the inserted properties
 * @param output 	whether the properties are related to output
 * @param child		true if the items in the list are children of the last property
 * 					with child = false
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */

ccProperties *epp_property_push_qhead(ccProperties *c_props, qhead *list, const char *list_name, bool output, bool child)
{
	ccProperties *ret;

	if (list->count == 0) {
		return c_props;
	}

	q_foreach(list) {
		if ((ret = epp_property_push(c_props, list_name, (char*)q_content(list), output, child)) == NULL) {
			return NULL;
		}
	}

	return ret;
}

/**
 * Add a name, value pair to the properties, where value is an integer
 * Allocate buffer on demand.
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param name		property name
 * @param value		property integer value
 * @param output	true if this property is related to output (response), false otherwise
 *
 * @returns			NULL in case of an allocation error, modified c_props otherwise
 */

ccProperties *epp_property_push_int(ccProperties *c_props, const char *name, int value, bool output)
{
	char str[12];

	if(c_props == NULL) {
		c_props = new ccProperties;
		if (c_props == NULL) {
				return NULL;
		}
		c_props->_maximum = ALLOC_STEP;
		c_props->_buffer = (ccLogProperty*)malloc(c_props->_maximum * sizeof(ccLogProperty));
		if (c_props->_buffer == NULL) {
				delete c_props;
				return NULL;
		}
		c_props->_length = 0;
	}


	if(c_props->_length >= c_props->_maximum) {
		c_props->_maximum += ALLOC_STEP;
		c_props->_buffer = (ccLogProperty*)realloc(c_props->_buffer, c_props->_maximum*sizeof(ccLogProperty));
		if (c_props->_buffer == NULL) {
			delete c_props;
			return NULL;
		}
	}
	snprintf(str, 12, "%i", value);
	c_props->_buffer[c_props->_length].name = wrap_str(name);

	if(c_props->_buffer[c_props->_length].name == NULL) {
		delete c_props;
		return NULL;
	}
	c_props->_buffer[c_props->_length].value = wrap_str(str);

	if(c_props->_buffer[c_props->_length].value == NULL) {
		delete c_props;
		delete c_props->_buffer[c_props->_length].name;
		return NULL;
	}

	c_props->_buffer[c_props->_length].output = output;
	c_props->_buffer[c_props->_length].child = false;
	c_props->_length++;

	return c_props;

}

/**
 *  * 	Add postal info to log item properties
 *   *  @param 	p 	log entry properties or a NULL pointer (in which
 *    * 					case a new data structure is allocated and returned)
 *     *  @param  pi	postal info
 *      *
 *       *  @returns 	log entry properties or NULL in case of an allocation error
 *        */
ccProperties *epp_log_postal_info(ccProperties *p, epp_postalInfo *pi)
{
	if(pi == NULL) return p;

	p = epp_property_push(p, "pi.name", pi->name, false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.organization", pi->org, false, false);
	if (p == NULL) return p;
	p = epp_property_push_qhead(p, &pi->streets, "pi.street", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.city", pi->city, false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.state", pi->sp, false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.postalCode", pi->pc, false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.countryCode", pi->cc, false, false);
	if (p == NULL) return p;

	return p;
}

/**
 *  * 	Add disclose info to log item properties
 *   *  @param 	p 	log entry properties or a NULL pointer (in which
 *    * 					case a new data structure is allocated and returned)
 *     *  @param  ed	disclose info
 *      *
 *       *  @returns 	log entry properties or NULL in case of an allocation error
 *        */
ccProperties *epp_log_disclose_info(ccProperties *p, epp_discl *ed)
{
	if(ed->flag == 1) {
		p = epp_property_push(p, "discl.policy", "private", false, false);
	} else if(ed->flag == 0) {
		p = epp_property_push(p, "discl.policy", "public", false, false);
	} else {
		p = epp_property_push(p, "discl.policy", "no exceptions", false, false);
	}

	if (p == NULL) return p;

	p = epp_property_push(p, "discl.name", ed->name ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.org", ed->org ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.addr", ed->addr ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.voice", ed->voice ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.fax", ed->fax ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.email", ed->email ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.vat", ed->vat ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.ident", ed->ident ? "true" : "false", false, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.notifyEmail", ed->notifyEmail ? "true" : "false", false, false);
	if (p == NULL) return p;

	return p;
}


/**
 * Add qhead with ds records to properties
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param list		list of ds records
 * @param list_name	base name for the inserted properties
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */
ccProperties *epp_property_push_ds(ccProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_ds *value;				/* ds record data structure */
	ccProperties *ret;	/* return value in case the list is not empty	*/

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_ds*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "keytag");
			if ((ret = epp_property_push_int(c_props, str, value->keytag, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "alg");
			if ((ret = epp_property_push_int(c_props, str, value->alg, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "digestType");
			if ((ret = epp_property_push_int(c_props, str, value->digestType, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "digest");
			if ((ret = epp_property_push(c_props, str, value->digest, false, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "maxSigLife");
			if ((ret = epp_property_push_int(c_props, str, value->maxSigLife, false)) == NULL) {
				return NULL;
			}
		}
		return ret;
	} else {
		return c_props;
	}

}

/**
 * ############################
 * Add qhead with xml errors to properties
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param list		list of xml elements and error messages
 * @param list_name	base name for the inserted properties
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */
ccProperties *epp_property_push_valerr(ccProperties *c_props, qhead *list, char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_error *value;			/* ds record data structure */
	ccProperties *ret;	/* return value in case the list is not empty	*/

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_error*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "element");
			if ((ret = epp_property_push(c_props, str, value->value, true, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "reason");
			if ((ret = epp_property_push(c_props, str, value->reason, true, false)) == NULL) {
				return NULL;
			}

		}
		return ret;
	} else {
		return c_props;
	}

}

/**
 * Add qhead with ns records to properties
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param list		list of ns records
 * @param list_name	base name for the inserted properties
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */
ccProperties *epp_property_push_nsset(ccProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_ns *value;				/* ds record data structure */
	ccProperties *ret;	/* return value in case the list is not empty	*/

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_ns*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "name");
			if ((ret = epp_property_push(c_props, str, value->name, false, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "addr");
			if ((ret = epp_property_push_qhead(c_props, &value->addr, str, false, true)) == NULL) {
				return NULL;
			}
		}
		return ret;
	} else {
		return c_props;
	}

}

/**
 * Add dnskey list to log item properties
 *
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param list		list of dnskey records
 * @param list_name	base name for the inserted properties
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */
ccProperties *epp_property_push_dnskey(ccProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH];
	epp_dnskey *value;
	ccProperties *ret;

	if (q_length(*list) > 0) {
		q_foreach(list) {
			value = (epp_dnskey*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "flags");
			if ((ret = epp_property_push_int(c_props, str, value->flags, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "protocol");
			if ((ret = epp_property_push_int(c_props, str, value->protocol, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "alg");
			if ((ret = epp_property_push_int(c_props, str, value->alg, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "publicKey");
			if ((ret = epp_property_push(c_props, str, value->public_key, false, false)) == NULL) {
				return NULL;
			}

		}
		return ret;

	} else {
		return c_props;
	}

}

/**
 * Log an epp command using fred-logd service. Raw content as well as
 * parsed values inserted as properties are sent to the logging facility
 *
 * @param	service 	a reference to the logging service CORBA object
 * @param	c			connection record
 * @param	request		raw content of the request
 * @param 	cdata		command data, parsed content
 * @param   cmdtype 	command type returned by parse_command function
 * @param 	sessionid   login id for the session
 *
 * @return  status
 */
auto_ptr<ccProperties> log_epp_command(epp_command_data *cdata, epp_red_command_type cmdtype, int sessionid)
{
#define PUSH_PROPERTY(seq, name, value)								\
	seq = epp_property_push(seq, name, value, false, false);	\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

#define PUSH_PROPERTY_INT(seq, name, value)							\
	seq = epp_property_push_int(seq, name, value, false);		\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

#define PUSH_QHEAD(seq, list, name)									\
	seq = epp_property_push_qhead(seq, list, name, false, false);	\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

	int res;								/* response from corba call wrapper */
	const char *cmd_name = NULL;					/* command name to be used
												one of the basic properties */
	char errmsg[MAX_ERROR_MSG_LEN];			/* error message returned from corba call */
	ccProperties *c_props = NULL;	/* properties to be sent to the log */
	/* data structures for every command */
	epps_create_contact *cc;
	epps_create_domain *cd;
	epps_create_nsset *cn;
	epps_create_keyset *ck;
	epps_delete *ed;
	epps_renew *er;
	epps_update_contact *uc;
	epps_update_domain *ud;
	epps_update_nsset *un;
	epps_update_keyset *uk;
	epps_transfer *et;
	epps_login *el;
	epps_check *ec;

	// first allocate some space for c_props
	c_props = new ccProperties;
	if (c_props == NULL) {
			throw bad_alloc();
	}
	c_props->_maximum = ALLOC_STEP;
	c_props->_buffer = (ccLogProperty*)malloc(c_props->_maximum * sizeof(ccLogProperty));
	if (c_props->_buffer == NULL) {
			delete c_props;
			throw bad_alloc();
	}
	c_props->_length = 0;

	errmsg[0] = '\0';
	if(cdata->type == EPP_DUMMY) {
		PUSH_PROPERTY (c_props, "command", "dummy");
		PUSH_PROPERTY (c_props, "clTRID", cdata->clTRID);
		PUSH_PROPERTY (c_props, "svTRID", cdata->svTRID);

		// TODO
		// res = epp_log_new_message(request, c_props, &errmsg);
		return auto_ptr<ccProperties>(c_props);
	}

	switch(cmdtype) {
		case EPP_RED_LOGIN:
			if (cdata->type == EPP_LOGIN){
				cmd_name = "login";

				el = static_cast<epps_login*>(cdata->data);

				PUSH_PROPERTY(c_props, "registrarId", el->clID);
				// type epp_lang:
				if (el->lang == LANG_CS) {
					PUSH_PROPERTY(c_props, "lang", "CZ");
				} else if (el->lang == LANG_EN) {
					PUSH_PROPERTY(c_props, "lang", "EN");
				} else {
					PUSH_PROPERTY_INT(c_props, "lang", el->lang);
				}
				PUSH_PROPERTY(c_props, "password", el->pw);
				PUSH_PROPERTY(c_props, "newPassword", el->newPW);
			} else {
				return auto_ptr<ccProperties>(c_props);
			}
			break;

		case EPP_RED_LOGOUT:
			cmd_name = "logout";
			break;

		case EPP_RED_CHECK:
			cmd_name = "check";
			ec = static_cast<epps_check*>(cdata->data);
			PUSH_QHEAD(c_props, &ec->ids, "checkId");
			break;

		case EPP_RED_INFO:

			switch(cdata->type) {
				case EPP_LIST_CONTACT:
					cmd_name = "listContact";
					break;
				case EPP_LIST_KEYSET:
					cmd_name = "listKeyset";
					break;
				case EPP_LIST_NSSET:
					cmd_name = "listNsset";
					break;
				case EPP_LIST_DOMAIN:
					cmd_name = "listDomain";
					break;
				case EPP_INFO_CONTACT: {
					epps_info_contact *i = static_cast<epps_info_contact*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					cmd_name = "infoContact";
					break;
				}
				case EPP_INFO_KEYSET: {
					epps_info_keyset *i = static_cast<epps_info_keyset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					cmd_name = "infoKeyset";
					break;
				}
				case EPP_INFO_NSSET: {
					epps_info_nsset *i = static_cast<epps_info_nsset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					cmd_name = "infoNsset";
					break;
				}
				case EPP_INFO_DOMAIN: {
					epps_info_domain *i = static_cast<epps_info_domain*>(cdata->data);

					PUSH_PROPERTY(c_props, "name", i->name)
					cmd_name = "infoDomain";
					break;
				}
				default:
					break;
			}
			break;

		case EPP_RED_POLL:
			cmd_name = "poll";
			if(cdata->type == EPP_POLL_ACK) {
				epps_poll_ack *pa = static_cast<epps_poll_ack*>(cdata->data);
				PUSH_PROPERTY(c_props, "msgId", pa->msgid);
			}
			break;

		case EPP_RED_CREATE:
			switch(cdata->type) {
				case EPP_CREATE_CONTACT:
					cmd_name = "createContact";
					cc = static_cast<epps_create_contact*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", cc->id);

					// postal info
					if ((c_props = epp_log_postal_info(c_props, &cc->pi)) == NULL) {
						throw bad_alloc();
					}

					PUSH_PROPERTY(c_props, "voice", cc->voice);
					PUSH_PROPERTY(c_props, "fax", cc->fax);
					PUSH_PROPERTY(c_props, "email", cc->email);
					PUSH_PROPERTY(c_props, "authInfo", cc->authInfo);

					// disclose info
					if ((c_props = epp_log_disclose_info(c_props, &cc->discl)) == NULL) {
						throw bad_alloc();
					}

					PUSH_PROPERTY(c_props, "vat", cc->vat);
					PUSH_PROPERTY(c_props, "ident", cc->ident);
					switch(cc->identtype) {
						case ident_UNKNOWN: PUSH_PROPERTY(c_props, "identType", "unknown"); break;
						case ident_OP:      PUSH_PROPERTY(c_props, "identType", "ID card"); break;
						case ident_PASSPORT: PUSH_PROPERTY(c_props, "identType", "passport"); break;
						case ident_MPSV:    PUSH_PROPERTY(c_props, "identType", "number assinged by ministry"); break;
						case ident_ICO:     PUSH_PROPERTY(c_props, "identType", "ICO"); break;
						case ident_BIRTHDAY: PUSH_PROPERTY(c_props, "identType", "birthdate"); break;
					}
					PUSH_PROPERTY(c_props, "notifyEmail", cc->notify_email);
						// COMMON

					break;

				case EPP_CREATE_DOMAIN:
					cmd_name = "createDomain";
					cd = static_cast<epps_create_domain*>(cdata->data);

					PUSH_PROPERTY(c_props, "name", cd->name);
					PUSH_PROPERTY(c_props, "registrant", cd->registrant);
					PUSH_PROPERTY(c_props, "nsset", cd->nsset);
					PUSH_PROPERTY(c_props, "keyset", cd->keyset);
					// qhead	 extensions;   /**< List of domain extensions.
					PUSH_PROPERTY(c_props, "authInfo", cd->authInfo);
					// COMMON

					PUSH_QHEAD(c_props, &cd->admin, "admin");
					PUSH_PROPERTY_INT(c_props, "period", cd->period);
					if (cd->unit == TIMEUNIT_MONTH) {
						PUSH_PROPERTY(c_props, "timeunit", "Month");
					} else if(cd->unit == TIMEUNIT_YEAR) {
						PUSH_PROPERTY(c_props, "timeunit", "Year");
					}
					PUSH_PROPERTY(c_props, "expirationDate", cd->exDate);
					break;

				case EPP_CREATE_NSSET:
					cmd_name = "createNsset";
					cn = static_cast<epps_create_nsset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", cn->id);
					PUSH_PROPERTY(c_props, "authInfo", cn->authInfo);
					PUSH_PROPERTY_INT(c_props, "reportLevel", cn->level);
									// COMMON
					if((c_props = epp_property_push_nsset(c_props, &cn->ns, "ns")) == NULL) {
						throw bad_alloc();
					}
					PUSH_QHEAD(c_props, &cn->tech, "techC");


					break;
				case EPP_CREATE_KEYSET:
					cmd_name = "createKeyset";
					ck = static_cast<epps_create_keyset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", ck->id);
					PUSH_PROPERTY(c_props, "authInfo", ck->authInfo);
					// COMMON

					if((c_props=epp_property_push_ds(c_props, &ck->ds, "ds")) == NULL) {
						throw bad_alloc();
					}
					if((c_props=epp_property_push_dnskey(c_props, &ck->keys, "keys")) == NULL) {
						throw bad_alloc();
						// throw bad_alloc();
					}

					PUSH_QHEAD(c_props, &ck->tech, "techContact");
					break;
				default:
					break;
			}

			break;
		case EPP_RED_DELETE:
			cmd_name = "delete";
			ed = static_cast<epps_delete*>(cdata->data);

			PUSH_PROPERTY(c_props, "id", ed->id);
			break;

		case EPP_RED_RENEW:
			cmd_name = "renew";
			er = static_cast<epps_renew*>(cdata->data);

			PUSH_PROPERTY(c_props, "name", er->name);
			PUSH_PROPERTY(c_props, "curExDate", er->curExDate);
			PUSH_PROPERTY_INT(c_props, "renewPeriod", er->period);
			if (er->unit == TIMEUNIT_MONTH) {
				PUSH_PROPERTY(c_props, "timeunit", "Month");
			} else if(cd->unit == TIMEUNIT_YEAR) {
				PUSH_PROPERTY(c_props, "timeunit", "Year");
			}
			PUSH_PROPERTY(c_props, "expirationDate", er->exDate);
			break;

		case EPP_RED_UPDATE:

			switch(cdata->type) {
				case EPP_UPDATE_CONTACT:
					cmd_name = "updateContact";

					uc = static_cast<epps_update_contact*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", uc->id);

					if ( (c_props=epp_log_postal_info(c_props, uc->pi)) == NULL) {
						throw bad_alloc();
					}

					PUSH_PROPERTY(c_props, "voice", uc->voice);
					PUSH_PROPERTY(c_props, "fax", uc->fax);
					PUSH_PROPERTY(c_props, "email", uc->email);
					PUSH_PROPERTY(c_props, "authInfo", uc->authInfo);

					if ( (c_props=epp_log_disclose_info(c_props, &uc->discl)) == NULL) {
						throw bad_alloc();
					}

					PUSH_PROPERTY(c_props, "vat", uc->vat);
					PUSH_PROPERTY(c_props, "ident", uc->ident);

					switch(uc->identtype) {
						case ident_UNKNOWN: PUSH_PROPERTY(c_props, "identType", "unknown"); break;
						case ident_OP:      PUSH_PROPERTY(c_props, "identType", "ID card"); break;
						case ident_PASSPORT: PUSH_PROPERTY(c_props, "identType", "passport"); break;
						case ident_MPSV:    PUSH_PROPERTY(c_props, "identType", "number assinged by ministry"); break;
						case ident_ICO:     PUSH_PROPERTY(c_props, "identType", "ICO"); break;
						case ident_BIRTHDAY: PUSH_PROPERTY(c_props, "identType", "birthdate"); break;
					}

					PUSH_PROPERTY(c_props, "notifyEmail", uc->notify_email);
						// COMMON
					break;

				case EPP_UPDATE_DOMAIN:
					cmd_name = "updateDomain";

					ud = static_cast<epps_update_domain*>(cdata->data);

					PUSH_PROPERTY(c_props, "name", ud->name);
					PUSH_PROPERTY(c_props, "registrant", ud->registrant);
					PUSH_PROPERTY(c_props, "nsset", ud->nsset);
					PUSH_PROPERTY(c_props, "keyset", ud->keyset);
					// qhead	 extensions;   /**< List of domain extensions.
					PUSH_PROPERTY(c_props, "authInfo", ud->authInfo);
					// COMMONs

					PUSH_QHEAD(c_props, &ud->add_admin, "addAdmin");
					PUSH_QHEAD(c_props, &ud->rem_admin, "remAdmin");
					PUSH_QHEAD(c_props, &ud->rem_tmpcontact, "remTmpcontact");

					break;

				case EPP_UPDATE_NSSET:
					cmd_name = "updateNsset";
					un = static_cast<epps_update_nsset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", un->id);
					PUSH_PROPERTY(c_props, "authInfo", un->authInfo);
					PUSH_PROPERTY_INT(c_props, "reportLevel", un->level);
					// COMMON

					PUSH_QHEAD(c_props, &un->add_tech, "addTechC");
					PUSH_QHEAD(c_props, &un->rem_tech, "remTechC");
					if((c_props = epp_property_push_nsset(c_props, &un->add_ns, "addNs")) == NULL) {
						throw bad_alloc();
					}
					PUSH_QHEAD(c_props, &un->rem_ns, "remNs");

					break;

				case EPP_UPDATE_KEYSET:
					cmd_name = "updateKeyset";
					uk = static_cast<epps_update_keyset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", uk->id);
					PUSH_PROPERTY(c_props, "authInfo", uk->authInfo);
					// COMMON

					PUSH_QHEAD(c_props, &uk->add_tech, "addTech");
					PUSH_QHEAD(c_props, &uk->rem_tech, "remTech");
					if((c_props = epp_property_push_ds(c_props, &uk->add_ds, "addDs")) == NULL) {
						throw bad_alloc();
					}
					if((c_props = epp_property_push_ds(c_props, &uk->rem_ds, "remDs")) == NULL) {
						throw bad_alloc();
					}

					if((c_props = epp_property_push_dnskey(c_props, &uk->add_dnskey, "addKeys")) == NULL) {
						throw bad_alloc();
					}
					if((c_props = epp_property_push_dnskey(c_props, &uk->rem_dnskey, "remKeys")) == NULL) {
						throw bad_alloc();
					}

					break;
			}
			break;

		case EPP_RED_TRANSFER:
			cmd_name = "transfer";
			et = static_cast<epps_transfer*>(cdata->data);

			PUSH_PROPERTY(c_props, "id", et->id);
			break;

		default:
			break;
	}

	PUSH_PROPERTY (c_props, "command", cmd_name);
  	PUSH_PROPERTY (c_props, "clTRID", cdata->clTRID);
	PUSH_PROPERTY (c_props, "svTRID", cdata->svTRID);

	return auto_ptr<ccProperties>(c_props);
	// res = epp_log_new_message(  request, c_props, &errmsg);

#undef PUSH_PROPERTY
#undef PUSH_PROPERTY_INT
#undef _QUOTE_STR
#undef INFO_CMD_CASE
}

