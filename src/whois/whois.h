#ifndef _WHOIS_H_
#define _WHOIS_H_

#include <string>
#include <vector>

//#include "src/corba/Whois2.hh"

//namespace Registry {
//namespace Whois {

struct PlaceAddress {
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string stateorprovince;
    std::string postal_code;
    std::string country_code;
};

struct Registrar
{
    unsigned long long id; //?
    std::string handle;
    std::string name;
    std::string organization;
    std::string url;
    std::string phone;
    std::string fax;
    PlaceAddress address;

    Registrar()
    : id(0)
    {}
};

struct RegistrarSeq
{
    std::vector<Registrar> rdl;/**< list of registrar data */
};

struct ContactIdentification {
    std::string identification_type;
    std::string identification_data;
};

struct Contact
{
    std::string handle;
    std::string organization;
    bool disclose_organization;
    std::string name;
    bool disclose_name;
    PlaceAddress address;
    bool disclose_address;
    std::string phone;
    bool disclose_phone;
    std::string fax;
    bool disclose_fax;
    std::string email;
    bool disclose_email;
    std::string notify_email;
    bool disclose_notify_email;
    std::string vat_number;
    bool disclose_vat_number;
    ContactIdentification identification;
    bool disclose_identification;
    std::string creating_registrar_handle;
    std::string sponsoring_registrar_handle;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed; //TODO make sure empty time is considered a valid input
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;

    Contact()
    : disclose_organization(false),
      disclose_name(false),
      disclose_address(false),
      disclose_phone(false),
      disclose_fax(false),
      disclose_email(false),
      disclose_notify_email(false),
      disclose_identification(false),
      disclose_vat_number(false),
      disclose_notify_email(false)
    {}
};

enum IPVersion { IPv4, IPv6 };

struct IPAddress
{
    std::string address;
    IPVersion version;
};

struct NameServer
{
    std::string fqdn;
    std::vector<IPAddress> ip_addresses;
};

struct NSSet{
    std:: string handle;
    std::vector<NameServer> nservers;
    std::vector<std::string> tech_contact_handles;
    std::string registrar_handle;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;
};

struct NSSetSeq
{
    std::vector<NSSet> nsss;
    bool limit_exceeed;

    NSSetSeq()
    : limit_exceeed(false)
    {}
};

struct DNSKey
{
    short flags;
    short protocol;
    short alg;
    std::string public_key;
};

struct KeySet{
    std::string handle;
    std::vector<DNSKey> dns_keys;
    std::vector<std::string> tech_contact_handles;
    std::string registrar_handle;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;
};

struct KeySetSeq
{
    std::vector<KeySet> kss;
    bool limit_exceeded;

    KeySetSeq()
    : limit_exceeded(false)
    {}
};

struct Domain{
    std::string fqdn;
    std::string registrant_handle;
    std::vector<std::string> admin_contact_handles;
    std::string nsset_handle;
    std::string keyset_handle;
    std::string registrar_handle;
    std::vector<std::string> statuses;
    boost::posix_time::ptime registered;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::gregorian::date expire;
    Nullable<boost::gregorian::date> validated_to;
};

struct DomainSeq
{
    std::vector<Domain> ds;
    bool limit_exceeded;

    DomainSeq()
    : limit_exceeded(false)
    {}
};

struct RegistrarGroup{
    std::string name;
    std::vector<std::string> members;
};

//struct RegistrarGroupList{
//    std::vector<RegistrarGroup> rgl;
//};

struct RegistrarCertification
{
    std::string registrar_handle;
    short score;
    unsigned long long evaluation_file_id;
};

struct RegistrarCertificationList
{
    std::vector<RegistrarCertification> rcl;
};

//struct ZoneFqdnList
//{
//    std::vector<std::string> zfl;
//};

struct ObjectStatusDesc
{
    std::string handle;
    std::string name;
};

struct ObjectStatusDescSeq
{
    std::vector<ObjectStatusDesc> osds;
};

struct ObjectNotExists
: virtual std::exception
{
    const char* what() const throw() {return "registry object with specified ID does not exist";}
};

struct InvalidHandle
: virtual std::exception
{
    const char* what() const throw() {return "registry object with specified handle does not exist";}
};

struct InternalServerError
: virtual std::exception
{
    const char* what() const throw() {return "internal server error";}
};

struct InvalidLabel
: virtual std::exception
{
    const char* what() const throw() {return "the label is invalid";}
};

struct UnmanagedZone
: virtual std::exception
{
    const char* what() const throw() {return "this zone is not managed";}
};

struct TooManyLabels
: virtual std::exception
{
    const char* what() const throw() {return "domain has too many labels";}
};

struct MissingLocalization
: virtual std::exception
{
    const char* what() const throw() {return "the localization is missing";}
};

class Server_impl //: public POA_Registry::Whois::WhoisIntf
{
private:
    static const std::string output_timezone;
    ObjectStatusDescSeq get_object_status_descriptions(const std::string& lang,
                                                       const std::string& type);
public:
    virtual ~Server_impl() {};

    Registrar get_registrar_by_handle(const std::string& handle);

    std::vector<Registrar> get_registrars();


    std::vector<RegistrarGroup> get_registrar_groups();

    std::vector<RegistrarCertification> get_registrar_certification_list();

    std::vector<std::string> get_managed_zone_list();

    Contact get_contact_by_handle(const std::string& handle);

    NSSet get_nsset_by_handle(const std::string& handle);

    NSSetSeq get_nssets_by_ns(const std::string& handle, unsigned long limit,
                              bool limit_exceeded);

    NSSetSeq get_nssets_by_tech_c(const std::string& handle, unsigned long limit,
                                  bool limit_exceeded);

    NameServer get_nameserver_by_fqdn(const std::string& handle);

    KeySet get_keyset_by_handle(const std::string& handle);

    KeySetSeq get_keysets_by_tech_c(const std::string& handle, unsigned long limit,
                                    bool limit_exceeded);

    Domain get_domain_by_handle(const std::string& handle);

    DomainSeq get_domains_by_registrant(const std::string& handle, unsigned long limit,
                                        bool limit_exceeded);

    DomainSeq get_domains_by_admin_contact(const std::string& handle,
                                           unsigned long limit,
                                           bool limit_exceeded);

    DomainSeq get_domains_by_nsset(const std::string& handle, unsigned long limit,
                                   bool limit_exceeded);

    DomainSeq get_domains_by_keyset(const std::string& handle, unsigned long limit,
                                    bool limit_exceeded);

    ObjectStatusDescSeq get_domain_status_descriptions(const std::string& lang);
    ObjectStatusDescSeq get_contact_status_descriptions(const std::string& lang);
    ObjectStatusDescSeq get_nsset_status_descriptions(const std::string& lang);
    ObjectStatusDescSeq get_keyset_status_descriptions(const std::string& lang);
};//Server_impl

#endif /* _WHOIS_H_ */
