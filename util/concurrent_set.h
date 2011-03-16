#ifndef CONCURRENT_SET_H_
#define CONCURRENT_SET_H_

#include <map>
#include <boost/thread.hpp>

template <class T>
class concurrent_set {

typedef typename std::set<T> set_type;
typedef typename set_type::size_type size_type;
typedef typename set_type::iterator iterator;

private:
    set_type aset;
    mutable boost::mutex lock;

public:
    concurrent_set() : aset() { }

    std::pair<iterator, bool>
    insert(const T &elem) {
        boost::mutex::scoped_lock guard(lock);
        return aset.insert(elem);
    }
    bool empty() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.empty();
    }
    size_type size() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.size();
    }
    iterator insert(iterator, const T &x) {
        boost::mutex::scoped_lock guard(lock);
        return aset.insert(x);
    }
    void erase(iterator position) {
        boost::mutex::scoped_lock guard(lock);
        aset.erase(position);
    }
    size_type erase(const T& x) {
        boost::mutex::scoped_lock guard(lock);
        return aset.erase(x);
    }
    void clear() {
        boost::mutex::scoped_lock guard(lock);
        aset.clear();
    }
    iterator begin() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.begin();
    }
    iterator end() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.end();
    }
};

#endif /*CONCURRENT_SET_H_*/
