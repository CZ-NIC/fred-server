#ifndef DOCGEN_H_
#define DOCGEN_H_

#include "types.h"
#include <iostream>

namespace Register
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
      GT_WARNING_LETTER ///< generate PDF with expiration warning letters
    };
    /// generator that has to be filled with stream of data
    class Generator {
     public:
      class ERROR {}; // any error during generation
      /// client is responsible for delete
      virtual ~Generator() {}
      /// return stream that has to be filled with required data
      virtual std::ostream& getInput() = 0;
      /// close input and wait for finishing generatrion
      virtual TID closeInput() throw (ERROR) = 0;
    };
    /// facade for subsystem responsible for document handling
    class Manager
    {
     public:
      /// public destructor for 
      virtual ~Manager() {}
      /// create generator returning output in given stream
      virtual Generator *createOutputGenerator(
        GenerationType type, 
        std::ostream& output, 
        const std::string& lang
      ) const throw (Generator::ERROR) = 0;
      /// create generator that save output as given filename
      virtual Generator *createSavingGenerator(
        GenerationType type, 
        const std::string& filename, unsigned filetype,
        const std::string& lang
      ) const throw (Generator::ERROR) = 0;
      /// generate document and return it in output stream
      virtual void generateDocument(
        GenerationType type, 
        std::istream& input, std::ostream& output,
        const std::string& lang        
      ) const throw (Generator::ERROR) = 0;
      /// generate document and store it in archive with given name
      virtual TID generateDocumentAndSave(
        GenerationType type,
        std::istream& input, 
        const std::string& name, unsigned filetype,
        const std::string& lang
      ) const throw (Generator::ERROR) = 0;
      static Manager *create(
        const std::string& path, const std::string& pathTemplates,
        const std::string& pathFM,
        const std::string& corbaNS
      );
    };
  }
} // Register

#endif /*DOCGEN_H_*/
