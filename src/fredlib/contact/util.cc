#include "src/fredlib/contact/util.h"

#include "fredlib/opcontext.h"

std::pair<std::string, unsigned long long> Fred::ContactUtil::contact_hid_to_handle_id_pair(unsigned long long hid) {
    Fred::OperationContext ctx;

    Database::Result contact_data = ctx.get_conn().exec_params(
       "SELECT "
       "        o_r.id      AS id_, "
       "        o_r.name    AS handle_ "
       "    FROM contact_history AS c_h "
       "        JOIN object_registry AS o_r USING (id) "
       "    WHERE "
       "        c_h.historyid = $1::integer ",
       Database::query_param_list(hid) );

   if (contact_data.size() != 1) {
       throw ExceptionUnknownContactHistoryId();
   }

   return std::make_pair(
       static_cast<std::string>((*contact_data.begin())["handle_"]),
       static_cast<unsigned long long>((*contact_data.begin())["id_"])
   );
}
