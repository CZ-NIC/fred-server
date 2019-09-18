/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <string>
#include "tools/logd_migration/m_epp_parser.hh"
#include "tools/logd_migration/migrate.hh"

/* strlen, strcpy */ 
#include <string.h> 

#define ALLOC_STEP 4

using namespace std;

char * wrap_str(const char *str)
{
	char *ret;
	int len = strlen(str) + 1;

	ret = (char*)malloc(len);
	if (ret == NULL) return NULL;

	mem_pool->push_back(ret);
	strncpy(ret, str, len);
	return ret;
}

// this would be much nicer
// typedef list<RequestProperty> RequestProperties;

#define MAX_ERROR_MSG_LEN 1024
/**
 * Add a name, value pair to the properties. Allocate memory and the property list itself
 * on demand
 * @param c_props	log entry properties or a NULL pointer (in which
 * 					case a new data structure is allocated and returned)
 * @param name		property name
 * @param value		property value
 * @param child 	true if the property is child to the last property with child = false
 *
 * @returns			NULL in case of an allocation error, modified c_props otherwise
 */
LibFred::Logger::RequestProperties *epp_property_push(LibFred::Logger::RequestProperties *c_props, const  char *name, const char *value, bool child)
{
	if(c_props == NULL) {
		c_props = new LibFred::Logger::RequestProperties();
	}

	if (value != NULL) {
		LibFred::Logger::RequestProperty p;

		p.name =  name;
		p.value = value;
		p.child = child;

		c_props->push_back(p);
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
 * @param child		true if the items in the list are children of the last property
 * 					with child = false
 *
 * @returns 		log entry properties or NULL in case of an allocation error
 *
 */

LibFred::Logger::RequestProperties *epp_property_push_qhead(LibFred::Logger::RequestProperties *c_props, qhead *list, const char *list_name, bool child)
{
	LibFred::Logger::RequestProperties *ret = NULL;

	if (list->count == 0) {
		return c_props;
	}

	q_foreach(list) {
		if ((ret = epp_property_push(c_props, list_name, (char*)q_content(list), child)) == NULL) {
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
 *
 * @returns			NULL in case of an allocation error, modified c_props otherwise
 */

LibFred::Logger::RequestProperties *epp_property_push_int(LibFred::Logger::RequestProperties *c_props, const char *name, int value)
{
	LibFred::Logger::RequestProperty p;
	char str[12];

	if(c_props == NULL) {
		c_props = new LibFred::Logger::RequestProperties;
	}

	snprintf(str, 12, "%i", value);

	p.name = wrap_str(name);
	p.value = wrap_str(str);
	p.child = false;

	c_props->push_back(p);

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
LibFred::Logger::RequestProperties *epp_log_postal_info(LibFred::Logger::RequestProperties *p, epp_postalInfo *pi)
{
	if(pi == NULL) return p;

	p = epp_property_push(p, "pi.name", pi->name, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.organization", pi->org, false);
	if (p == NULL) return p;
	p = epp_property_push_qhead(p, &pi->streets, "pi.street", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.city", pi->city, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.state", pi->sp, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.postalCode", pi->pc, false);
	if (p == NULL) return p;
	p = epp_property_push(p, "pi.countryCode", pi->cc, false);
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
LibFred::Logger::RequestProperties *epp_log_disclose_info(LibFred::Logger::RequestProperties *p, epp_discl *ed)
{
	if(ed->flag == 1) {
		p = epp_property_push(p, "discl.policy", "private", false);
	} else if(ed->flag == 0) {
		p = epp_property_push(p, "discl.policy", "public", false);
	} else {
		p = epp_property_push(p, "discl.policy", "no exceptions", false);
	}

	if (p == NULL) return p;

	p = epp_property_push(p, "discl.name", ed->name ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.org", ed->org ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.addr", ed->addr ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.voice", ed->voice ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.fax", ed->fax ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.email", ed->email ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.vat", ed->vat ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.ident", ed->ident ? "true" : "false", false);
	if (p == NULL) return p;
	p = epp_property_push(p, "discl.notifyEmail", ed->notifyEmail ? "true" : "false", false);
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
LibFred::Logger::RequestProperties *epp_property_push_ds(LibFred::Logger::RequestProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_ds *value;				/* ds record data structure */

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_ds*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "keytag");
			if ((c_props = epp_property_push_int(c_props, str, value->keytag)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "alg");
			if ((c_props = epp_property_push_int(c_props, str, value->alg)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "digestType");
			if ((c_props = epp_property_push_int(c_props, str, value->digestType)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "digest");
			if ((c_props = epp_property_push(c_props, str, value->digest, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "maxSigLife");
			if ((c_props = epp_property_push_int(c_props, str, value->maxSigLife)) == NULL) {
				return NULL;
			}
		}
        }
        return c_props;

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
LibFred::Logger::RequestProperties *epp_property_push_valerr(LibFred::Logger::RequestProperties *c_props, qhead *list, char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_error *value;			/* ds record data structure */

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_error*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "element");
			if ((c_props = epp_property_push(c_props, str, value->value, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "reason");
			if ((c_props = epp_property_push(c_props, str, value->reason, false)) == NULL) {
				return NULL;
			}

		}
        }
        return c_props;

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
LibFred::Logger::RequestProperties *epp_property_push_nsset(LibFred::Logger::RequestProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH]; /* property name */

	epp_ns *value;				/* ds record data structure */

	if (q_length(*list) > 0) {

		q_foreach(list) {
			value = (epp_ns*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "name");
			if ((c_props = epp_property_push(c_props, str, value->name, false)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "addr");
			if ((c_props = epp_property_push_qhead(c_props, &value->addr, str, true)) == NULL) {
				return NULL;
			}
		}
        }
        return c_props;

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
LibFred::Logger::RequestProperties *epp_property_push_dnskey(LibFred::Logger::RequestProperties *c_props, qhead *list, const char *list_name)
{
	char str[LOG_PROP_NAME_LENGTH];
	epp_dnskey *value;

	if (q_length(*list) > 0) {
		q_foreach(list) {
			value = (epp_dnskey*)q_content(list);

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "flags");
			if ((c_props = epp_property_push_int(c_props, str, value->flags)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "protocol");
			if ((c_props = epp_property_push_int(c_props, str, value->protocol)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "alg");
			if ((c_props = epp_property_push_int(c_props, str, value->alg)) == NULL) {
				return NULL;
			}

			str[0] = '\0';
			snprintf(str, LOG_PROP_NAME_LENGTH, "%s.%s", list_name, "publicKey");
			if ((c_props = epp_property_push(c_props, str, value->public_key, false)) == NULL) {
				return NULL;
			}

		}
        }
        return c_props;

}

/**
 * Log an epp command using fred-logd service. Raw content as well as
 * parsed values inserted as properties are sent to the logging facility
 *
 * @param 	cdata		command data, parsed content
 * @param   	cmdtype 	command type returned by parse_command function
 * @param 	sessionid   login id for the session
 * @param	request_type_id	to be removed - already in request table
 *
 * @return  status
 */
unique_ptr<LibFred::Logger::RequestProperties> log_epp_command(epp_command_data *cdata, epp_red_command_type cmdtype, int /*sessionid*/, epp_action_type *request_type_id)
{
#define PUSH_PROPERTY(seq, name, value)								\
	seq = epp_property_push(seq, name, value, false);	\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

#define PUSH_PROPERTY_INT(seq, name, value)							\
	seq = epp_property_push_int(seq, name, value);		\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

#define PUSH_QHEAD(seq, list, name)									\
	seq = epp_property_push_qhead(seq, list, name, false);	\
	if(seq == NULL) {	\
		throw bad_alloc();	\
	}

	//int res;								/* response from corba call wrapper */
												

	LibFred::Logger::RequestProperties *c_props = NULL;	/* properties to be sent to the log */
	/* data structures for every command */
	epps_sendAuthInfo *ai;
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

	c_props = new LibFred::Logger::RequestProperties;
	

	if(cdata->type == EPP_DUMMY) {
		*request_type_id = (epp_action_type)999900;
		PUSH_PROPERTY (c_props, "command", "dummy");
		PUSH_PROPERTY (c_props, "clTRID", cdata->clTRID);
		PUSH_PROPERTY (c_props, "svTRID", cdata->svTRID);

		// res = epp_log_new_message(request, c_props, &errmsg);
		return unique_ptr<LibFred::Logger::RequestProperties>(c_props);
	}

	switch(cmdtype) {
		case EPP_RED_LOGIN:
			if (cdata->type == EPP_LOGIN){
				*request_type_id = ClientLogin;

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
				ai = static_cast<epps_sendAuthInfo*>(cdata->data);

				switch(cdata->type) {
					case EPP_SENDAUTHINFO_CONTACT:
						*request_type_id = ContactSendAuthInfo;

						break;
					case EPP_SENDAUTHINFO_DOMAIN:
						*request_type_id = DomainSendAuthInfo;

						break;
					case EPP_SENDAUTHINFO_NSSET:
						*request_type_id = NSSetSendAuthInfo;

						break;
					case EPP_SENDAUTHINFO_KEYSET:
						*request_type_id = KeySetSendAuthInfo;

						break;
					default:
						break;
				}

				PUSH_PROPERTY(c_props, "id", ai->id);

				return unique_ptr<LibFred::Logger::RequestProperties>(c_props);
			}
			break;

		case EPP_RED_LOGOUT:
			*request_type_id = ClientLogout;

			break;

		case EPP_RED_CHECK:
			switch(cdata->type) {
				case EPP_CHECK_CONTACT:
					*request_type_id = ContactCheck;
					break;
				case EPP_CHECK_DOMAIN:
					*request_type_id = DomainCheck;
					break;
				case EPP_CHECK_NSSET:
					*request_type_id = NSsetCheck;
					break;
				case EPP_CHECK_KEYSET:
					*request_type_id = KeysetCheck;
					break;
                default:
                    break;
			}

			ec = static_cast<epps_check*>(cdata->data);
			PUSH_QHEAD(c_props, &ec->ids, "checkId");
			break;

		case EPP_RED_INFO:

			switch(cdata->type) {
				case EPP_LIST_CONTACT:
					*request_type_id = ListContact;

					break;
				case EPP_LIST_KEYSET:
					*request_type_id = ListKeySet;

					break;
				case EPP_LIST_NSSET:
					*request_type_id = ListNSset;

					break;
				case EPP_LIST_DOMAIN:
					*request_type_id = ListDomain;

					break;
				case EPP_INFO_CONTACT: {
					epps_info_contact *i = static_cast<epps_info_contact*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					*request_type_id = ContactInfo;

					break;
				}
				case EPP_INFO_KEYSET: {
					epps_info_keyset *i = static_cast<epps_info_keyset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					*request_type_id = KeysetInfo;

					break;
				}
				case EPP_INFO_NSSET: {
					epps_info_nsset *i = static_cast<epps_info_nsset*>(cdata->data);

					PUSH_PROPERTY(c_props, "id", i->id)
					*request_type_id = NSsetInfo;

					break;
				}
				case EPP_INFO_DOMAIN: {
					epps_info_domain *i = static_cast<epps_info_domain*>(cdata->data);

					PUSH_PROPERTY(c_props, "name", i->name)
					*request_type_id = DomainInfo;

					break;
				}
				default:
					break;
			}
			break;

		case EPP_RED_POLL:

			if(cdata->type == EPP_POLL_ACK) {
				*request_type_id = PollAcknowledgement;
				epps_poll_ack *pa = static_cast<epps_poll_ack*>(cdata->data);
				PUSH_PROPERTY(c_props, "msgId", pa->msgid);
			} else {
				*request_type_id = PollResponse;
			}
			break;

		case EPP_RED_CREATE:
			switch(cdata->type) {
				case EPP_CREATE_CONTACT:
					*request_type_id = ContactCreate;

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
					*request_type_id = DomainCreate;

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
					*request_type_id = NSsetCreate;

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
					*request_type_id = KeysetCreate;

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
					*request_type_id = (epp_action_type)999901;
					break;
			}

			break;
		case EPP_RED_DELETE:
			switch(cdata->type) {
				case EPP_DELETE_CONTACT:
					*request_type_id = ContactDelete;
					break;
				case EPP_DELETE_DOMAIN:
					*request_type_id = DomainDelete;
					break;
				case EPP_DELETE_NSSET:
					*request_type_id = NSsetDelete;
					break;
				case EPP_DELETE_KEYSET:
					*request_type_id = KeysetDelete;
					break;
				default:
					*request_type_id = (epp_action_type)999902;
					break;
			}

			ed = static_cast<epps_delete*>(cdata->data);

			PUSH_PROPERTY(c_props, "id", ed->id);
			break;

		case EPP_RED_RENEW:
			*request_type_id = DomainRenew;

			er = static_cast<epps_renew*>(cdata->data);

			PUSH_PROPERTY(c_props, "name", er->name);
			PUSH_PROPERTY(c_props, "curExDate", er->curExDate);
			PUSH_PROPERTY_INT(c_props, "renewPeriod", er->period);

			if (er->unit == TIMEUNIT_MONTH) {
				PUSH_PROPERTY(c_props, "timeunit", "Month");
			} else if(er->unit == TIMEUNIT_YEAR) {
				PUSH_PROPERTY(c_props, "timeunit", "Year");
			}
			PUSH_PROPERTY(c_props, "expirationDate", er->exDate);
			break;

		case EPP_RED_UPDATE:

			switch(cdata->type) {
				case EPP_UPDATE_CONTACT:
					*request_type_id = ContactUpdate;


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
					*request_type_id = DomainUpdate;


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
					*request_type_id = NSsetUpdate;

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
					*request_type_id = KeysetUpdate;

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
				default:
					// TODO remove these things  ...
					*request_type_id = (epp_action_type)999903;
					break;
			}
			break;

		case EPP_RED_TRANSFER:
			switch(cdata->type) {
				case EPP_TRANSFER_CONTACT:
					*request_type_id = ContactTransfer;
					break;
				case EPP_TRANSFER_DOMAIN:
					*request_type_id = DomainTransfer;
					break;
				case EPP_TRANSFER_NSSET:
					*request_type_id = NSsetTransfer;
					break;
				case EPP_TRANSFER_KEYSET:
					*request_type_id = KeysetTransfer;
					break;
				default:
					*request_type_id = (epp_action_type)999904;
			}


			et = static_cast<epps_transfer*>(cdata->data);

			PUSH_PROPERTY(c_props, "id", et->id);
			break;

                case EPP_RED_EXTCMD:
                default:
                    switch (cdata->type) {
                        case EPP_TEST_NSSET:
                            *request_type_id = nssetTest;
                            break;
                        case EPP_SENDAUTHINFO_CONTACT:
                            *request_type_id = ContactSendAuthInfo;
                            break;
                        case EPP_SENDAUTHINFO_DOMAIN:
                            *request_type_id = DomainSendAuthInfo;
                            break;
                        case EPP_SENDAUTHINFO_NSSET:
                            *request_type_id = NSSetSendAuthInfo;
                            break;
                        case EPP_SENDAUTHINFO_KEYSET:
                            *request_type_id = KeySetSendAuthInfo;
                            break;
                        case EPP_CREDITINFO:
                            *request_type_id = ClientCredit;
                            break;
                        case EPP_INFO_LIST_DOMAINS:
                            *request_type_id = InfoListDomains;
                            break;
                        case EPP_INFO_LIST_CONTACTS:
                            *request_type_id = InfoListContacts;
                            break;
                        case EPP_INFO_LIST_KEYSETS:
                            *request_type_id = InfoListKeysets;
                            break;
                        case EPP_INFO_LIST_NSSETS:
                            *request_type_id = InfoListNssets;
                            break;
                        case EPP_INFO_DOMAINS_BY_NSSET:
                            *request_type_id = InfoDomainsByNsset;
                            break;
                        case EPP_INFO_DOMAINS_BY_KEYSET:
                            *request_type_id = InfoDomainsByKeyset;
                            break;
                        case EPP_INFO_DOMAINS_BY_CONTACT:
                            *request_type_id = InfoDomainsByContact;
                            break;
                        case EPP_INFO_NSSETS_BY_NS:
                            *request_type_id = InfoNssetsByNs;
                            break;
                        case EPP_INFO_NSSETS_BY_CONTACT:
                            *request_type_id = InfoNssetsByContact;
                            break;
                        case EPP_INFO_GET_RESULTS:
                            *request_type_id = InfoGetResults;
                            break;
                        case EPP_INFO_KEYSETS_BY_CONTACT:
                            *request_type_id = InfoKeysetsByContact;
                            break;
                        default:
                            break;
                    }
                    break;		
	}

        // TODO cmd_name unused
	// PUSH_PROPERTY (c_props, "command", cmd_name);
  	PUSH_PROPERTY (c_props, "clTRID", cdata->clTRID);
	PUSH_PROPERTY (c_props, "svTRID", cdata->svTRID);


	return unique_ptr<LibFred::Logger::RequestProperties>(c_props);
	// res = epp_log_new_message(  request, c_props, &errmsg);

#undef PUSH_PROPERTY
#undef PUSH_PROPERTY_INT
#undef _QUOTE_STR
#undef INFO_CMD_CASE
}

