/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef PAGETABLE_MAILS_HH_89987C2BE0114E259F627A785471CC93
#define PAGETABLE_MAILS_HH_89987C2BE0114E259F627A785471CC93

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Mails_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase 
{
private:
  std::unique_ptr<LibFred::Mail::List> mail_list_;
  MailerManager mm;

public:
  ccReg_Mails_i(LibFred::Mail::List *_list, NameService *ns);
  ~ccReg_Mails_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::Mail::Mail* findId(ccReg::TID _id);
  
//  DECL_ATTRIBUTE_ID(id);
//  DECL_ATTRIBUTE_TYPE(status,CORBA::Long);
//  DECL_ATTRIBUTE_TYPE(type,CORBA::UShort);
//  DECL_ATTRIBUTE_STR(handle);
//  DECL_ATTRIBUTE_STR(fulltext);
//  DECL_ATTRIBUTE_STR(attachment);
//  DECL_ATTRIBUTE_DATETIME(createTime);
};

#endif /*PAGETABLE_MAILS_H_*/
