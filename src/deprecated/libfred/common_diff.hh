#ifndef COMMON_DIFF_HH_BCD9399CECFE4D62BAF6615978C80452
#define COMMON_DIFF_HH_BCD9399CECFE4D62BAF6615978C80452

#include <map>
#include <string>
#include <memory>

#include "src/util/types/stringify.hh"
#include "src/deprecated/libfred/object.hh"


namespace LibFred {


typedef std::map<std::string, std::pair<std::string, std::string> > ChangesMap;


/* test */
template<class T, class MT, class LT, class FT>
std::unique_ptr<const T> get_object_by_hid(MT *_m,
                           const unsigned long long &_hid)
{
    Settings settings;
    settings.set("filter.history", "on");

    Database::Filters::Union uf(&settings);
    FT *filter = new FT();
    filter->addHistoryId().setValue(Database::ID(_hid));
    uf.addFilter(filter);

    std::unique_ptr<LT> data(_m->createList());
    data->reload(uf);

    if (data->size() != 1) {
        throw std::runtime_error(str(boost::format(
                        "get_object_by_hid: not found (hid=%1%)") % _hid));
    }

    T* object_data_ptr = dynamic_cast<T*>(data->get(0));
    if (!object_data_ptr) {
        throw std::runtime_error("get_object_by_hid: cast error");
    }

    data->release(0);
    return std::unique_ptr<const T> (object_data_ptr);
}



template<class T>
void compare_and_fill(ChangesMap &_changes,
                      const std::string &_field,
                      const T &_prev,
                      const T &_act)
{
    if (_prev != _act) {
        std::string prev = stringify(_prev);
        std::string act  = stringify(_act);

        _changes[_field] = std::make_pair(prev, act);
    }
}



template<class T>
void compare_and_fill_admin_contacts(LibFred::ChangesMap &_changes,
                                     const std::string &_field_prefix,
                                     const T *_prev,
                                     const T *_act)
{
    bool changed = (_prev->getAdminCount() != _act->getAdminCount());
    for (unsigned int i = 0; changed != true && i < _prev->getAdminCount(); ++i) {
        if (_prev->getAdminIdByIdx(i) != _act->getAdminIdByIdx(i)) {
            changed = true;
            break;
        }
    }
    if (changed) {
        std::string prev_admins, act_admins;

        for (unsigned int i = 0; i < _prev->getAdminCount(); ++i) 
            prev_admins += _prev->getAdminHandleByIdx(i) + " ";

        for (unsigned int i = 0; i < _act->getAdminCount(); ++i)
            act_admins  += _act->getAdminHandleByIdx(i) + " ";

        _changes[_field_prefix + ".admin_c"] = std::make_pair(prev_admins, act_admins);
    }
}



void _diff_object(ChangesMap &_changes,
                  const LibFred::Object *_prev,
                  const LibFred::Object *_act);



std::pair<unsigned long long, unsigned long long>
    get_last_history(const unsigned long long &_id, const unsigned int &_type);


}

#endif /*COMMON_DIFF_H__*/

