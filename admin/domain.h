#ifndef DOMAIN_H_
#define DOMAIN_H_

namespace Admin 
{
 namespace Domain 
 {
  class DBsql;
  /// tokenized domain name type
  typedef std::vector<std::string> DomainName;
  /// main entry class
  class Manager
  {
    virtual ~Manager() = 0;
   public:
    virtual std::string makeEnumDomain(const std::string& number)
      throw (NOT_A_NUMBER) = 0;
    virtual DomainName makeDomain(const std::string& fqdn) 
      throw (INVALID_DOMAIN_NAME) = 0; 
    static Manager *create(DBsql *db);
  };
 } // namespace Domain
} // namespace Admin

#endif /*DOMAIN_H_*/
