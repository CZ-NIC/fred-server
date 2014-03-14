#ifndef TESTS_SETUP_UTILS_1121354561334_
#define TESTS_SETUP_UTILS_1121354561334_

#include <fredlib/contact.h>
#include <fredlib/domain.h>
#include <fredlib/keyset.h>
#include <fredlib/nsset.h>

#include <vector>
#include <vector>

struct setup_get_registrar_handle {
    std::string registrar_handle;

    setup_get_registrar_handle( );
};

struct setup_contact {
    std::string                 handle_;
    unsigned long long          id_;
    setup_get_registrar_handle  registrar_;
    Fred::InfoContactOutput     data_;

    setup_contact();
};

struct setup_domain {
    std::string                 fqdn_;
    unsigned long long          id_;
    setup_get_registrar_handle  registrar_;
    setup_contact               owner_;
    Fred::InfoDomainOutput      data_;

    setup_domain();
};

struct setup_keyset {
    std::string                 handle_;
    unsigned long long          id_;
    setup_get_registrar_handle  registrar_;
    Fred::InfoKeysetOutput      data_;

    setup_keyset();
};

struct setup_nsset {
    std::string                 handle_;
    unsigned long long          id_;
    setup_get_registrar_handle  registrar_;
    Fred::InfoNssetOutput       data_;

    setup_nsset();
};

struct setup_nonexistent_object_historyid {
    unsigned long long history_id_;

    setup_nonexistent_object_historyid();
};

#endif
