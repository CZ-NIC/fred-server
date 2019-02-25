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
/**
 * @file epp_parser.h
 * Interface to component which parses xml documents and returns document's
 * content in form of a structure.
 */
#ifndef M_EPP_PARSER_HH_B14652141FE5443B9628A0E3E3007DD8
#define M_EPP_PARSER_HH_B14652141FE5443B9628A0E3E3007DD8

/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file epp_common.h
 *
 * The most important structures, function definitions and routine declarations
 * are found in this file. Since they are used by all components of mod_eppd,
 * they are most important and should be read first when trying to understand
 * to the module's code.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <memory>

#include "tools/logd_migration/migrate.hh"

/** Standard EPP xml namespace */
#define NS_EPP	"urn:ietf:params:xml:ns:epp-1.0"
/** Our custom namespace used for contact object */
#define NS_CONTACT	"http://www.nic.cz/xml/epp/contact-1.5"
/** Our custom namespace used for domain object */
#define NS_DOMAIN	"http://www.nic.cz/xml/epp/domain-1.4"
/** Our custom namespace used for nsset object */
#define NS_NSSET	"http://www.nic.cz/xml/epp/nsset-1.2"
/** Our custom namespace used for keyset object */
#define NS_KEYSET	"http://www.nic.cz/xml/epp/keyset-1.2"
/** Our custom namespace used for extensions definition */
#define NS_FRED		"http://www.nic.cz/xml/epp/fred-1.4"
/** Our custom namespace used for enum validation extension */
#define NS_ENUMVAL	"http://www.nic.cz/xml/epp/enumval-1.1"
/** Namespace + location of epp xml schema */
#define LOC_EPP	NS_EPP " epp-1.0.xsd"
/** Namespace + location of contact xml schema */
#define LOC_CONTACT	NS_CONTACT " contact-1.5.xsd"
/** Namespace + location of domain xml schema */
#define LOC_DOMAIN	NS_DOMAIN " domain-1.4.xsd"
/** Namespace + location of nsset xml schema */
#define LOC_NSSET	NS_NSSET " nsset-1.2.xsd"
/** Namespace + location of keyset xml schema */
#define LOC_KEYSET	NS_KEYSET " keyset-1.2.xsd"
/** Namespace + location of fred xml schema */
#define LOC_FRED	NS_FRED " fred-1.3.xsd"
/** Namespace + location of enumval xml schema */
#define LOC_ENUMVAL	NS_ENUMVAL " enumval-1.1.xsd"






/**
 * Enumaration of statuses returned by validator.
 */
typedef enum {
	VAL_OK,        /**< Document is valid. */
	VAL_NOT_VALID, /**< Document does not validate. */
	VAL_ESCHEMA,   /**< Error when loading or parsing schema. */
	VAL_EINTERNAL  /**< Internal error (malloc failed). */
}valid_status;

// --- ^ ripped from epp_xmlcommon.h


/** Log levels used for logging to eppd log file. */
typedef enum {
	EPP_FATAL = 1,/**< Error, the module is not in operational state. */
	EPP_ERROR,    /**< Error caused usually by client, module is operational. */
	EPP_WARNING,  /**< Errors which are not serious but should be logged. */
	EPP_INFO,     /**< This is the default log level. */
	EPP_DEBUG     /**< Contents of requests and responses are logged. */
}epp_loglevel;

/** Maximum property name length for fred-logd logging facility */
#define LOG_PROP_NAME_LENGTH 50

/** EPP context is a group of variables used often together.
 *
 * The two items inside the struct are void pointers because we don't want
 * to export apache datatypes in all other modules sharing this header file.
 */
typedef struct {
	void *pool; 	/**< Pool for allocations */
	void *conn;	/**< Connection handler */
	int session;
}epp_context;

/**
 * Enumeration of codes of all EPP commands this module is able to handle.
 * The object specific commands are written as EPP_{command}_{object}.
 */
typedef enum {
	EPP_UNKNOWN_CMD = 0,
	/*
	 * 'dummy' is not a command from point of view of epp client, but is
	 * command from central repository's point of view
	 */
	EPP_DUMMY,
	/* session commands */
	EPP_LOGIN,
	EPP_LOGOUT,
	/* query commands */
	EPP_CHECK_CONTACT,
	EPP_CHECK_DOMAIN,
	EPP_CHECK_NSSET,
	EPP_CHECK_KEYSET,
	EPP_INFO_CONTACT,
	EPP_INFO_DOMAIN,
	EPP_INFO_NSSET,
	EPP_INFO_KEYSET,
	EPP_LIST_CONTACT,
	EPP_LIST_DOMAIN,
	EPP_LIST_NSSET,
	EPP_LIST_KEYSET,
	EPP_POLL_REQ,
	EPP_POLL_ACK,
	/* transform commands */
	EPP_CREATE_CONTACT,
	EPP_CREATE_DOMAIN,
	EPP_CREATE_NSSET,
	EPP_CREATE_KEYSET,
	EPP_DELETE_CONTACT,
	EPP_DELETE_DOMAIN,
	EPP_DELETE_NSSET,
	EPP_DELETE_KEYSET,
	EPP_UPDATE_CONTACT,
	EPP_UPDATE_DOMAIN,
	EPP_UPDATE_NSSET,
	EPP_UPDATE_KEYSET,
	EPP_TRANSFER_CONTACT,
	EPP_TRANSFER_DOMAIN,
	EPP_TRANSFER_NSSET,
	EPP_TRANSFER_KEYSET,
	EPP_RENEW_DOMAIN,
	/* protocol extensions */
	EPP_SENDAUTHINFO_CONTACT,
	EPP_SENDAUTHINFO_DOMAIN,
	EPP_SENDAUTHINFO_NSSET,
	EPP_SENDAUTHINFO_KEYSET,
	EPP_TEST_NSSET,
	EPP_CREDITINFO,
	/* info functions */
	EPP_INFO_LIST_CONTACTS,
	EPP_INFO_LIST_DOMAINS,
	EPP_INFO_LIST_NSSETS,
	EPP_INFO_LIST_KEYSETS,
	EPP_INFO_DOMAINS_BY_NSSET,
	EPP_INFO_DOMAINS_BY_KEYSET,
	EPP_INFO_DOMAINS_BY_CONTACT,
	EPP_INFO_NSSETS_BY_CONTACT,
	EPP_INFO_NSSETS_BY_NS,
	EPP_INFO_KEYSETS_BY_CONTACT,
	EPP_INFO_GET_RESULTS
}epp_command_type;

/**
 * Enumeration of implemented extensions.
 */
typedef enum {
	EPP_EXT_ENUMVAL
}domain_ext_type;

/**
 * Enumeration of EPP objects which this server operates on.
 */
typedef enum {
	EPP_UNKNOWN_OBJ = 0,
	EPP_CONTACT,
	EPP_DOMAIN,
	EPP_NSSET,
	EPP_KEYSET
}epp_object_type;

/**
 * definition of languages (english is default)
 */
typedef enum {
	LANG_EN	= 0,
	LANG_CS,
}epp_lang;

/**
 * In case that central repository finds out that some parameter is bad,
 * there has to be way how to propagate this information back to client.
 * The standard requires that client provided value has to be surrounded
 * with xml tags, of which the central repository is not aware. Therefore
 * exact specification of errors is needed.
 */
typedef enum {
	errspec_poll_msgID = 0,
	errspec_contact_handle,
	errspec_contact_cc,
	errspec_nsset_handle,
	errspec_nsset_tech,
	errspec_nsset_dns_name,
	errspec_nsset_dns_addr,
	errspec_nsset_dns_name_add,
	errspec_nsset_dns_name_rem,
	errspec_nsset_tech_add,
	errspec_nsset_tech_rem,
	errspec_keyset_handle,
  	errspec_keyset_tech,
  	errspec_keyset_dsrecord,
 	errspec_keyset_dsrecord_add,
  	errspec_keyset_dsrecord_rem,
	errspec_keyset_dnskey,
	errspec_keyset_dnskey_add,
	errspec_keyset_dnskey_rem,
  	errspec_keyset_tech_add,
  	errspec_keyset_tech_rem,
	errspec_registrar_author,
	errspec_domain_fqdn,
	errspec_domain_registrant,
	errspec_domain_nsset,
	errspec_domain_keyset,
	errspec_domain_period,
	errspec_domain_admin,
	errspec_domain_tmpcontact,
	errspec_domain_ext_valDate,
	errspec_domain_ext_valDate_missing,
	errspec_domain_curExpDate,
	errspec_domain_admin_add,
	errspec_domain_admin_rem,
	/* input errors */
	errspec_not_valid,
	errspec_poll_msgID_missing,
	errspec_contact_identtype_missing,
	errspec_transfer_op
}epp_errorspec;

typedef enum {
	ClientLogin = 100,
	ClientLogout = 101,
	PollAcknowledgement = 120,
	PollResponse = 121,
	ContactCheck = 200,
	ContactInfo = 201,
	ContactDelete = 202,
	ContactUpdate = 203,
	ContactCreate = 204,
	ContactTransfer = 205,
	NSsetCheck = 400,
	NSsetInfo = 401,
	NSsetDelete = 402,
	NSsetUpdate = 403,
	NSsetCreate = 404,
	NSsetTransfer = 405,
	DomainCheck = 500,
	DomainInfo = 501,
	DomainDelete = 502,
	DomainUpdate = 503,
	DomainCreate = 504,
	DomainTransfer = 505,
	DomainRenew = 506,
	DomainTrade = 507,
	KeysetCheck = 600,
	KeysetInfo = 601,
	KeysetDelete = 602,
	KeysetUpdate = 603,
	KeysetCreate = 604,
	KeysetTransfer = 605,
	UnknownAction = 1000,
	ListContact = 1002,
	ListNSset = 1004,
	ListDomain = 1005,
	ListKeySet = 1006,
	ClientCredit = 1010,
	nssetTest = 1012,
	ContactSendAuthInfo = 1101,
	NSSetSendAuthInfo = 1102,
	DomainSendAuthInfo = 1103,
	Info = 1104,
	GetInfoResults = 1105,
	KeySetSendAuthInfo = 1106,
        InfoListContacts = 1200,
        InfoListDomains = 1201,
        InfoListNssets = 1202,
        InfoListKeysets = 1203,
        InfoDomainsByNsset = 1204,
        InfoDomainsByKeyset = 1205,
        InfoDomainsByContact = 1206,
        InfoNssetsByContact = 1207,
        InfoNssetsByNs = 1208,
        InfoKeysetsByContact = 1209,
        InfoGetResults = 1210
} epp_action_type;


/**
 * The struct represents one epp error in ExtValue element. It is either
 * validation error (in that case surrounding tags are contained in value
 * and standalone is set to 1) or error reported by central repository
 * (in that case surrounding tags are missing and has to be completed
 * according to #epp_errorspec value).
 */
typedef struct {
	/** Client provided input which caused the error. */
	char	*value;
	/**
	 * Specification of surrounding XML tags.
	 *
	 * For validation errors this is set to errspec_not_valid.
	 */
	epp_errorspec spec;
	/**
	 * Human readable reason of error.
	 *
	 * For schema validity errors it is filled by mod_eppd (by message from
	 * libxml) which is prefixed by localized message retrieved from
	 * central registry. In all other cases it is left empty and filled
	 * by CR.
	 */
	char	*reason;
	/** Position of faulty element if it is part of list. */
	int	 position;
}epp_error;

/**
 * @defgroup queuegroup Queue structure and utilities
 * @{
 */

/**
 * Queue item type.
 */
typedef struct queue_item_t qitem;
/**
 * Definition of queue item type.
 */
struct queue_item_t {
	qitem   *next;	  /**< Link to next item in a queue. */
	void	*content; /**< Pointer to content of item. */
};

/**
 * Queue structure used on countless places throughout the program.
 *
 * It is one way linked list of items, consisting of two parts: head and body.
 */
typedef struct {
	int	 count;     /**< Optimization for length() function. */
	qitem	*body;      /**< Items in a queue. */
	qitem	*cur;       /**< Currently selected item. */
}qhead;

/** Get length of a queue. */
#define q_length(_qhead)	((_qhead).count)
/** Shift to next item in a queue. */
#define q_next(_qhead)	\
	((_qhead)->cur = ((_qhead)->cur) ? (_qhead)->cur->next : NULL)
/** Get content of current item. */
#define q_content(_qhead)	((_qhead)->cur->content)
/** Reset current item to the first one. */
#define q_reset(_qhead)	((_qhead)->cur = (_qhead)->body)
/**
 * Iterate through items in a list. cl advances each round to next item in
 * list, until the sentinel is encountered. Caller must be sure that the
 * list pointer is at the beginning when using this macro - use cl_reset
 * for that.
 */
#define q_foreach(_qhead)	\
	for ((_qhead)->cur = (_qhead)->body; (_qhead)->cur != NULL; (_qhead)->cur = (_qhead)->cur->next)
/**
 * Add new item to a queue (the item will be enqueued at the end of queue).
 *
 * @param pool    Pool from which the new item will be allocated.
 * @param head    The queue.
 * @param data    Pointer to data which shoud be enqueued.
 * @return        0 if successfull, otherwise 1.
 */
int q_add(void *pool, qhead *head, void *data);

/** @} */


/* ********************************************************************* */


/**
 * Structure for holding status' names and values.
 */
typedef struct {
	char	*value;	/**< Status name. */
	char	*text;  /**< Status value. */
}epp_status;

/**
 * Structure gathers postal info about contact.
 */
typedef struct {
	char	*name;  /**< Name. */
	char	*org;	/**< Organization. */
	qhead	 streets; /**< 3x street. */
	char	*city;  /**< City. */
	char	*sp;	/**< State or province. */
	char	*pc;	/**< Postal code. */
	char	*cc;	/**< Country code. */
}epp_postalInfo;

/**
 * Disclose information of contact.
 * All items except flag inside are treated as booleans.
 * Value 1 means it is an exception to data collection policy.
 * Flag represents the default server policy.
 * Example: if server data collection policy is "public" (flag == 0)
 * 	then value 1 in this structure means the item should be hidden.
 */
typedef struct {
	/**
	 * Value 1 means following items are exception to server policy, which
	 * is assumed to be private (hide all items).
	 * Value 0 means following items are exception to server policy, which
	 * is assumed to be public (show all items).
	 * And value -1 means there are not elements that require exceptional
	 * behaviour.
	 */
	char	flag;
	unsigned char	name; /**< Contact's name is exceptional. */
	unsigned char	org;  /**< Contact's organization is exceptional. */
	unsigned char	addr; /**< Contact's address is exceptional. */
	unsigned char	voice;/**< Contact's voice (tel. num.) is exceptional. */
	unsigned char	fax;  /**< Contact's fax number is exceptional. */
	unsigned char	email;/**< Contact's email address is exceptional. */
	unsigned char	vat;  /**< Contact's VAT is exceptional. */
	unsigned char	ident;/**< Contact's ident is exceptional. */
	/** Contact's notification emai is exceptional. */
	unsigned char	notifyEmail;
}epp_discl;

/**
 * Nameserver has a name and possibly more than one ip address.
 */
typedef struct {
	char	*name;	 /**< fqdn of nameserver. */
	qhead	 addr; /**< List of ip addresses. */
}epp_ns;

/**
 * Delegation signer information, together with public key information.
 * For more detailed information about the individual fields see
 * RFC 4310.
 */
typedef struct {
	unsigned short	keytag;  	/**< Key tag for DNSKEY RR (RFC 4043 for details) */
	unsigned char	alg;		/**< Algorithm type */
	unsigned char	digestType;	/**< Digest type (must be SHA-1) */
	char	*digest;		/**< Key digest (currently it must a valid SHA-1 */

	int	maxSigLife;		/**< Signature expiration period, zero means that the field is empty */
	/* optional dns rr (-1 in theese fields means that they are empty) */
/*
	unsigned flags;
	unsigned protocol;
	unsigned key_alg;
	char	*pubkey;
*/
} epp_ds;

/** DNS Key record - http://rfc-ref.org/RFC-TEXTS/4034/chapter2.html */
typedef struct {
	unsigned short flags;		/**< key properties. supported values are 0, 256, 257 */
	unsigned char  protocol;	/**< = 3 */
	unsigned char  alg;		/**< algorithm type */
	char 	*public_key;		/**< base64 encoded public key */
} epp_dnskey;

/** Type of identification number used in contact object. */
typedef enum {
	ident_UNKNOWN, /**< Unknown value can also mean undefined. */
	ident_OP,      /**< Number of ID card. */
	ident_PASSPORT,/**< Number of passport. */
	ident_MPSV,    /**< Number assigned by "ministry of work and ...". */
	ident_ICO,     /**< ICO. */
	ident_BIRTHDAY /**< Date of birth. */
}epp_identType;

typedef enum {
	TIMEUNIT_MONTH,
	TIMEUNIT_YEAR
}epp_timeunit;

/** Structure holding answer to EPP check command. */
typedef struct {
	int	 avail;  /**< True if object is available, false otherwise. */
	char	*reason; /**< If object is not available, here is the reason. */
}epp_avail;

/** Structure holding answer to EPP creditInfo command. */
typedef struct {
	char	*zone;   /**< True if object is available, false otherwise. */
	unsigned long credit; /**< Credit in cents. */
}epp_zonecredit;

/** DNSSEC extension used for updates. */
typedef struct {
	qhead	chg_ds; /**< Signatures to be changed. */
	qhead	add_ds; /**< Signatures to be added. */
	qhead	rem_ds; /**< Signatures to be removed. */
}epp_ext_domain_upd_dnssec;

typedef struct {
	domain_ext_type extType; /**< Identifier of extension. */
	union {
		char	*ext_enumval; /**< Domain validation.*/
		qhead	 ext_dnssec_cr; /** List of digital sigs for domain. */
		epp_ext_domain_upd_dnssec ext_dnssec_upd; /**< DNSSEC. */
	}ext; /**< Extension. */
}epp_ext_item;

/** Type of poll message. */
typedef enum {
	pt_transfer_contact, /**< Contact was transferred. */
	pt_delete_contact,   /**< Contact was deleted because not used. */
	pt_transfer_nsset,   /**< Nsset was transferred. */
	pt_delete_nsset,     /**< Nsset was deleted because not used. */
	pt_transfer_keyset,  /**< KeySet was transferred. */
	pt_delete_keyset,    /**< KeySet was deleted because not used. */
	pt_techcheck,        /**< Technical check results. */
	pt_transfer_domain,  /**< Domain was transferred. */
	pt_impexpiration,    /**< Domain will expire in near future. */
	pt_expiration,       /**< Domain expired. */
	pt_impvalidation,    /**< Domain validation will expire soon. */
	pt_validation,       /**< Domain validation expired. */
	pt_outzone,          /**< Domain was outaged from zone. */
	pt_delete_domain,    /**< Domain was deleted. */
	pt_lowcredit,        /**< Credit of registrator is low. */
}epp_pollType;

/** Structure containing result of one technical test. */
typedef struct {
	char	*testname;
	int	 status;
	char	*note;
}epp_testResult;

/* ********************************************************************* */


/** Login parameters. */
typedef struct {
	char	*clID;   /**< Client ID. */
	char	*pw;     /**< Password. */
	char	*newPW;  /**< New password. */
	qhead	 objuri; /**< currently not used */
	qhead	 exturi; /**< currently not used */
	unsigned lang;   /**< Language. */
}epps_login;

/** Check contact, domain and nsset parameters. */
typedef struct {
	qhead	ids;    /**< IDs of checked objects. */
	qhead	avails; /**< Booleans + reasons. */
}epps_check;

/** Info contact parameters. */
typedef struct {
	char	*id;       /**< Id of wanted contact (input). */
	char	*handle;   /**< Id of wanted contact (output).*/
	char	*roid;     /**< ROID of object. */
	qhead	 status;   /**< Contact's status. */
	epp_postalInfo pi; /**< Postal info. */
	char	*voice;    /**< Telephone number. */
	char	*fax;      /**< Fax number. */
	char	*email;    /**< Email address. */
	char	*clID;     /**< Owner's ID. */
	char	*crID;     /**< ID of creator. */
	char	*crDate;   /**< Creation date. */
	char	*upID;     /**< ID of last updater. */
	char	*upDate;   /**< Last updated. */
	char	*trDate;   /**< Last transfered. */
	char	*authInfo; /**< Authorization information. */
	epp_discl discl;   /**< Disclose information section. */
	char	*vat;      /**< VAT tax ID. */
	char	*ident;      /**< Contact's unique ident. */
	epp_identType identtype;   /**< Type of unique ident. */
	char	*notify_email; /**< Notification email. */
}epps_info_contact;

/** Info domain parameters. */
typedef struct {
	char	*name;    /**< FQDN of wanted domain (input). */
	char	*handle;  /**< FQDN of wanted domain (output). */
	char	*roid;    /**< ROID of object. */
	qhead	 status;  /**< Domain's status. */
	char	*registrant; /**< Registrant of domain. */
	qhead	 tmpcontact; /**< Temporary contact used for migration. */
	qhead	 admin;   /**< Admin contact for domain. */
	char	*nsset;   /**< Nsset of domain. */
	char 	*keyset;  /**< Keyset for domain */
	char	*clID;    /**< Owner's ID. */
	char	*crID;    /**< ID of creator. */
	char	*crDate;  /**< Creation date. */
	char	*exDate;  /**< Expiration date. */
	char	*upID;    /**< ID of last updater. */
	char	*upDate;  /**< Last updated. */
	char	*trDate;  /**< Last transfered. */
	char	*authInfo;/**< Authorization information. */
	qhead	 extensions; /**< List of domain extensions. */
}epps_info_domain;

/** Info nsset parameters. */
typedef struct {
	char	*id;      /**< Id of wanted nsset (input). */
	char	*handle;  /**< Id of wanted nsset (output). */
	char	*roid;    /**< ROID of object. */
	qhead	 status;  /**< Nsset's status. */
	char	*clID;    /**< Owner's ID. */
	char	*crID;    /**< ID of creator. */
	char	*crDate;  /**< Creation date. */
	char	*upID;    /**< ID of last updater. */
	char	*upDate;  /**< Last updated. */
	char	*trDate;  /**< Last transfered. */
	char	*authInfo;/**< Authorization information. */
	qhead	 ns;      /**< List of nameservers. */
	qhead	 tech;    /**< List of technical contacts for nsset. */
	int	 level;   /**< Report level. */
}epps_info_nsset;

/** Info keyset parameters */
typedef struct {
	char	*id;      /**< Id of wanted keyset (input). */
	char	*handle;  /**< Id of wanted keyset (output). */
	char	*roid;    /**< ROID of object. */
	qhead	 status;  /**< Keyset's status. */
	char	*clID;    /**< Owner's ID. */
	char	*crID;    /**< ID of creator. */
	char	*crDate;  /**< Creation date. */
	char	*upID;    /**< ID of last updater. */
	char	*upDate;  /**< Last updated. */
	char	*trDate;  /**< Last transfered. */
	char	*authInfo;/**< Authorization information. */
	qhead	 ds;      /**< List of delegation signers. */
	qhead 	 keys;	  /**< List of DNS Key records */
	qhead	 tech;    /**< List of technical contacts for keyset. */
} epps_info_keyset;

/** Poll request parameters. */
typedef struct {
	int	 count;    /**< Count of waiting messages. */
	char	*msgid;    /**< ID of next message in a queue. */
	char	*qdate;    /**< Date of message submission. */
	epp_pollType type; /**< Type of poll message. */
	union {
		char	*handle;
		struct {
			char	*handle;
			char	*date;
			char	*clID;
		}hdt; /**< Handle, date, registrator structure. */
		struct {
			char	*handle;
			char	*date;
		}hd; /**< Handle, date structure. */
		struct {
			char	*handle;
			qhead	 fqdns;
			qhead	 tests;
		}tc; /**< Structure with results of technical tests. */
		struct {
			char	*zone;
			unsigned long limit;
			unsigned long credit;
		}lc; /**< Low credit structure. */
	}msg; 	/**< Message data. */
}epps_poll_req;

/** Poll acknoledge parameters. */
typedef struct {
	char	*msgid;   /**< ID of acknoledged message. */
	int	 count;   /**< Count of waiting messages. */
	char	*newmsgid;/**< ID of first message in a queue. */
}epps_poll_ack;

/** Create contact parameters. */
typedef struct {
	char	*id;       /**< Id of wanted contact (input). */
	epp_postalInfo pi; /**< Postal info. */
	char	*voice;    /**< Telephone number. */
	char	*fax;      /**< Fax number. */
	char	*email;    /**< Email address. */
	char	*authInfo; /**< Authorization information. */
	epp_discl discl;   /**< Disclose information section. */
	char	*vat;      /**< VAT tax ID. */
	char	*ident;    /**< Contact's unique ident. */
	epp_identType identtype;/**< Type of unique ident. */
	char	*notify_email;  /**< Notification email. */
	char	*crDate;   /**< Creation date of contact. */
}epps_create_contact;

/** Create domain parameters. */
typedef struct {
	char	*name;    /**< FQDN of wanted domain (input). */
	char	*registrant; /**< Registrant of domain. */
	qhead	 admin;   /**< Admin contact for domain. */
	char	*nsset;   /**< Nsset of domain. */
	char 	*keyset;  /**< Keyset for domain */
	int	 period;  /**< Registration period in months. */
	epp_timeunit unit;/**< Registration period's unit. */
	char	*authInfo;/**< Authorization information. */
	qhead	 extensions; /**< List of domain extensions. */
	char	*crDate;  /**< Creation date of domain. */
	char	*exDate;  /**< Expiration date of domain. */
}epps_create_domain;

/** Create nsset parameters. */
typedef struct {
	char	*id;      /**< Id of wanted nsset (input). */
	char	*authInfo;/**< Authorization information. */
	qhead	 ns;      /**< List of nameservers. */
	qhead	 tech;    /**< List of technical contacts for nsset. */
	char	*crDate;  /**< Creation date of nsset. */
	int	 level;   /**< Report level. */
}epps_create_nsset;

/** Create keyset parameters */
typedef struct {
	char 	*id;		/**< Id of wanted keyset (input). */
	char 	*authInfo;	/**< Authorization information. */
	qhead	ds;		/**< List of delegation signers */
	qhead   keys;		/**< List of DNS Key records */
	qhead 	tech;		/**< List of technical contacts for keyset */
	char 	*crDate;	/**< Creation date of keyset. */
}epps_create_keyset;

/** Delete parameters. */
typedef struct {
	char	*id;      /**< ID of object to be deleted. */
}epps_delete;

/** Renew domain parameters. */
typedef struct {
	char	*name;      /**< Name of renewed domain. */
	char	*curExDate; /**< Current expiration date. */
	int	 period;    /**< Renew period. */
	epp_timeunit unit;  /**< Registration period's unit. */
	qhead	 extensions;/**< List of domain extensions. */
	char	*exDate;    /**< New expiration date. */
}epps_renew;

/** Update contact parameters. */
typedef struct {
	char	*id;            /**< Id of wanted contact (input). */
	epp_postalInfo *pi;     /**< Postal info. */
	char	*voice;         /**< Telephone number. */
	char	*fax;           /**< Fax number. */
	char	*email;         /**< Email address. */
	char	*authInfo;      /**< Authorization information. */
	epp_discl discl;        /**< Disclose information section. */
	char	*vat;           /**< VAT tax ID. */
	char	*ident;         /**< Contact's unique ident. */
	epp_identType identtype;/**< Type of unique ident. */
	char	*notify_email;  /**< Notification email. */
}epps_update_contact;

/** Update domain parameters. */
typedef struct {
	char	*name;         /**< FQDN of wanted domain (input). */
	char	*registrant;   /**< Registrant of domain. */
	qhead	 add_admin;    /**< Admin contacts to be added. */
	qhead	 rem_admin;    /**< Admin contacts to be removed. */
	qhead	 rem_tmpcontact; /**< Temporary contact used for migration. */
	char	*nsset;        /**< Nsset of domain. */
	char	*keyset;       /**< Keyset of domain. */
	char	*authInfo;     /**< Authorization information. */
	qhead	 extensions;   /**< List of domain extensions. */
}epps_update_domain;

/** Update nsset parameters. */
typedef struct {
	char	*id;           /**< Id of wanted nsset (input). */
	qhead	 add_tech;     /**< Technical contacts to be added. */
	qhead	 rem_tech;     /**< Technical contacts to be removed. */
	qhead	 add_ns;       /**< Nameservers to be added. */
	qhead	 rem_ns;       /**< Nameservers to be removed. */
	char	*authInfo;     /**< Authorization information. */
	int	 level;        /**< Report level. */
}epps_update_nsset;

/** Update keyset parameters */
typedef struct {
	char	*id;           /**< Id of wanted keyset (input). */
	qhead	 add_tech;     /**< Technical contacts to be added. */
	qhead	 rem_tech;     /**< Technical contacts to be removed. */
	qhead	 add_ds;       /**< Delegation signers to be added. */
	qhead	 rem_ds;       /**< Delegation signers to be removed. */
	qhead 	 add_dnskey;   /**< DNSKEYs to be added. */
	qhead 	 rem_dnskey;   /**< DNSKEYs to be removed. */
	char	*authInfo;     /**< Authorization information. */
}epps_update_keyset;

/** Transfer parameters. */
typedef struct {
	char	*id;           /**< Id of transfered object. */
	char	*authInfo;     /**< Authorization information. */
}epps_transfer;

/** SendAuthInfo parameters. */
typedef struct {
	char	*id;          /**< Handle of object. */
}epps_sendAuthInfo;

/** CreditInfo parameters. */
typedef struct {
	qhead	 zonecredits; /**< List of credits for individual zones. */
}epps_creditInfo;

/** Test parameters. */
typedef struct {
	char	*id;    /**< ID of tested nsset. */
	qhead	 names; /**< Fqdns of domains to be tested with nsset. */
	int	 level; /**< Level of tests (-1 if not overriden). */
}epps_test;

/** Parameters of obsolete command 'list' and getResults command. */
typedef struct {
	qhead	 handles;     /**< List of handles. */
}epps_list;

/**
 * All Info functions, which accept single key on input and count on
 * output (domainsByNsset, domainsByContact, nssetsByContact, nssetsByNs).
 * This structure is used also by new list functions, handle is NULL for them.
 */
typedef struct {
	char	*handle;      /**< Search key. */
	unsigned int count;   /**< Count of results. */
}epps_info;

/**
 * This structure is central to the concept of the whole module. The
 * communication among module's components is done through this structure.
 * It gathers outputs of parsing stage and serves as input/output
 * for corba function call stage and after that as input for response
 * generation stage. Structure fits for all kinds of possible commands and
 * their extensions. The structure is self-identifing, which means, that
 * it holds information about which command it holds.
 */
typedef struct {
	char	*clTRID;/**< client's TRID */
	char	*svTRID;/**< server's TRID */
	int	 	rc;    /**< EPP return code defined in standard. */
	char	*msg;   /**< Text message coresponding to return code. */
	char	*xml_in;/**< XML as it is received from client. */
	/* parsed_doc and xpath_ctx are needed for error reporting. */
	void	*parsed_doc; /**< Parsed XML document tree. */
	void	*xpath_ctx;  /**< XPath context. */
	/** True if there should be no resdata section or msgQ section. */
	short	 noresdata;
	/** List of validation errors or errors from central repository. */
	qhead	 errors;

	/**
	 * Identification of epp command. This value influences selection
	 * from in and out union.
	 */
	epp_command_type type;
	/**
	 * Command data
	 * (Input + output parameters for all possible epp commands).
	 */
	void	*data;
}epp_command_data;


/* ********************************************************************* */

/**
 * Write a log message to eppd log file.
 *
 * @param epp_ctx EPP context structure (connection, pool and session id).
 * @param level   Log level.
 * @param fmt     Printf-style format string.
 */
void epplog(epp_context *epp_ctx, epp_loglevel level, const char *fmt, ...);

/**
 * @defgroup allocgroup Functions for memory allocation.
 *
 * A memory allocated by these functions is automatically freed when
 * processing of request is finished.
 *
 * @{
 */

/**
 * Allocate memory from memory pool.
 *
 * @param pool    Memory pool.
 * @param size    Number of bytes to allocate.
 * @return        Pointer to allocated memory.
 */
void *epp_malloc(void *pool, unsigned size);

/**
 * Allocate memory from memory pool and prezero it.
 *
 * @param pool   Memory pool.
 * @param size   Number of bytes to allocate.
 * @return       Pointer to allocated memory.
 */
void *epp_calloc(void *pool, unsigned size);

/**
 * Duplicate string from argument, the memory will be allocated from
 * memory pool.
 *
 * @param pool   Memory pool.
 * @param str    String which is going to be duplicated.
 * @return       Pointer duplicated string.
 */
char *epp_strdup(void *pool, const char *str);

/**
 * Concatenate two strings in arguments, the memory will be allocated from
 * memory pool.
 *
 * In case of memory allocation failure or if one of arguments is NULL
 * the function returns NULL.
 *
 * @param pool   Memory pool.
 * @param str1   String which will be the first one.
 * @param str2   String which will be appended.
 * @return       Pointer to new string.
 */
char *epp_strcat(void *pool, const char *str1, const char *str2);

/**
 * Print formatted string.
 *
 * @param pool   Memory pool.
 * @param fmt    Format of string.
 * @return       Formatted string allocated from pool.
 */
char *epp_sprintf(void *pool, const char *fmt, ...);

/** EPP parser status values.
 *
 * The ordering of items in enumeration is important, because relation greater,
 * smaller is used for resolution between cases when connection should be
 * closed and when it shouldn't.
 */
typedef enum {
	PARSER_CMD_LOGIN,  /**< Login command. */
	PARSER_CMD_LOGOUT, /**< Logout command. */
	PARSER_CMD_OTHER,  /**< A command other than login and logout. */
	PARSER_NOT_VALID,  /**< Request does not validate. */
	/**
	 * Request is not command but <hello> frame this indicates that greeting
	 * should be generated.
	 */
	PARSER_HELLO,
	/*
	 * when following status values are returned, connection is closed
	 */
	PARSER_NOT_COMMAND,/**< Request is not a command nor hello frame. */
	PARSER_NOT_XML,    /**< Request is not xml. */
	PARSER_ESCHEMA,    /**< Error when parsing xml schema. */
	/**
	 * Internal parser error (e.g. malloc failed). This error is
	 * esspecialy serious, therefor its log severity SHOULD be higher
	 * than of the other errors.
	 */
	PARSER_EINTERNAL
} parser_status;

/**
 * Enumeration of all implemented EPP commands as defined in rfc.
 * It is REDuced form - without object suffix. And is used as hash
 * value in command hash table for fast recognition of commands.
 */
typedef enum {
	EPP_RED_UNKNOWN_CMD,
	EPP_RED_LOGIN,
	EPP_RED_LOGOUT,
	EPP_RED_CHECK,
	EPP_RED_INFO,
	EPP_RED_POLL,
	EPP_RED_TRANSFER,
	EPP_RED_CREATE,
	EPP_RED_DELETE,
	EPP_RED_RENEW,
	EPP_RED_UPDATE,
        EPP_RED_EXTCMD
}epp_red_command_type;

char * wrap_str(const char *str);

/**
 * This routine initializes libxml's parser, hash table for command
 * recognition and parses xml schema, which is returned.
 *
 * @param url_schema  XML schema location.
 * @return            Parsed xml schema.
 */
void *
epp_parser_init(const char *url_schema);

/**
 * This will cleanup command hash table, libxml's parser and release
 * parsed xml schema.
 *
 * @param schema    Parsed xml schema.
 */
void
epp_parser_init_cleanup(void *schema);

int
read_epp_dnskey(void *pool, xmlXPathContextPtr xpathCtx, epp_dnskey *key);

/**
 * This is the main workhorse of parser component. It's task is to parse
 * request and get data saved in structure.
 *
 * @param epp_ctx   Epp context (pool, connection and session id).
 * @param loggedin  True if client is logged in.
 * @param schema    Parsed xml schema used for validation.
 * @param request   Request to be processed.
 * @param bytes     Length of the request.
 * @param cdata     Output of parsing stage (xml data converted to structure).
 * @param cmd_type	Output of commnad type (used also by logging)
 * @return          Status of parsing.
 */
parser_status
epp_parse_command(
		const char *request,
		unsigned bytes,
		epp_command_data **cdata,
		epp_red_command_type *cmd_type);

/**
 * This will cleanup xpath context and parsed document tree.
 *
 * @param cdata_arg    cdata structure containing items to be cleaned up.
 */
/*
void
epp_parser_request_cleanup(void *cdata_arg);
*/

std::unique_ptr<LibFred::Logger::RequestProperties> log_epp_command(epp_command_data *cdata, epp_red_command_type cmdtype, int sessionid, epp_action_type *request_type_id);


typedef	std::list<void*> pool_subst;

class pool_manager {
private:
pool_subst * pool;

public:
	pool_manager(pool_subst *p) {
		pool = p;
	}	
	~pool_manager() {
		pool_subst::iterator it = pool->begin();	

		for(;it != pool->end();it++) {
			free (*it); //i know, i know, it shouldn't be delete, but free()'d
		}
		
		pool->clear();	
	}
};

extern pool_subst *mem_pool;

void epp_parser_request_cleanup(void *cdata_arg);

#endif /* M_EPP_PARSER_H */
