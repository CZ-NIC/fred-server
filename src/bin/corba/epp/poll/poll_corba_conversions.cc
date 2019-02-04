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

#include "src/bin/corba/epp/poll/poll_corba_conversions.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/epp/corba_conversions.hh"
#include "util/util.hh"

#include <boost/variant.hpp>


namespace LibFred
{
namespace Corba
{

namespace
{

PollMessage wrap_transfer_event_into_poll_message(const Epp::Poll::TransferEvent& _src);
template<typename Flag>
PollMessage wrap_transfer_event_data_into_poll_message(const Epp::Poll::TransferEvent::Data<Flag>& _src);

PollMessage wrap_message_event_into_poll_message(const Epp::Poll::MessageEvent& _src);
template<typename Flag>
PollMessage wrap_message_event_data_into_poll_message(const Epp::Poll::MessageEvent::Data<Flag>& _src);

PollMessage wrap_low_credit_event_into_poll_message(const Epp::Poll::LowCreditEvent& _src);

PollMessage wrap_low_request_fee_info_event_into_poll_message(const Epp::Poll::RequestFeeInfoEvent& _src);

PollMessage wrap_tech_check_event_into_poll_message(const Epp::Poll::TechCheckEvent& _src);

PollMessage wrap_update_info_event_into_poll_message(const Epp::Poll::UpdateInfoEvent& _src);
template<typename Flag>
PollMessage wrap_update_info_event_data_into_poll_message(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src);


struct TransferEventWrapper : boost::static_visitor<PollMessage>
{
    template<typename Flag>
    PollMessage operator()(const Epp::Poll::TransferEvent::Data<Flag>& _src) const
    {
        return wrap_transfer_event_data_into_poll_message(_src);
    }
};

template<typename> struct TransferTypes;

template<>
struct TransferTypes<Epp::Poll::TransferEvent::TransferContact>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_contact;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::TransferDomain>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_domain;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::TransferNsset>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_nsset;
};

template<>
struct TransferTypes<Epp::Poll::TransferEvent::TransferKeyset>
{
    static const ccReg::PollType value = ccReg::polltype_transfer_keyset;
};

template<typename Flag>
PollMessage wrap_transfer_event_data_into_poll_message(const Epp::Poll::TransferEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_HandleDateReg hdm;
    hdm.date = LibFred::Corba::wrap_string_to_corba_string(to_iso_extended_string(_src.transfer_date));
    hdm.handle = LibFred::Corba::wrap_string_to_corba_string(_src.object_handle);
    hdm.clID = LibFred::Corba::wrap_string_to_corba_string(_src.dst_registrar_handle);
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = TransferTypes<Flag>::value;
    return ret;
}

PollMessage wrap_transfer_event_into_poll_message(const Epp::Poll::TransferEvent& _src)
{
    return boost::apply_visitor(TransferEventWrapper(), _src.message);
}

struct MessageEventWrapper : boost::static_visitor<PollMessage>
{
    template<typename Flag>
    PollMessage operator()(const Epp::Poll::MessageEvent::Data<Flag>& _src) const
    {
        return wrap_message_event_data_into_poll_message(_src);
    }
};

template<typename> struct MessageTypes;

template<>
struct MessageTypes<Epp::Poll::MessageEvent::DeleteContact>
{
    static const ccReg::PollType value = ccReg::polltype_delete_contact;
};
template<>
struct MessageTypes<Epp::Poll::MessageEvent::IdleDeleteContact>
{
    static const ccReg::PollType value = ccReg::polltype_delete_contact;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::IdleDeleteDomain>
{
    static const ccReg::PollType value = ccReg::polltype_delete_domain;
};
template<>
struct MessageTypes<Epp::Poll::MessageEvent::DeleteDomain>
{
    static const ccReg::PollType value = ccReg::polltype_delete_domain;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::Validation>
{
    static const ccReg::PollType value = ccReg::polltype_validation;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::Outzone>
{
    static const ccReg::PollType value = ccReg::polltype_outzone;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::IdleDeleteKeyset>
{
    static const ccReg::PollType value = ccReg::polltype_delete_keyset;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::IdleDeleteNsset>
{
    static const ccReg::PollType value = ccReg::polltype_delete_nsset;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::ImpExpiration>
{
    static const ccReg::PollType value = ccReg::polltype_impexpiration;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::Expiration>
{
    static const ccReg::PollType value = ccReg::polltype_expiration;
};

template<>
struct MessageTypes<Epp::Poll::MessageEvent::ImpValidation>
{
    static const ccReg::PollType value = ccReg::polltype_impvalidation;
};

template<typename Flag>
PollMessage wrap_message_event_data_into_poll_message(const Epp::Poll::MessageEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_HandleDateReg hdm;
    hdm.date = LibFred::Corba::wrap_string_to_corba_string(to_iso_extended_string(_src.date));
    hdm.handle = LibFred::Corba::wrap_string_to_corba_string(_src.handle);
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = MessageTypes<Flag>::value;
    return ret;
}

PollMessage wrap_message_event_into_poll_message(const Epp::Poll::MessageEvent& _src)
{
    return boost::apply_visitor(MessageEventWrapper(), _src.message);
}

PollMessage wrap_low_credit_event_into_poll_message(const Epp::Poll::LowCreditEvent& _src)
{
    ccReg::PollMsg_LowCredit hdm;
    hdm.zone = LibFred::Corba::wrap_string_to_corba_string(_src.zone);
    hdm.credit = LibFred::Corba::wrap_string_to_corba_string(_src.credit.get_string(".2f"));
    hdm.limit = LibFred::Corba::wrap_string_to_corba_string(_src.limit.get_string(".2f"));
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = ccReg::polltype_lowcredit;
    return ret;
}

PollMessage wrap_tech_check_event_into_poll_message(const Epp::Poll::TechCheckEvent& _src)
{
    ccReg::PollMsg_Techcheck hdm;
    hdm.handle = LibFred::Corba::wrap_string_to_corba_string(_src.handle);
    hdm.fqdns.length(_src.fqdns.size());
    for (std::size_t idx = 0; idx < _src.fqdns.size(); ++idx)
    {
        hdm.fqdns[idx] = LibFred::Corba::wrap_string_to_corba_string(_src.fqdns[idx]);
    }
    hdm.tests.length(_src.tests.size());
    for (std::size_t idx = 0; idx < _src.tests.size(); ++idx)
    {
        hdm.tests[idx].testname = LibFred::Corba::wrap_string_to_corba_string(_src.tests[idx].testname);
        LibFred::Corba::wrap_int(_src.tests[idx].is_test_successful(), hdm.tests[idx].status);
        hdm.tests[idx].note = LibFred::Corba::wrap_string_to_corba_string(_src.tests[idx].note);
    }
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = ccReg::polltype_techcheck;
    return ret;
}

PollMessage wrap_low_request_fee_info_event_into_poll_message(const Epp::Poll::RequestFeeInfoEvent& _src)
{
    ccReg::PollMsg_RequestFeeInfo hdm;
    hdm.periodFrom = LibFred::Corba::wrap_string_to_corba_string(
        LibFred::Corba::convert_time_to_local_rfc3339(_src.from));
    hdm.periodTo = LibFred::Corba::wrap_string_to_corba_string(
        LibFred::Corba::convert_time_to_local_rfc3339(_src.to - boost::posix_time::seconds(1)));
    LibFred::Corba::wrap_int(_src.free_count, hdm.totalFreeCount);
    LibFred::Corba::wrap_int(_src.used_count, hdm.usedCount);
    hdm.price = LibFred::Corba::wrap_string_to_corba_string(_src.price.get_string(".2f"));
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = ccReg::polltype_request_fee_info;
    return ret;
}

struct UpdateInfoEventWrapper : boost::static_visitor<PollMessage>
{
    template<typename Flag>
    PollMessage operator()(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src) const
    {
        return wrap_update_info_event_data_into_poll_message(_src);
    }
};

template<typename> struct UpdateInfoTypes;

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::UpdateContact>
{
    static const ccReg::PollType value = ccReg::polltype_update_contact;
};

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::UpdateDomain>
{
    static const ccReg::PollType value = ccReg::polltype_update_domain;
};

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::UpdateKeyset>
{
    static const ccReg::PollType value = ccReg::polltype_update_keyset;
};

template<>
struct UpdateInfoTypes<Epp::Poll::UpdateInfoEvent::UpdateNsset>
{
    static const ccReg::PollType value = ccReg::polltype_update_nsset;
};

template<typename Flag>
PollMessage wrap_update_info_event_data_into_poll_message(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src)
{
    ccReg::PollMsg_Update hdm;
    hdm.opTRID = LibFred::Corba::wrap_string_to_corba_string(Util::make_svtrid(_src.transaction_id));
    LibFred::Corba::wrap_int(_src.poll_id, hdm.pollID);
    PollMessage ret;
    ret.content = new CORBA::Any;
    ret.content <<= hdm;
    ret.type = UpdateInfoTypes<Flag>::value;
    return ret;
}

PollMessage wrap_update_info_event_into_poll_message(const Epp::Poll::UpdateInfoEvent& _src)
{
    return boost::apply_visitor(UpdateInfoEventWrapper(), _src.message);
}

struct ConvertorToPollMessage : boost::static_visitor<Corba::PollMessage>
{
    Corba::PollMessage operator()(const Epp::Poll::TransferEvent& _transfer_event) const
    {
        return Corba::wrap_transfer_event_into_poll_message(_transfer_event);
    }

    Corba::PollMessage operator()(const Epp::Poll::MessageEvent& _message_event) const
    {
        return Corba::wrap_message_event_into_poll_message(_message_event);
    }

    Corba::PollMessage operator()(const Epp::Poll::LowCreditEvent& _low_credit_event) const
    {
        return Corba::wrap_low_credit_event_into_poll_message(_low_credit_event);
    }

    Corba::PollMessage operator()(const Epp::Poll::TechCheckEvent& _tech_check_event) const
    {
        return Corba::wrap_tech_check_event_into_poll_message(_tech_check_event);
    }

    Corba::PollMessage operator()(const Epp::Poll::RequestFeeInfoEvent& _request_fee_info_event) const
    {
        return Corba::wrap_low_request_fee_info_event_into_poll_message(_request_fee_info_event);
    }

    Corba::PollMessage operator()(const Epp::Poll::UpdateInfoEvent& _update_info_event) const
    {
        return Corba::wrap_update_info_event_into_poll_message(_update_info_event);
    }
};

} // namespace LibFred::Corba::{anonymous}

PollMessage wrap_into_poll_message(const ::Epp::Poll::PollRequestOutputData::Message& _src)
{
    return boost::apply_visitor(ConvertorToPollMessage(), _src);
}

} // namespace LibFred::Corba
} // namespace LibFred
