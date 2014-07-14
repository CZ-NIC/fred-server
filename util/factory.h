#ifndef FACTORY_H__
#define FACTORY_H__

#include <map>
#include <string>
#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "singleton.h"
#include "map_get.h"


namespace Util {


/*
 * Abstract factory template
 *
 * \param Base      base class type of concrete implementation which should be returned
 * \param Key       key type for registration in internal map
 */
template<typename Base, typename Creator, typename Key = std::string>
class Factory : public Singleton<Factory<Base, Creator, Key> >
{
public:
    typedef Key key_type;
    void register_class(const Key &_key, Creator *_class_creator)
    {
        // reregistration is not allowed
        if (!class_creators_.insert(std::make_pair(_key, _class_creator)).second) {
            throw std::runtime_error(std::string("key '") + _key + "' already registered");
        }
    }


    /* boost::shared_ptr<Base> create(const Key &_key) const */


    /*
     * Creates instance of class using registered class creator
     *
     * \param _key      name of registered class to instance
     * \return          instance of concrete type through naked Base pointer
     */
    Base* create(const Key &_key) const
    {
        typename FunctionMap::const_iterator it = class_creators_.find(_key);
        if (it != class_creators_.end()) {
            return (it->second)->create();
        }
        else {
            throw std::out_of_range("factory: key not found");
        }
    }

    /*
     * Creates instance of class using registered class creator
     *
     * \param _key      name of registered class to instance
     * \return          instance of concrete type through shared Base pointer
     */
    boost::shared_ptr<Base> create_sh_ptr(const Key &_key) const
    {
        typename FunctionMap::const_iterator it = class_creators_.find(_key);
        if (it != class_creators_.end()) {
            return (it->second)->create_sh_ptr();
        }
        else {
            throw std::out_of_range("factory: key not found");
        }
    }


    /*
     * \return          all registered keys
     */
    std::vector<Key> get_keys() const
    {
        return map_get_keys(class_creators_);

    }


private:
    typedef std::map<Key, Creator* > FunctionMap;
    FunctionMap class_creators_;
};




template<typename Base>
struct ClassCreator
{
    virtual ~ClassCreator(){}
    virtual Base* create() const = 0;
    virtual boost::shared_ptr<Base> create_sh_ptr() const = 0;
};




template<typename Base, typename Derived>
struct DerivedClassCreator : public ClassCreator<Base>
{
    Base* create() const
    {
        return new Derived();
    }

    boost::shared_ptr<Base> create_sh_ptr() const
    {
        return boost::shared_ptr<Base>(
                dynamic_cast<Base*>(new Derived())
        );
    }
};



/*
 * Inherit from this class to autoregister Derived class in factory
 * (type of Factory<Base, Derived, Key>)
 */
template<typename Base, typename Derived, typename Key = std::string>
class FactoryAutoRegister
{
private:
    struct ExecRegistration
    {
        ExecRegistration()
        {
            Factory<Base, ClassCreator<Base>, Key>::instance_ref().register_class(
                    Derived::registration_name(),
                    new DerivedClassCreator<Base, Derived>());
        }
    };

    template<ExecRegistration&> struct ref_it { };

    static ExecRegistration register_object;
    static ref_it<register_object> referrer;
};


template<typename Base, typename Derived, typename Key>
typename FactoryAutoRegister<Base, Derived, Key>::ExecRegistration FactoryAutoRegister<Base, Derived, Key>::register_object;



/*
 * helper for pre main factory object registration take place
 * even if concrete implementations are statically linked
 */
#define FACTORY_MODULE_INIT_DECL(name) \
    bool name##_init(); static bool name = name##_init();

#define FACTORY_MODULE_INIT_DEFI(name) \
    bool name##_init() { return true; }


}

#endif /*FACTORY_H__*/

