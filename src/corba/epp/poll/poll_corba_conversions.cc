/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/corba/epp/poll/poll_corba_conversions.h"
#include "util/corba_conversion.h"
#include "util/util.h"

#include <boost/variant.hpp>

// to be removed I guess
#include "src/old_utils/util.h"
namespace Corba
{

namespace
{

// copy paste from epp_impl.cc -- to be removed
std::string formatTime(const boost::posix_time::ptime& tm) {
   char buffer[100];
   convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
   return buffer;
}

struct TransferEventWrapper : boost::static_visitor<AnyType>
{
    template<typename Flag>
    AnyType operator()(const Epp::Poll::TransferEvent::Data<Flag>& _src) const
    {
        return wrap_transfer_event_data_into_any(_src);
    }
};

template<typename> struct TransferTypes;

template<>
struct TransferTypes<Epp::Poll::TransferEvent::transfer_contact>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_contact;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::transfer_domain>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_domain;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::transfer_nsset>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_nsset;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::transfer_keyset>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_keyset;
};

} // namespace Corba::{anonymous}

template<typename Flag>
AnyType wrap_transfer_event_data_into_any(const Epp::Poll::TransferEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_HandleDateReg_var hdm = new ccReg::PollMsg_HandleDateReg;
    hdm->date = CORBA::string_dup(to_iso_extended_string(_src.transfer_date).c_str());
    hdm->handle = CORBA::string_dup(_src.handle.c_str());
    hdm->clID = CORBA::string_dup(_src.client_id.c_str());
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = TransferTypes<Flag>::value;
    return ret;
}

AnyType wrap_transfer_event_into_any(const Epp::Poll::TransferEvent& _src)
{
    return boost::apply_visitor(TransferEventWrapper(), _src.message);
}

namespace
{

struct MessageEventWrapper : boost::static_visitor<AnyType>
{
    template<typename Flag>
    AnyType operator()(const Epp::Poll::MessageEvent::Data<Flag>& _src) const
    {
        return wrap_message_event_data_into_any(_src);
    }
};

template<typename> struct MessageTypes;

template<>
struct MessageTypes<Epp::Poll::MessageEvent::delete_contact>
{
    static const ccReg::PollType value = ccReg::polltype_delete_contact;
};
template<>
struct MessageTypes<Epp::Poll::MessageEvent::idle_delete_contact>
{
    static const ccReg::PollType value = ccReg::polltype_delete_contact;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::idle_delete_domain>
{
    static const ccReg::PollType value = ccReg::polltype_delete_domain;
};
template<>
struct MessageTypes<Epp::Poll::MessageEvent::delete_domain>
{
    static const ccReg::PollType value = ccReg::polltype_delete_domain;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::validation>
{
    static const ccReg::PollType value = ccReg::polltype_validation;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::outzone>
{
    static const ccReg::PollType value = ccReg::polltype_outzone;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::idle_delete_keyset>
{
    static const ccReg::PollType value = ccReg::polltype_delete_keyset;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::idle_delete_nsset>
{
    static const ccReg::PollType value = ccReg::polltype_delete_nsset;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::imp_expiration>
{
    static const ccReg::PollType value = ccReg::polltype_impexpiration;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::expiration>
{
    static const ccReg::PollType value = ccReg::polltype_expiration;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::imp_validation>
{
    static const ccReg::PollType value = ccReg::polltype_impvalidation;
};

} // namespace Corba::{anonymous}

template<typename Flag>
AnyType wrap_message_event_data_into_any(const Epp::Poll::MessageEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_HandleDateReg_var hdm = new ccReg::PollMsg_HandleDateReg;
    hdm->date = CORBA::string_dup(to_iso_extended_string(_src.date).c_str());
    hdm->handle = CORBA::string_dup(_src.handle.c_str());
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = MessageTypes<Flag>::value;
    return ret;
}

AnyType wrap_message_event_into_any(const Epp::Poll::MessageEvent& _src)
{
    return boost::apply_visitor(MessageEventWrapper(), _src.message);
}

AnyType wrap_low_credit_event_into_any(const Epp::Poll::LowCreditEvent& _src)
{
    ccReg::PollMsg_LowCredit_var hdm = new ccReg::PollMsg_LowCredit;
    hdm->zone = CORBA::string_dup(_src.zone.c_str());
    hdm->limit = CORBA::string_dup(_src.credit.get_string(".2f").c_str());
    hdm->credit = CORBA::string_dup(_src.limit.get_string(".2f").c_str());
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = ccReg::polltype_lowcredit;
    return ret;
}

AnyType wrap_tech_check_event_into_any(const Epp::Poll::TechCheckEvent& _src)
{
    ccReg::PollMsg_Techcheck_var hdm = new ccReg::PollMsg_Techcheck;
    hdm->handle = CORBA::string_dup(_src.handle.c_str());
    hdm->fqdns.length(_src.fqdns.size());
    for (std::size_t i = 0; i < _src.fqdns.size(); ++i)
    {
        hdm->fqdns[i] = CORBA::string_dup(_src.fqdns[i].c_str());
    }
    for (std::size_t i = 0; i < _src.tests.size(); ++i)
    {
        hdm->tests[i].testname = CORBA::string_dup(_src.tests[i].testname.c_str());
        hdm->tests[i].status = _src.tests[i].status;
        hdm->tests[i].note = CORBA::string_dup(_src.tests[i].note.c_str());
    }
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = ccReg::polltype_techcheck;
    return ret;
}

AnyType wrap_low_request_fee_info_event_into_any(const Epp::Poll::RequestFeeInfoEvent& _src)
{
    ccReg::PollMsg_RequestFeeInfo_var hdm = new ccReg::PollMsg_RequestFeeInfo;
    hdm->periodFrom = CORBA::string_dup(formatTime(_src.from).c_str());
    hdm->periodTo = CORBA::string_dup(formatTime(_src.to - boost::posix_time::seconds(1)).c_str());
    CorbaConversion::wrap_int(_src.free_count, hdm->totalFreeCount);
    CorbaConversion::wrap_int(_src.used_count, hdm->usedCount);
    hdm->price = CORBA::string_dup(_src.price.get_string(".2f").c_str());
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = ccReg::polltype_request_fee_info;
    return ret;
}

namespace
{

struct UpdateInfoEventWrapper : boost::static_visitor<AnyType>
{
    template<typename Flag>
    AnyType operator()(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src) const
    {
        return wrap_update_info_event_data_into_any(_src);
    }
};

template<typename> struct UpdateInfoTypes;

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::update_domain>
{
    static const ccReg::PollType value = ccReg::polltype_update_domain;
};

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::update_keyset>
{
    static const ccReg::PollType value = ccReg::polltype_update_keyset;
};

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::update_nsset>
{
    static const ccReg::PollType value = ccReg::polltype_update_nsset;
};

} // namespace Corba::{anonymous}

template<typename Flag>
AnyType wrap_update_info_event_data_into_any(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_Update_var hdm = new ccReg::PollMsg_Update;
    hdm->opTRID = CORBA::string_dup(Util::make_svtrid(_src.transaction_id).c_str());
    CorbaConversion::wrap_int(_src.poll_id, hdm->pollID);
    AnyType ret;
    ret.any = new CORBA::Any;
    ret.any <<= hdm;
    ret.type = UpdateInfoTypes<Flag>::value;
    return ret;
}

AnyType wrap_update_info_event_into_any(const Epp::Poll::UpdateInfoEvent& _src)
{
    return boost::apply_visitor(UpdateInfoEventWrapper(), _src.message);
}

} // namespace Corba
