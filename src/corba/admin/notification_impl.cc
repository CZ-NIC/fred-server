#include <stdexcept>

#include "src/corba/util/corba_conversions_string.cc"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include "src/fredlib/notify.h"

#include "src/admin/notification/notification.h"

#include "src/corba/admin/notification_impl.h"

namespace Registry {

    namespace Notification {

        DomainEmailSeq *Notification_i::notify_outzoneunguarded_domain_email_list(const DomainEmailSeq &domain_email_seq) {

            try {

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

                std::vector<std::pair<unsigned long long, std::string> > invalid_domain_email_list;

								invalid_domain_email_list = Admin::Notification::notify_outzoneunguarded_domain_email_list(domain_email_list);

                DomainEmailSeq_var invalid_domain_email_seq = new DomainEmailSeq();

								if(!invalid_domain_email_list.empty()) {
                    unsigned long index = 0;
                    invalid_domain_email_seq->length(domain_email_list.size());
                    for(std::vector<std::pair<unsigned long long, std::string> >::const_iterator it = domain_email_list.begin();
                        it != domain_email_list.end();
                        ++it, ++index
                    ) {
                        invalid_domain_email_seq[index].domain_id = CORBA::ULongLong(it->first);
                        invalid_domain_email_seq[index].email = Corba::wrap_string(it->second);
                    }
                }

                return invalid_domain_email_seq._retn();

            } catch(Admin::Notification::INTERNAL_ERROR &e) {
                throw INTERNAL_SERVER_ERROR();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }

        }

    }

}

/* vim: set et sw=4 : */
