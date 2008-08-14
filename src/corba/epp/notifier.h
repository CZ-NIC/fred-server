#include "corba/mailer_manager.h"
#include "action.h"
#include "old_utils/dbsql.h"

#ifndef ID
#define ID unsigned  int
#endif

// type of object  1 contact  2 nsset 3 domain 4 keyset
#define OBJECT_CONTACT 1
#define OBJECT_NSSET   2
#define OBJECT_DOMAIN  3
#define OBJECT_KEYSET  4

#define OLD -1
#define NEW  1

#define REGISTRANT_CONTACT 1  // owner of fomain
#define ADMIN_CONTACT     2 // admin-c at the domain  
#define TECH_CONTACT      3 // tech-c at  nsset
#define MAIL_CONTACT      4 // transformed contacts 
#define KEY_CONTACT       5 // tech-c at keyset
typedef struct
{
  ID contactID; // id of the constact
  short type; // type of the contact admin or tech 
  short modify; // old  -1 , new 1 , not changed  0 
  std::string notifyEmail; // notify e-mail 
  std::string Email; //email 
} NotifyST;

// Notifier class 
class EPPNotifier
{
  MailerManager *mm;
  DB *db;
  std::vector<NotifyST> notify;

  short enum_action; // id fom  enum_action table EPP operation
  ID registrarID; // idof registrar who make change
  ID objectID; // type of the object  where make change

  bool disable; // for fast upload of objects
  std::string extraEmails; // for fixing notification of notifyEmail change
public:
  void addExtraEmails(const std::string& emails) 
  {
	  extraEmails += " ";
	  extraEmails += emails;
  }
  EPPNotifier(
    bool _disable, MailerManager *mailManager, DB *dbs, ID regid, ID objectid); // add default contacts for object ID
  ~EPPNotifier();
  bool Send(); // send e-mail messages by mailer manager

  ID GetObjectID() {return objectID;}
  ; // return ID of the object
  ID GetRegistrarID() {return registrarID;}
  ; //  return ID of the registrar


  void AddNSSetTechByDomain(
    ID domainID); // add ale tech-c of the nsset linked with domain with  SQL query
  void AddNSSetTech(
    ID nssetID); // add all tech-c with nsset  with  SQL query
  void AddKeySetTechByDomain(
          ID domainID); //add all tech-c of keyset linket with domain with SQL query
  void AddKeySetTech(
          ID keysetID); //add all tech-c with keyset with sql query
  void AddDomainAdmin(
    ID domainID); // add all admin-c at domain with  SQL query
  void AddDomainRegistrant(
    ID domainID); // add owner of domain with  SQL query

  void AddRegistrant(
    ID contactID)
  {
    AddContactID(contactID, REGISTRANT_CONTACT, 0);
  }
  ;
  void AddRegistrantNew(
    ID contactID)
  {
    AddContactID(contactID, REGISTRANT_CONTACT, NEW);
  }
  ;
  void AddRegistrantOld(
    ID contactID)
  {
    AddContactID(contactID, REGISTRANT_CONTACT, OLD);
  }
  ;

  void AddTech(
    ID contactID)
  {
    AddContactID(contactID, TECH_CONTACT, 0);
  }
  ;
  void AddTechNew(
    ID contactID)
  {
    AddContactID(contactID, TECH_CONTACT, NEW);
  }
  ;
  void AddTechOld(
    ID contactID)
  {
    AddContactID(contactID, TECH_CONTACT, OLD);
  }
  ;

  void AddAdmin(
    ID contactID)
  {
    AddContactID(contactID, ADMIN_CONTACT, 0);
  }
  ;
  void AddAdminNew(
    ID contactID)
  {
    AddContactID(contactID, ADMIN_CONTACT, NEW);
  }
  ;
  void AddAdminOld(
    ID contactID)
  {
    AddContactID(contactID, ADMIN_CONTACT, OLD);
  }
  ;

private:
  void AddContactID(
    ID contactID, short type, short mod); // add contactID to the list notify

  void AddDefault()
  {
    switch (enum_action) {
      case EPP_ContactCreate:
      case EPP_ContactUpdate:
      case EPP_ContactDelete:
      case EPP_ContactTransfer:
        AddContactID(objectID, 0, 0); // normal contact
        break;
      case EPP_DomainCreate:
      case EPP_DomainDelete:
      case EPP_DomainRenew:
      case EPP_DomainUpdate:
      case EPP_DomainTransfer:
        AddDomainRegistrant(objectID); // owner of domain
        AddDomainAdmin(objectID); // admin-c of domain
        break;
      case EPP_NSsetCreate:
      case EPP_NSsetUpdate:
      case EPP_NSsetDelete:
      case EPP_NSsetTransfer:
        AddNSSetTech(objectID); // tech-c of the nsset
        break;
      case EPP_KeySetCreate:
      case EPP_KeySetUpdate:
      case EPP_KeySetDelete:
      case EPP_KeySetTransfer:
        AddKeySetTech(objectID); // tech-c of keyset
        break;
    }

  }

  // is it new contact for logger
  const char *GetContactModify(
    short mod)
  {
    if (mod > mod)
      return "new!";
    else
      return "";
  }
  ;

  // type oft the contact for logger
  const char *GetContactType(
    short type)
  {
    switch (type) {
      case REGISTRANT_CONTACT:
        return "registrant: ";
      case TECH_CONTACT:
        return "tech-c: ";
      case ADMIN_CONTACT:
        return "admin-c: ";
      case MAIL_CONTACT:
        return "mail-c: ";
      default:
        return "unknow: ";
    }

  }
  ;

  // name of the tamplete from mail_type table
  const char * getTemplate()
  {
    switch (enum_action) {
      case EPP_ContactUpdate:
      case EPP_NSsetUpdate:
      case EPP_DomainUpdate:
      case EPP_KeySetUpdate:
        return "notification_update";

      case EPP_ContactTransfer:
      case EPP_NSsetTransfer:
      case EPP_DomainTransfer:
      case EPP_KeySetTransfer:
        return "notification_transfer";

      case EPP_ContactCreate:
      case EPP_NSsetCreate:
      case EPP_DomainCreate:
      case EPP_KeySetCreate:
        return "notification_create";

      case EPP_DomainRenew:
        return "notification_renew";

      case EPP_DomainDelete:
      case EPP_NSsetDelete:
      case EPP_ContactDelete:
      case EPP_KeySetDelete:
        return "notification_delete";

      default:
        return "unknow_template";

    }

  }

};
