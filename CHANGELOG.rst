CHANGELOG
=========

UNRELEASED
----------

* Remove duplicate ``libfred`` tests

* Remove obsolete ``getRequestCount*`` (moved to LoggerCorba)

2022-07-28 (2.53.1)
-------------------

* Invoicing

  - switch from ``doc2pdf`` to ``secretary/libtypist``
  - switch from ``pyfred/mailer`` to ``messenger/libhermes``
  - switch from ``pyfred/filemanager`` to ``fileman/libfiled``

2022-05-19 (2.52.0)
-------------------

* Public requests - switch from mailer/pyfred and fileman to messenger and libfiled

2022-04-29 (2.51.0)
-------------------

* Add ``--send_notifications`` option to fred-admin ``--object_regular_procedure`` call. Default is not to send email notifications.

2022-02-28 (2.50.0)
-------------------

* Epp - generate update poll messages in all update operations when not done by sponsoring registrar

* Domain browser - normalize string data of contact (empty string / null values / whitespace) when finding merge candidates

* Rework object factories to manual registration instead of auto-registration which was not working properly

* Fix unix whois registrar search (upper case registrar handle)

2022-02-01 (2.49.0)
-------------------

* Add possibility to mark registrar as internal

  - not in registrar list in public interfaces but still traceable when linked to another object
  - not charged

* Epp - on succesfull ``info_contact`` command used with authinfo parameter (to get all contact private data)
        generate new authinfo password (same behaviour as ``transfer_*`` commands)

* Epp / Public request - allow to send contact authinfo on registry e-mail even when contact has ``serverTransferProhibited`` status

* Update CMake build

2021-12-22 (2.48.2)
-------------------

* Fix mojeid - add specific exception (instead of internal server error) when transferring contact attached to external identity

* Fix domain browser contact merge candidate list - exclude contacts attached to external identity

* Fix rpm build (fedora 35) and remove support for CentOS7

2021-12-14 (2.48.1)
-------------------

* Quick fix - auto registration tests in admin. contact verification

2021-11-08 (2.48.0)
-------------------

* Add concept of external identity (service) that can be linked to registry contact

* Modify domain browser backend to be usable with both mojeid contacts and contacts linked to external identity service

* Add new contact states to inform that some of the contact attributes are locked and cannot be changed

  - appropriate changes in epp contact update / contact auto merge procedure

2021-11-05 (2.47.1)
-------------------

* Fix rpm build (fedora 35) and remove support for CentOS7

2021-05-18 (2.47.0)
-------------------

* Add configuration for epp backend to specify privacy policy for contact data between registrars

* Add optional authinfo parameter to info contact method

2021-08-09 (2.46.2)
-------------------

* Remove explicit python call in external command running ``filemanager_client``

* Fix update contact poll message additional recipients (bump ``libfred``)

2021-06-14 (2.46.1)
-------------------

* Public request processing - add ``registrar_url`` parameter to epp sendauthinfo e-mail template context

2021-04-21 (2.46.0)
-------------------

* Whois backend - Optimize info domain method (bump libfred)

* MojeID - Add ``auto_pin3_sending`` configuration option

2021-03-18 (2.45.9)
-------------------

* Fix invoice list - loading of related objects

2021-02-23 (2.45.8)
-------------------

* Fix authinfo sending when not confirmed automatically

2020-12-15 (2.45.7)
-------------------

* Fix poll message unittests

* Fix ``add_credit_to_invoice`` function (when used without outer transaction)

* Fix authinfo e-mail type when requested through epp

* Fix CI

2020-11-02 (2.45.6)
-------------------

* Fix retrieving registrars blocked "today"

2020-10-20 (2.45.5)
-------------------

* Validators as inline functions

2020-10-01 (2.45.4)
-------------------

* Fix missing parameter validators

2.45.3 (2020-08-13)
-------------------

* Add domain_lifecycle_parameters commands into fred-admin

2.45.2 (2020-08-11)
-------------------

* Fix domain filter performance issue

2.45.1 (2020-08-07)
-------------------

* Fix domain filter for Daphne

2.45.0 (2020-07-13)
-------------------

* Use parameters from new table domain_lifecycle_parameters

2.44.0 (2020-02-20)
-------------------

* MojeID

  * add new method for validated data update (no mojeid verification states canceling)

  * fix exception in send_new_pin3 method

2.43.0 (2020-02-03)
-------------------

* Add man pages for main ``fred-admin`` commands

2.42.3 (2020-01-31)
-------------------

* Fix rpm for RHEL8 and F31

2.42.2 (2020-01-28)
-------------------

* fred-admin - add ``--charge-to-end-of-previous-month`` option to ``charge_registry_access_fee_*`` commands

2.42.1 (2020-01-08)
-------------------

* Invoice export

  * add administrative fee operation

  * remove generic fine operation

2.42.0 (2019-09-11)
-------------------

* fred-admin

  * new commands for charging registrar access fee

    * ``charge_registry_access_fee_annual``

    * ``charge_registry_access_fee_monthly``

  * fix ``registrar_add_zone`` command - prevent duplicate records

  * fix ``registrar_add`` command - unhandled exception on invalid varsymbol

* Adapt to new random data generator interface

* Registrar credit manipulation and initialization using new
  libfred ``CreateRegistrarCreditTransaction`` operation

* adifd - return possibility to change registrar handle

* Update spec file for F31 and Centos/RHEL 8

2.41.2 (2019-11-27)
-------------------

* Add Fee and Fine operations to invoice export

* Fix export for account invoices with not paid amount

2.41.1 (2019-11-08)
-------------------

* VAT calculation changes

2.41.0 (2019-07-18)
-------------------

* Epp - create poll message when contact or domain is deleted administratively (to notify sponsoring registrar)

* Epp, MojeID - create poll message when contact is changed to:

  * sponsoring registrar of this contact if change was done by other than sponsoring registrar

  * sponsoring registrars of domains where this contact is assigned as holder or admin-c

* Epp - fix contact data change detection (to avoid dropping contact verification states)

* MojeID - fix contact address discloseflag change due to loss of contact
           verification states in `update_transfer_contact_prepare` method

2.40.3 (2019-07-11)
-------------------

* Fix configuration according to documented instalation procedure

2.40.2 (2019-06-19)
-------------------

* Fix rpm build (fedora 30)

2.40.1 (2019-06-10)
-------------------

* MojeID - fix contact data change detection (to avoid dropping contact verification states)

* Whois - fix log serverity for failed idn conversion of domain name

2.40.0 (2019-04-26)
-------------------

* Accounting (fred-accifd) - payment import

  * add optional custom tax date when specifying registrar manually

  * fix - use account date as tax date when matching registrar automatically

2.39.1 (2019-05-02)
-------------------

* Fix CMake (relative path for LIBFRED_DIR)

2.39.0 (2019-03-20)
-------------------

* License GNU GPLv3+

* CMake fixes

* Autotools removal

* Library libfred moved to separate repository (easier to reuse)

* Removed old database model structures and rewriting them to libfred operations (phase 2)

* Unique registrar payment identifier

* Administrative domain blocking/unblocking

  * when unblocking domain, unblock also linked contact (only if possible - must
    be also admin. blocked and not linked to another admin. blocked domain)

* Contact verification / MojeID

  * do not cancel identifiedContact and validatedContact flags on only letter case change in contact name

* CI fixes

2.38.3 (2019-02-26)
-------------------

* Fix mojeid method for sending new pin3 (always create new request)

2.38.2 (2019-02-11)
-------------------

* Add systemd services for fedora packages

2.38.1 (2019-01-15)
-------------------

* Fix accounting (fred-accifd) - getting registrar by payment data (invalid data and logging)

2.38.0 (2018-08-16)
-------------------

* epp disclose flags defaults configuration

* mojeid

  * join fist/last name to name

  * check for registrar configuration

* public request impl. refactoring

  * asynchronous processing of authinfo and block/unblock requests by fred-admin command

  * status enum renaming

  * new verification type for requests (government)

* new interface for accounting (registrar credit) - (phase 1)

  * bank payments moved to separate project (django-pain, fred-pain)

  * backend will manage only registrar credit transactions through this interface (fred-accifd)

  * preparations for moving invoices as well

* fix - whois nameserver validity check

* fixes in random generator initialization (tests, call id logging)

* removed code duplicity

* removed old database model structures and rewriting them to libfred operations (phase 1)

2.37.1 (2018-05-17)
-------------------

* bugfix of authinfo sending (automatic on registry e-mail) for multiple recipients

* fix csv serializer (escaping)

2.37.0 (2018-04-20)
-------------------

* quick fix to change default disclose policy to hide (will be revisited)

* public request interface - add impl. for personal info request

* epp - update contact poll message type

* fix registrar credit record initialization (after zone access is granted)

2.36.0 (2018-04-14)
-------------------

* switch to new common date/time and buffer data types in idl

* fix - record statement internal server error for not registered objects

* fix - epp contact update - deletion of street(s) in permanent address

2.35.0 (2018-03-01)
-------------------

* adapt to mail_archive changes (columns message_params, mail_type_id)

* fix - record statement for object in deleteCandidate state

2.34.0 (2018-02-01)
-------------------

* transitioned to a newer C++ standard (C++14)

* epp - registrars' password is stored as hash (pbkdf2 sha512)

* reimplemented object deletion (object types by name, spread during time argument)

* reimplemented generation of poll messages

* repository structure reworked

2.33.1 (2017-12-01)
-------------------

* epp - fix response code for invalid country code in mailing address (without reason message)

2.33.0 (2017-09-12)
-------------------

* epp rewrite - domain, nsset, keyset, contact - code cleanup

* epp rewrite - poll methods and credit info method

* epp contact support mailing address

* add possibility to dump configuration parameters to debug log (for testing
  configuration changes)

* add registry record statement interface impl.

2.32.0 (2017-09-06)
-------------------

* add regex configuration (database) for contact / nsset / keyset handles

2.31.0 (2017-06-09)
-------------------

* automatic keyset management interface impl.

2.30.0 (2017-03-13)
-------------------

* dedicated interface for public requests (authinfo, object block/unblock requests)

* contact duplicates merge procedure optimization and improvements

  * add flag for warning letter sending to comparison

  * add all contact addresses to comparison

* fix domain outzone warning e-mail (new template parameter)

* fix trailing dot in whois (webwhois/rdap backend)

* fix ShellCmd (occasional select timeout because of SIGCHLD was processed by other thread)

* fix Optys undelivered messages reports processing (CZ.NIC only)

2.29.2 (2017-03-30)
-------------------

* replace usage of user-defined aggregate function array_accum with built-in array_agg

2.29.1 (2017-03-08)
-------------------

* fix - epp domain renew operation bill item (date_from)

2.29.0 (2016-12-19)
-------------------

* epp backend rewrite - domain methods

* epp backend keysets - add configuration of prohibited dnskey algorithms

* epp backend contacts - fix authorization error reason message (update operation)

* epp backend nssets - add configuration for min/max ns hosts

* centos7 with old stdint and mpdecimal-2.4.2 build fixes

* support for boost 1.48

2.28.0 (2016-10-09)
-------------------

* epp backend rewrite - nsset and keyset methods

2.27.1 (2016-10-13)
-------------------

* whois

  * fix - contact disclose flags (was uninitialized)

  * fix - keyset states (was uninitialized)

  * fix - nsset nameserver ip addresses (accumulation bug)

  * fix - logging severity

2.27.0 (2016-09-07)
-------------------

* whois - internal searation of implementation and corba wrapper

  * fix - enum domain search bug

  * fix - domain delete pending bug

* mojeid

  * method for direct contact validation

  * fix - logging severity

* epp - fix - notification configuration

* new interface method for custom e-mail notification about domain going outzone (after expiration)

2.26.1 (2016-09-07)
-------------------

* gcc 6.1.1 + boost 1.60.0 fixes (tested on fedora 24 and gentoo)

2.26.0 (2016-07-10)
-------------------

* epp backend rewrite - contact methods

* fredlib

  * removed change of sponsoring registrar from update operations

  * (separate operation for transfer)

* fix Nullable get_value_or_default() method

2.25.1 (2016-07-07)
-------------------

* fix - admin. domain unblocking - unwanted delete (status update order)

2.25.0 (2016-06-20)
-------------------

* contact create notification - full data to e-mail

* fix - contact disclose[name|organization|address]

* fix - mojeid backend log severity

* fix - mojeid transfer error handling

* fredlib

  * refactored history data save in new operations

  * transfer operations

2.24.1 (2016-06-20)
-------------------

* fix - epp - allow idn in nameserver (nsset create and update)

* fix - mojeid backend log severity

2.24.0 (2016-04-10)
-------------------

* mojeid backend rewrite

* object event notification made async (epp, mojeid) - fred-admin command

* fredlib

  * operation context creator / two phase commit support

2.23.1 (2016-06-20)
-------------------

* fix - epp - allow idn in nameserver (nsset create and update)

2.23.0 (2016-01-20)
-------------------

* whois backend refactoring (new methods for webwhois client)

* fredlib

  * contact address structure fixes

  * missing includes

* database setup fixture for tests fix (postgresql version - pid vs procpid
  in pg_stat_activity)

2.22.0 (2015-05-19)
-------------------

* build warnings and distribution fixes

* fredlib fixes

  * InfoContactData constructor initialization

  * UpdateContactByHandle discard reference from 'handle' member

  * InfoRegistrarData.vat_payer not Nullable

  * tests added

* domainbrowser

  * interface reworked (simplified)

  * add mailing address to signed-on user contact info

  * add flag to change user preference whether send domain warning letter or not

* mojeid

  * new method for (re)send mojeid card

  * add configuration for letter sending limits

* mojeid/verification - phone format checker fix (discard leading/trailing spaces)

* expiration warning letters

  * log severity fixes

  * contact address validity check improved

  * fix - not to send/generate letters for outdated domain states (expirationWarning)

* epp - update contact error handling fix

* epp/verification - changes conditions for canceling contact verification states
  (name, organization, e-mail, telephone, address)

* messaging - allow to send letter to invalid address (used in admin contact
  verification)

* adifd - add destination account number to payment detail

2.21.1 (2015-03-30)
-------------------

* logger - fix - insert to request_data and request_property_values tables
  rewritten to prepared statements

2.21.0 (2015-01-27)
-------------------

* contact merge procedure

  * removed unused option

  * fixed duplicated contact search query

* public request and object state request locking simplified

* fredlib - object state impl. cleanup

* mojeid

  * new pin3 resending

  * fix e-mail format check

  * add 2 aditional shipping address types

* rdap backend - 'delete pending' status handling

* fixed set/unset discloseaddress flag (mojeid, epp)

* log severity fixes

2.20.5 (2015-02-16)
-------------------

* admin. contact verification - fix check detail for deleted contact

2.20.4 (2015-02-09)
-------------------

* mojeid - cancel account method now also delete contact

2.20.3 (2015-01-27)
-------------------

* fix saving letter country name (not country code)
  for admin. verification letters

2.20.2 (2014-12-31)
-------------------

* company address change

2.20.1 (2014-12-12)
-------------------

* mojeid

  * fix methods for verification states synchronization

  * fix priority of contact ssn type/value save due to
    validation requirements

  * removed unused ssn_type from corba interface

2.20.0 (2014-10-17)
-------------------

* mojeid - contact - additional addresses (mailing, shipping, billing)

* fredlib - fix domain info (missing zone)

* tests - restructured, shared utils, testcase isolation

* filter out database password from log

* fix

  * admin. verification (cz postal address test)

  * object state cancellation

* fix

  * log messages severity (mojeid, contact verification)

2.19.2 (2014-10-24)
-------------------

* admin. contact verification

  * new automatic test (email domain in managed zones)

  * fix email host test (more email addresses comma separated)

2.19.1 (2014-09-01)
-------------------

* rdap - fix - return timestamps in UTC

* domainbrowser - fix - canceling multiple object state requests

* mojeid/verification - fix sms text

* adifd - fix resending public request messages (PIN3)

2014-08-01 Jan Korous, Jan Zima, Michal Strnad, Jiri Sadek (2.19.0)
-------------------

* domain browser

  * new backend

  * manual contact duplicate merge feature

  * object blocking/unblocking fixes

* fix object blocking/unblocking compatibility between domainbrowser and public requests

* contact duplicate merge (procedure)

  * rules fixes (user/admin blocking, mojeid)

  * speed-up

* message forwarding service mapping and configuration

  * new sender for OPTYS service (CZ.NIC only)

* whois backend rewrite prototype (now used only for rdap)

* epp

  * admin. contact verification (add check when updating contact is now configurable)

  * fix saving request_id for contact check

2.18.0 (2014-06-12)
-------------------

* admin. contact verification implementation

* fredlib - fixes, operation interface changes, impl. refactoring, new operations added

* admin. domain block fix - creating poll update messages

* corba utils - common type (un)wrappers

* nullable/optional types enhancements

* doxygen code documentation started!

2.17.1 (2014-03-26)
-------------------

* fix mojeid identification validator (country, postal code checks removed)

2.17.0 (2014-02-19)
-------------------

* fix input xml escaping for pdf document generator

* document generator external command is now called by 'ShellCmd' instead of
  'system' call to get better error logging

* adifd - methods for resend messages (only contact verification PIN2/PIN3)
  associated with public request

* mojeid

  * allow contact update before PIN3 (in conditionally identification status)

  * birthday format check (contact.ssn) in mojeid transfer is now
    more clever and support several notations; fix saving to db (iso format)

* contact verification - birthday is excluded from checks

* mojeid/contact verification - 30 days registration "protection" period for
  'e-mail' and 'telephone' values is now disabled for identical contact (by id)

2.16.3 (2014-05-12)
-------------------

* fix idn fqdn check

2.16.2 (2014-02-17)
-------------------

* fix rpm dependencies

2.16.1 (2014-02-10)
-------------------

* bugfix in mojeid interface (wrong exception translation at corba wrapper)

2.16.0 (2013-11-11)
-------------------

* new interface for administrative blocking/unblocking domains (and holders)

* epp operation charging has now configuration option

* serveral idn support fixes/enhancements (still mostly for experimental purpose due to lack of
  definition of allowed character sets)

  * configuration option

  * enabled for system registrar

* fix logger object references filter

* fix epp poll req/ack commands - overflow of count values

2.15.3 (2013-11-15)
-------------------

* contact merge - generate new authinfo for destination contact

2.15.2 (2013-10-25)
-------------------

* fix zone name resolution

* fix nsset host fqdn length

* fix epp keyset command input value escaping in sql

2.15.1 (2013-09-25)
-------------------

* whois (contact) reminder - fix sql for postgresql >= 8.4

2.15.0 (2013-08-07)
-------------------

* mojeid - managing of disclose flags removed from interface

2.14.1 (2013-06-05)
-------------------

* mojeid - implementation of getUnregistrableHandlesIter() idl method
  transfer contact handles to client by small chunks

2.14.0 (2013-04-02)
-------------------

* automatic procedure for duplicate contacts merging

2.13.5 (2013-04-17)
-------------------

* removed fix pagetable filter sort in database which caused problem with
  selection object history

2.13.4 (2013-04-02)
-------------------

* fix pagetable limit settings (didn't work at all)

* fix pagetable filter sort in database

2.13.3 (2013-01-11)
-------------------

* notification letters send (postservis upload) is done in two batches (domestic and foreign)

* fix save of recipient postal address and contact reference into letter archive

2.13.2 (2012-12-18)
-------------------

* epp - fix update contact (cancel contact verification status check)

2.13.1 (2012-12-06)
-------------------

* reverted logging for document generator (Bad file descriptor error when
  using ShellCmd)

2.13.0 (2012-11-20)
-------------------

* fix/improvement in cancel state function

* fix compilation issues with boost >=1.50

* more detailed error logging for document generator external command

* epp

  * allow to set discloseaddress flag in update contact command

  * fix check for glue ip in create/update nsset commands

  * fix display of contact states (schemas corrected)

* mojeid

  * contact status condition changed for setting discloseaddress flag
    (validatedContact -> identifiedContact)

  * contactUnidentifyPrepare(..) method removed

  * fix sms message

  * fix logging messages severity

* logger - fix logging boost format string

2.12.4 (2012-10-17)
-------------------

* mojeid/contact verification - fix pin3 message type (registered_letter -> letter)

2.12.3 (2012-10-10)
-------------------

* mojeid

  * fix logging messages severity

  * updated phone check regex

* whois

  * add log context

  * add method call identificator

* disabled logging of sql result

2.12.2 (2012-10-04)
-------------------

* adifd - fix processing of public request (locking)

2.12.1 (2012-10-03)
-------------------

* mojeid

  * cancel account fix (missing lock)

  * logging of wrong password (pin1/2) exception

2.12.0 (2012-09-06)
-------------------

* contact verification implementation

  * mojeid, public request appropriate changes

  * epp - identification states handling in contact update

* mojeid

  * implementation separated from corba layer

  * mostly separated from registry code

  * add method for canceling mojeid account (preserve identification state)

  * fix conditional contact update

  * removed identification method from create/transfer contact interface

  * contact checks speedup

* public request / manual object state locking feature to serialize requests

* logger

  * removed output flag from properties interface

  * fix usage of connection releaser

* epp

  * fix low credit poll message

  * fix domain renew for maximum period

  * fix technical_test command to load default domain set

* banking - fix payment processing for registrar with no access to zone to pay debt

* request fee

  * fix request count within one day + tests

  * fix registrar zone access check in request charging

* adifd

  * method for getting summary of expiring domains (performance issues fix)

  * fix method for object detail

2.11.2 (2012-06-11)
-------------------

* mojeid - fixed validated contact update checks (birthday change bug)

2.11.1 (2012-06-07)
-------------------

* mojeid/epp - fixed request notifications

* mojeid - fixed checks for discloseaddress flag change

2.11.0 (2012-05-14)
-------------------

* mojeid

  * allow to change discloseaddress flag

  * constant pins in demo mode

  * as-you-type check backend method returning unregistrable handles

  * add check to don't allow data update for conditionally identified contact

  * add contact authinfo getter

* request fee

  * count requests for commands with object handle list as parameter

  * poll commands are excluded from request fee

* disable update request notification if there are no changes

* refuse to change object in deleteCandidate status

* invoicing - fred-admin interface for adding new prefixes

* epp code fixes - throw spec removed

2.10.0 (2012-04-27)
-------------------

* epp action removed from fred

2.9.11 (2012-03-22)
-------------------

* epp - do not notify command with specific cltrid (system registrar only)

2.9.10 (2012-03-19)
-------------------

* fix zone selection for domain registration

2.9.9 (2012-03-13)
------------------

* memory leaks fixes - objects changes notifier, documents

* whois reminder fix - interval change for contact selection (duplicate
  email sending)

* epp - removed temp-c domain notification

* banking

  * payment import/processing fix (negative credit balance)

  * daphne bank payment list fix (duplicate payment)

* invoicing - invoice total price in page table fix

* build fix - Makefile.am

2.9.8 (2011-12-23)
------------------

* adifd - history record switched from action_id to logger request_id

2.9.7 (2011-11-14)
------------------

* request fee charging fixes (registrar in zone sql, time period, defaults)

* invoice xml export fix (check for valid date period - -inf,+inf problem)

2.9.6 (2011-11-07)
------------------

* custom date for creating request fee poll messages

* custom date (poll message) for request fee charging

* request fee charging fixess (invoice operation crdate, transaction added)

* several log message corrected

2.9.5 (2011-10-31)
------------------

* whois reminder fix for linked status (must be valid)

* bank payment import fix (statement/payment list check)

2.9.4 (2011-10-31)
------------------

* chargeRequestFee fix (all registrars)

2.9.3 (2011-10-28)
------------------

* fred-admin help description fixes

* chargeDomainCreate/Renew(...) now checks object_id for 0 value

* chargeRequestFee(...) return value corrected

2.9.2 (2011-10-24)
------------------

* billing fix - annual partitioning (unrepeated operations)

* fix registrar blocking (limit 0)

2.9.1 (2011-10-21)
------------------

* billing fixes

  * interval for operations selection

  * annual partitioning

  * default taxdate

* rpm dependencies

2.9.0 (2011-10-18)
------------------

* invoicing

  * database schema rework

  * post paid operations (allowed negative credit)

  * registrar credit separation from invoices

  * distribution of charged operation (price) to deposit invoices moved
    to billing

  * invoice_factoring command renamed to invoice_billing (fred-admin)

  * request fee charging impl - charge_request_fee command (fred-admin)

* banking - check of registrar zone access in payment import

* registrar request fee limits and blocking feature

* whois reminder - now reminds only contact with linked status

* Decimal wrapper for mpdecimal library (by Stefan Krah
  <skrah@bytereef.org>) - use for money operations

* mojeid

  * invalid date format unhadled exception fix

  * authinfo save fix

2.8.10 (2011-10-17)
-------------------

* fixed whois handling of deleteCandidate state (new registered domain in
  actual day)

2.8.9 (2011-10-11)
------------------

* fixed fred-admin object_delete_canadidates command (debug output)

2.8.8 (2011-09-27)
------------------

* domain deletion is now batched in daily procedure

* during delete day, domains to be deleted or already deleted
  are shown in whois with special state deleteCandidate (which is now external)

2.8.7 (2011-09-27)
------------------

* fixed creating request fee poll messages - logger call date/time
  conversions

2.8.6 (2011-09-26)
------------------

* fred-adifd - interface for getting last request fee info data

* epp - interface for deleting all sessions for given registrar

* fixed creating request fee poll messagess on first day of month

* object state changes notifications now uses correctly enum
  parameters config

2.8.5 (2011-09-02)
------------------

* epp - fix sql input data escape in client login

2.8.4 (2011-08-11)
------------------

* logger

  * request count interface has now proper data types for dates

  * added method counting request for all usernames

* mojeid - fixed fax format checker

* whois reminder - sql optimization

2.8.3 (2011-07-08)
------------------

* poll request fee

  * sql timestamp conversion fix

  * performance issues fix (proper partition by service)

  * message create duplicity check added

2.8.2 (2011-07-06)
------------------

* poll request fee

  * internal interfaces data type changes

  * code cleanup

* fixed function for object state set

* invoicing tests fix

2.8.1 (2011-07-04)
------------------

* fixed return type for corba method

2.8.0 (2011-07-04)
------------------

* poll message for requests charging impl.

2.7.6 (2011-06-29)
------------------

* fred-admin - invoice_archive fix (no registrar organization name in pdf)

2.7.5 (2011-06-20)
------------------

* createAccountInvoice(s) functions fix (date conversion)

  * interface string dates was replaced with boost::gregorian::date type

* fred-admin

  * sending registered letters fix

  * broken commands commented out

* invoicing tests fixes

2011-06-17 (2.7.4)
------------------

* epp create domain operation fix (division by zero - period)

* insufficient credit log message severity adjusted

* fred-admin

  * memory invalid read fix

  * corba client fix (BAD_INV_ORDER exception)

* invoicing tests enhancements and fixes

2.7.3 (2011-06-14)
------------------

* invoice archive fix (invoice list reload fix)

* whois contact reminder fix (call with specified date in past
  should not select objects created in future from that date)

* invoicing tests fixes

2.7.2 (2011-06-07)
------------------

* new invoicing tests

* invoicing fix

  * price values overflow problem

  * money/price conversions

  * fred-admin --invoice_credit

* fredlib/getCreditByZone returns string now

* fredlib/getBankAccounts don't thow on empty list

2.7.1 (2011-05-26)
------------------

* mojeid - interface changes for methods for 2PC

* invoicing fix

  * vat computation + test

  * error handling, logging

  * money conversions

2.7.0 (2011-05-20)
------------------

* fred-admin

  * configuration redesigned and rewritten

  * command for manual creating/sending of registered letters

* whois contact reminder implemented

* invoicing refactoring - removed from old_utils

* mojeid

  * data validation fixes (fax, notify_email)

  * notification error handling fixes

  * create notification added

* logger - exception handling in corba wrapper

2.6.5 (2011-05-16)
------------------

* fix domain create charging

2.6.4 (2011-04-20)
------------------

* spec file changes (omniorb package name)

* logger filter optimization for one record (id filter)

2.6.3 (2011-04-07)
------------------

* fix logger filters - performance

2.6.2 (2011-03-28)
------------------

* fix date filter

* fix bank payment sort by memo

* fix logger - username and userid was not logged for requests

2.6.1 (2011-03-17)
------------------

* request cache fix - exception specification removed, cache double search
  fixed

2.6.0 (2011-02-24)
------------------

* servers build enhancements

* servers sources splitted, initialization rewritten

* new tests added

* fred-mifd

  * request notification (same as in epp)

  * authinfo attribut added

  * message content fixes

* fred-log

  * queries performace fixes

  * session cache

* fred-adifd

  * pagetable query limit

  * new filters (banking, messages)

* messages - message templates fixes (countrycode)

* fred-pifd (whois) and fred-adifd (admin) implementation changes due to idl
  interface split

* epp - update enum domain - enumdir attribute is not mandatory (fixed) -
  schema deps

2.5.13 (2010-12-20)
-------------------

* fix invalid throw usage

* fix bad query in commitPreparedTransaction(...)

2.5.12 (2010-12-14)
-------------------

* fred-mifd - SK support, epp action clienttrid removed from
  queries, public request locking (select for update fix)

2.5.11 (2010-12-03)
-------------------

* fred-mifd contact unidentification implemented

2.5.10 (2010-11-24)
-------------------

* fred-pifd whois contact display fix complete (disclose flags), previous
  bug fix removed

* fred-mifd processing identification - more exceptions for error
  state distinction

2.5.9 (2010-11-11)
------------------

* fred-pifd quick bug fix release (mojeid - whois disclose flags)

2.5.8 (2010-11-08)
------------------

* fred-mifd create/transfer checks rewritten, bugfixes

2.5.6 (2010-10-25)
------------------

* fred-mifd another sql query performance fix

2.5.5 (2010-10-25)
------------------

* fred-mifd sql query performance fix

2.5.4 (2010-10-25)
------------------

* fred-mifd some database insert/update checks added

2.5.3 (2010-10-25)
------------------

* fred-mifd fixes (contact data validation - phone, address check,
  required check trimmed, contact transfer poll message)

2.5.2 (2010-10-24)
------------------

* fred-mifd fixes

2.5.0 (2010-10-18)
------------------

* new logger interface

* new fred-mifd server for MojeID backend functions

2.4.3 (2010-08-27)
------------------

* sending of expiration letters fix

* config defaults changed

* spec file update

2.4.2 (2010-07-23)
------------------

* generation of expiration letters fix

* logger request detail minor fix (id)

2.4.1 (2010-07-22)
------------------

* Coverity errors fixes

* postservice

  * order state check

    * limit for domains per letter

    * batch processing

    * configuration and logging fixes

* old banking client removed

2.4.0 (2010-06-17)
------------------

* registrar groups and certification implemented

* expiration letters notification refactoring

  * now sending with postservice - optional

  * multiple domains in one letter (address grouping)

  * new letter format

* Epp update_domain command - changes behaviour

* if changing nsset and not keyset,
  keyset will be removed (to not break dnssec)

* Epp sessions - add locks (thread safe issue with session counter)

* Inactivation of domain from dns is now notified by mails with
  generic addresses automatically (like kontakt@domena.cz, info@domena.cz..)

* fixes in logd component

* tests for model, groups, certifications

2.3.11 (2010-06-16)
-------------------

* fix enumdir to use publish flag correctly (forgotten from 2.2)

2.3.10 (2010-04-28)
-------------------

* fix/optimization invoice list (VAT)

* fix datetime column data convert in bank payment list

* fix union filter/sorting in epp actions and filters

* add psql notice handler for log

2.3.9 (2010-04-08)
------------------

* fix content of notification emails

* fix performance issues in logd

* fix several small bugs in invoicing

2.3.8 (2010-03-31)
------------------

* fix getCreditByZone(...) in invoice manager (bad cast - bad money format)

* fix bank payment default sort in pagetable

2.3.7 (2010-03-28)
------------------

* bank payment processing only payments from registrars fix

* filter serialization fixes

* unexpected exception from mailer fix

* Logger - queries reduction

2.3.6 (2010-03-22)
------------------

* bank payment type column default value set

* bank payment import/processing fixes

2.3.5 (2010-03-18)
------------------

* fred-admin importing bank xml fixes

* Logger pagetable hadling fixes

* create_domain billing fixes

* Generated sql with order by clause fixes

2.3.3, 2.3.4 (2010-03-13)
-------------------------

* Compiling fixes:

  * Removed build of test-model due to boost version (non)compatibility

  * Fixed logger (scoped_lock)

2.3.2 (2010-03-12)
------------------

* Logger filtering moved from adifd to logger itself

* adifd calls logd using CORBA to access audit log

* New tests added

* Bugfixes

2.3.1 (2010-02-16)
------------------

* Bugfix release (fred-admin mainly)

2.3.0 (2010-02-16)
------------------

* New audit component - daemon for logging all external
  inputs to fred system

* Banking subsystem rewritten - uses xml structure
  for statement/payment import generated by external transproc
  utility

* Registrar and Zone access management interface for administration
  interface implemented

* Memory leaks fixes

2.2.0 (2009-11-09)
------------------

* Functionality for enum dictionary project

* action - public request relationship removal
  (idl interface change/fix)

* DS records functionality completely removed

2.1.14 (2009-08-12)
-------------------

* Minor code fixes (coverity)

* Fixed not-thread-safe zone loading/handling in epp interface

* CSOB payments script added to package

* fred-admin options update; minor bug fixes

* Adding of DS record to Keyset is prohibited, removal is still possible

2.1.13 (2009-07-01)
-------------------

* list of dnskey algorithms expanded to satisfy RFC 5155.

* ability to manually add domain into the zone

* extended number of options for `fred-admin`

* fixed unsafe syslog logging

* fixed init script /bin/sh compatibility

* harcoded value for handle protection period is now parameter
  in database 'enum_parameters' table

2.1.12 (2009-06-22)
-------------------

* Bugfix in registrar data load (cross table)

2.1.11 (2009-05-25)
-------------------

* Init script updated with zone and registrar initialization

* No restart needed when adding new zone

2.1.10 (2009-05-14)
-------------------

* Minor bugfixes detected by coverity and valgrind

* Bugfix in update notification

* Bugfix in logging message

* Bugfix in fred-admin (command-line parameters issues)

2.1.9 (2009-05-05)
------------------

* Bugfixes in notifications

* Bugfixes in mailer manager subsystem

* Bugfix in update_domain and delete_domain (zone check handling)

* fred-admin regular object procedure changed in order to delete
  domains first

* Update notification improved - now includes changes made by update command

* Refactoring of db library

  * restructualized

  * type conversions - better separation of stringize for human readable
    output and sqlize for serialization to queries

  * old connection handling in new library for old code compatibility

2.1.8 (2009-03-25)
------------------

* Bugfixes

  * SQL for registrar list fixed

  * Parameter --factoring in fred-banking now works

  * Sorting generally and sorting of invoices in webadmin works
    better now

  * Fixed problem in storing long xml answers

  * Fixed zone check in case of uppercased fqdn of host in nsset

* Better handling of object changes in table 'history'

* Changes in notifications

  * DeleteContact EPP command was not notified

  * Regular delete commands are not notified

  * Better check of invalid emails in notification process

* Daily regular procedure now delete objects before notification to speed
  up delete

2.1.7 (2009-02-10)
------------------

* Missing source file str_corbaout.h in distribution package

2.1.6 (2009-02-06)
------------------

* Bugfix in modification of OR-connected filters

* Bugfix in search by notifyemail in history filter - wrong column specified

2.1.5 (2009-01-06)
------------------

* Bugfix in creation of public request

2.1.4 (2008-12-17)
------------------

* Bugfixes in admin backend

  * bad sql generation in filtr by any contact map

  * error in opening domain filter containing keyset or nsset filter

* Fixing few memory leaks

* Bugfix in banking to support negative invoices

2.1.3 (2008-12-03)
------------------

* Bugfix in keyset notification and public request processing

2.1.2 (2008-11-11)
------------------

* Bugfix domain deletion

2.0.8, 2.1.1 (2008-11-07)
-------------------------

* Sorting of domains in PDF warning letter (by country, org, name)

* Bugfix in email notification (domain notification failed) (in 2.0.7)

* Bugfix fred-admin registrar api (--zone_add --registrar_add --registrar_add_zone)

* Update config file with [banking] section (must be at the end!)
  to allow fred-banking work with default /etc/fred/server.conf

2.1.0 (2008-10-20)
------------------

* Configure script minor changes to support omniORB 4.0

* EPP

  * number of dsrecords, dnskeys and techcontacts fixed

  * reason and response messages revised

2.0.6 (2008-10-15)
------------------

* Fixing table sorting by datetime fields (DateTime operator)

* Fixing loading of filter containing datetime interval

  * missing filter type specification

  * timezone conversion (rewritten from boost to SQL)

* Fixing reference to PDF and XML file in invoice detail

* Fixing XML output of fred-admin --invoice_list command

2.0.5 (2008-09-30)
------------------

* Number of admin corba sessions limited

* Database connection management improved

* Fixing fred-admin --invoice_list option

* Fixing interval filter SQL serialization

2.0.2, 2.0.3, 2.0.4 (2008-09-30)
--------------------------------

* Reverted change of --conf parameter to --config

* Duplicity of DS records in different KeySets allowed

* Admin interface interval filter fixed

* DomainUpdate action closing fixed

2.0.1 (2008-09-18)
------------------

* Refactoring fred-admin

(2008-09-18)
------------

* Logging system rewritten to support context (known NDC, MDC) messaging
  because of multithreading

* Added server configuration for CORBA Nameservice context

* Object state information in Daphne

(2008-09-10)
------------

* Both logging methods merged to new logger by LOG macro redefinion

* New configuration format and storage class processing (used
  boost's program_options approach)

* New configuration options added (see config/server.conf.in for details)

2.0.0 (2008-08-15)
------------------

* Merged history and dnssec branches

* ADIFD

  * history for domain, contact, nsset, keyset added to administation
    interface

    * history details method

    * filter serialization accept setting structure (for history on/off handling)

  * deleted hardcoded user list (until user management authentication will
    be done in frontend)

    * changed TableRow to IDL module Register and all fields
      rewritten to CORBA Any type

  * links in PageTable are done by OID structure (id, handle, type)

* RIFD

  * implementation of DNSSEC extension features

  * new object KEYSET

  * implemented EPP commands

    * create_keyset, update_keyset, info_keyset, check_keyset,  delete_keyset, list_keysets

(2008-07-25)
------------

* Database library (util/db) slightly rewitten

  * required appropriate changes in other modules
    (need testing if something is not broken!)

(2008-07-15)
------------

* Bugfix - Missing ORDER BY in notification component

1.11.0 (2008-07-13)
-------------------

* Bugfix - Fixed exception handling in public requests

* Changes to implement IDL numRowsOverLimit()
  method in all pagetables. It detect if number of rows in result set
  was limited by defined constant (load_limit_ in CommonList).

* EppAction

  * filter for Requested Handle -> object doesn't need to be in registry

  * output xml added to detail

  * EppActionType changed from string only to id - name pair for proper
    filtering

  * EppActionTypeList is now loaded from database (not hardcoded)

* Bugfix - Registrar reload() badly handled findIDSequence()

* Bugfix - Cancel/OutZone Domain filter handles badly special
  type LAST_DAY +- offset

1.10.0 (2008-06-26)
-------------------

* querying invoices, mails, files

* adding filters to domains

* fixing state change poll messages

1.9.3 (2008-06-12)
------------------

* bugfix - database connection leaking

1.9.2 (2008-06-11)
------------------

* bugfix - emails with request for authinfo was sent to bad address

1.9.1 (2008-06-05)
------------------

* public request system fixes

  * epp delele operation blocked on serverUpdateProhibited

  * fixed type in mail with answer

  * speedup of status update

(2008-06-04)
------------

* logging into syslog by default

* date interval filtering fixed

* compliation warning cleaned

1.9.0 (2008-05-30)
------------------

* new public request module

* complete new architecture of server

* backend admin interface features

* enhanced build system

1.8.3 (2008-05-16)
------------------

* almost full IDN support

  * encodeIDN, decodeIDN function in zone manager

  * parametrized check of domain fqdn (allow xn-- at the start of domain)

  * whois and admin backend translate utf8 string into and from ascii form


* disabling default generation of poll messages about delete cont/nsset

* optimizing query for objects to delete

* fixing type of object in notificiation email about delete of nsset

1.8.2 (2008-03-25)
------------------


* object delete procedure and notification made parametrized

* admin command --zone_add fill records in zone_soa and zone_ns tables


* changed mailer_manager and tech_check resolving of corba object (managed
  by pyfred) when needed not in initialization - removes dependency on
  starting order of fred-rifd and pyfred servers

* fixing date in notification of technical contact about removal from zone

* update contact notification is now sent to old notify adress as well

* really small bugfixes

  * fixing creation of path to xslt templates

  * fixing return value during action element insertion

  * fixing missing disconnection of database session in adif backend

* invoicing bugfixes

  * uninitialized determination value

  * language of PDF based on country instead of vat application

1.8.1 (2008-02-25)
------------------

* changing admin session handling

  * every session has separate session object

  * simple garbage collector for session with 30 minut of inactivity

1.8.0 (2008-02-09)
------------------


* adding parameter to fred-admin

  * registrar creation

  * zone creation

  * zone registrar access creation

* repairing queries into action table

  * removing LEFT JOINS

  * parsing EPP commands into action_elements table

  * fixing date time querying of actions

  * better wildcard handling

* dns hostname check agains enum_tld tables

* credit discovering supported in admin interface

* letter generation supported in admin interface

* initial sorting support (domains by exdate)

* more columns and faster load in admin invoice list

* locking of EPP command (*new configuration option*)

* rpm building support

* complete refactoring of build scripts .am and .ac

* invoicing improvements

  * use english PDF template for foreign registrars

  * new option in fred-admin --invoice_dont_send to disable mailing

  * export partial VAT in xml (for new PDF format)

  * speedup (call ANALYZE)

1.7.6 (2007-11-16)
------------------

* object state requests can be added through fred-admin

* emails with notification about expiration have registrar name instead
  of registrar handle

* postgresql NOTICE messages on client side disabled

* regular daily periodical procedure implemented in fred-admin

* removal of unused code expiration (fred-expiration) and whois

* overlapping zones supported

* info request optimalization

* nsset create and update fixes

  * better hostname control (refactored to use registry library)

  * check of hostname duplicity in request

  * check for count of dns was outside Action (returned no SVTRID)

* hack in timezone conversion removed

* notification fixes

  * pdf letters generation

  * sql fixes in email notification

* lot of banking and invoicing fixes

  * GPC parser

  * database sequence management

  * invoicing by zone

  * rounding bug in float->int conversion

  * change type long -> long long for extra long invoice numbers

  * support for export to accounting company

1.7.3
-----

* remove libdaemon

1.6.4
-----

* invoice mailing ignore registrars without email

* repair version numbers of transform poll messages

* bug in initialization in unix whois server

1.6.3
-----

* bug in update of disclose flags fixed

* bug in update of address fixed

* faulty namespace version in poll message corrected

1.6.2
-----

* disclose flags for vat, notifyEmail and ident

* temporary contact handling

* new logging infrastructure (without libdaemon)

* new config options (restricted_handles,disable_epp_notifier)

* reconnect in every login to admin interface

* sql optimalization
