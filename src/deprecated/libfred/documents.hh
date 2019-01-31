#ifndef DOCUMENTS_HH_81664F77E9D4483CAA2A6969E0515678
#define DOCUMENTS_HH_81664F77E9D4483CAA2A6969E0515678

#include "libfred/types.hh"
#include <iostream>
#include <stdexcept>
#include <memory>

namespace LibFred
{
  namespace Document
  {
    /// supported types of generation
    enum GenerationType 
    {
      GT_INVOICE_PDF, ///< generate PDF of invoice from XML data
      GT_ADVANCE_INVOICE_PDF, ///< generate PDF of advance invoice from XML data
      GT_AUTHINFO_REQUEST_PDF, ///< generate PDF with request for authinfo
      GT_PUBLIC_REQUEST_PDF, ///< generate PDF with any public request
      GT_INVOICE_OUT_XML, ///< generate exporting XML of invoice
      GT_ACCOUNTING_XML, ///< generate accounting XML
      GT_WARNING_LETTER, ///< generate PDF with expiration warning letters
      GT_CONTACT_IDENTIFICATION_LETTER_PIN2, ///< mojeid identificaton letter with pin2 code
      GT_CONTACT_IDENTIFICATION_LETTER_PIN3,  ///< mojeid identificaton letter with pin3 code
      GT_CONTACT_REIDENTIFICATION_LETTER_PIN3,  ///< mojeid reidentificaton letter with pin3 code
      GT_CONTACT_VALIDATION_REQUEST_PIN3,  ///< PDF with validation request
      GT_CONTACT_VERIFICATION_LETTER_PIN3,  ///< contact verification letter with pin3 code
      GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_NOTICE,
      GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_THANK_YOU,
      GT_CONTACT_IDENTIFICATION_LETTER_PIN3_OPTYS,
      GT_MOJEID_CARD, ///< letter with mojeid (emergency) card
      GT_RECORD_STATEMENT_DOMAIN, ///< domain record statement pdf printout
      GT_RECORD_STATEMENT_NSSET, ///< nsset record statement pdf printout
      GT_RECORD_STATEMENT_KEYSET, ///< keyset record statement pdf printout
      GT_RECORD_STATEMENT_CONTACT, ///< contact record statement pdf printout
    };
    /// generator that has to be filled with stream of data
    class Generator {
     public:
      class ERROR : public std::runtime_error
      {
      public:
          ERROR()
          : std::runtime_error("Generator ERROR")
          {}
      }; // any error during generation
      /// client is responsible for delete
      virtual ~Generator() {}
      /// return stream that has to be filled with required data
      virtual std::ostream& getInput() = 0;
      /// close input and wait for finishing generatrion
      virtual TID closeInput() = 0;
    };
    /// facade for subsystem responsible for document handling
    class Manager
    {
     public:
      /// public destructor for 
      virtual ~Manager() {}
      /// create generator returning output in given stream
      virtual std::unique_ptr<LibFred::Document::Generator> createOutputGenerator(
        GenerationType type, 
        std::ostream& output, 
        const std::string& lang
      ) const = 0;
      /// create generator that save output as given filename
      virtual std::unique_ptr<LibFred::Document::Generator> createSavingGenerator(
        GenerationType type, 
        const std::string& filename, unsigned filetype,
        const std::string& lang
      ) const = 0;
      /// generate document and return it in output stream
      virtual void generateDocument(
        GenerationType type, 
        std::istream& input, std::ostream& output,
        const std::string& lang        
      ) const = 0;
      /// generate document and store it in archive with given name
      virtual TID generateDocumentAndSave(
        GenerationType type,
        std::istream& input, 
        const std::string& name, unsigned filetype,
        const std::string& lang
      ) const = 0;
      static std::unique_ptr<LibFred::Document::Manager> create(
        const std::string& path, const std::string& pathTemplates,
        const std::string& pathFM,
        const std::string& corbaNS
      );
    };
  }
} // Fred

#endif /*DOCGEN_H_*/
