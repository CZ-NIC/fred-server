#include <stdexcept>

#include "src/corba/util/corba_conversions_string.cc"

#include "src/fredlib/notify.h"

#include "src/admin/notification/notification.h"

#include "src/corba/admin/notification_impl.h"

namespace Registry {

    namespace Notification {

        void Notification_i::notify_outzoneunguarded_domain_email_list(const DomainEmailSeq &domain_email_seq) throw (INTERNAL_SERVER_ERROR, INVALID_VALUE) {

            try {

                Fred::OperationContextCreator ctx;

                std::vector<std::pair<unsigned long long, std::string> > domain_email_list;

                // convert Corba DomainEmailSeq to C++ vector of pairs (domain_email_list)
                for(unsigned long long i = 0; i < domain_email_seq.length(); ++i) {
                    domain_email_list.push_back(
                        std::make_pair(
                            static_cast<unsigned long long>(domain_email_seq[i].domain_id),
                            Corba::unwrap_string(domain_email_seq[i].email)
                        )
                    );
                }

								Admin::Notification::notify_outzoneunguarded_domain_email_list(ctx, domain_email_list);

                ctx.commit_transaction();

            } catch(const Admin::Notification::VALUE_ERROR &e) {
                throw INVALID_VALUE(CORBA::string_dup(e.what()));
            } catch(Admin::Notification::INTERNAL_ERROR &e) {
                throw INTERNAL_SERVER_ERROR();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }

        }

    }

}

/* vim: set et sw=4 : */
