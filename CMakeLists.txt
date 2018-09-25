cmake_minimum_required(VERSION 3.5)

find_program(IDL_PROGRAM omniidl)
find_program(GIT_PROGRAM git)
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(program_name fred-server)
project(${program_name} CXX)

macro(set_default variable_name variable_value)
    if(NOT ${variable_name})
        set(${variable_name} ${variable_value})
    endif()
endmacro()

macro(normalize_path variable_name variable_value)
    string(REGEX REPLACE "//*" "/" "${variable_name}" "${variable_value}")
    if(NOT "x${${variable_name}}" STREQUAL "x/")
        string(REGEX REPLACE "/$" "" "${variable_name}" "${${variable_name}}")
    endif()
endmacro()

macro(set_default_path variable_name variable_value)
    if(NOT ${variable_name})
        normalize_path(${variable_name} ${variable_value})
    endif()
endmacro()

set_default(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set_default(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT VERSION)
    if(NOT GIT_PROGRAM)
        message(FATAL_ERROR "git not found; you need to pass -DVERSION=")
    endif()
    execute_process(COMMAND ${GIT_PROGRAM} "describe" "--tags"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_VARIABLE RAWVERSION)
    string(REGEX REPLACE "-.*\n" "" VERSION "${RAWVERSION}")
endif()
set(TARBALL_FILE_NAME "${program_name}-${VERSION}")

set_default(IDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../idl/idl)

set_default(PACKAGE ${program_name})

normalize_path(NORMALIZED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if("x${NORMALIZED_INSTALL_PREFIX}" STREQUAL "x/usr/local")
    set(USR_PREFIX "")
    set(USR_SHARE_PREFIX "")
else()
    set(USR_SHARE_PREFIX "/usr")
    if(USE_USR_PREFIX)
        set(USR_PREFIX "/usr")
    else()
        set(USR_PREFIX "")
    endif()
endif()

set_default_path(BINDIR ${CMAKE_INSTALL_PREFIX}/${USR_PREFIX}/bin)
set_default_path(DATAROOTDIR ${CMAKE_INSTALL_PREFIX}/${USR_SHARE_PREFIX}/share)
set_default_path(LOCALSTATEDIR ${CMAKE_INSTALL_PREFIX}/var)
set_default_path(MONITORINGIPS ${CMAKE_INSTALL_PREFIX}/etc/fred/monitoring_hosts.conf)
set_default_path(SBINDIR ${CMAKE_INSTALL_PREFIX}/${USR_PREFIX}/sbin)
set_default_path(SYSCONFDIR ${CMAKE_INSTALL_PREFIX}/etc)
set_default_path(PREFIX ${CMAKE_INSTALL_PREFIX})

set_default_path(CONFIG_FILE ${SYSCONFDIR}/fred/server.conf)
set_default_path(HPMAIL_CONFIG ${SYSCONFDIR}/fred/hpmail.conf)
set_default_path(OPTYS_CONFIG ${SYSCONFDIR}/fred/optysmail.conf)

set_default(DBHOST localhost)
set_default(DBNAME fred)
set_default(DBPASS password)
set_default(DBUSER fred)
set_default(EPP_OPERATIONS_CHARGING false)
set_default(HAVE_BOOST_SERIALIZATION 1)
set_default(HAVE_LOGGER 1)
set_default(HP_LOGIN_NAME dreplech)
set_default(HP_LOGIN_NOTE test_fred)
set_default(HP_LOGIN_PASSWORD dreplech)
set_default(NSCONTEXT fred)
set_default(NSHOST localhost)
set_default(OPTYS_LOGIN_NAME test)
set_default(OPTYS_LOGIN_PASSWORD test)
set_default_path(TEST_DATA ${DATAROOTDIR})

set_default(OPTYS_LOGIN_PORT 22)
set_default(RIFDPORT 2222)
set_default(PIFDPORT 2223)
set_default(ADIFDPORT 2224)
set_default(LOGDPORT 2226)
set_default(MIFDPORT 2227)
set_default(MSGDPORT 2228)
set_default(DBIFDPORT 2229)
set_default(AKMDPORT 2230)
set_default(RSIFDPORT 2231)
set_default(ACCIFDPORT 2232)
set_default(NSPORT 2809)
set_default(DBPORT 5432)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/hpmail.conf.in ${CMAKE_BINARY_DIR}/config/hpmail.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/optysmail.conf.in ${CMAKE_BINARY_DIR}/config/optysmail.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/server.conf.in ${CMAKE_BINARY_DIR}/config/server.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/init-scripts/fred-server.in ${CMAKE_BINARY_DIR}/init-scripts/fred-server @ONLY)

find_package(Threads REQUIRED)
set(Threads_LIBRARIES Threads::Threads)
find_package(PostgreSQL REQUIRED)
find_package(Boost 1.53.0
    COMPONENTS
    system
    thread
    date_time
    program_options
    regex
    random
    serialization
    filesystem
    unit_test_framework
    REQUIRED)

set_default(BOOST_INCLUDE_DOXYGEN_PATH ${Boost_INCLUDE_DIRS})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxy.conf.in ${CMAKE_BINARY_DIR}/doxy.conf)

find_package(PkgConfig REQUIRED)

pkg_search_module(LIBSSH REQUIRED libssh)
pkg_search_module(ZLIB REQUIRED zlib)
pkg_search_module(LIBCURL REQUIRED libcurl)
pkg_search_module(MINIZIP REQUIRED minizip)
pkg_search_module(LIBIDN REQUIRED libidn>=0.5.18)
pkg_search_module(LIBCRYPTO REQUIRED libcrypto)
pkg_search_module(XML2 REQUIRED libxml-2.0>=2.5.0)
pkg_search_module(OMNIORB4 REQUIRED omniORB4>=4.1.2)
pkg_search_module(OMNIDYNAMIC4 REQUIRED omniDynamic4>=4.1.2)
pkg_search_module(OPENSSL REQUIRED openssl)
pkg_search_module(LIBCURL REQUIRED libcurl)

include(CheckIncludeFileCXX)
function(abort_if_headers_not_found)
    foreach(header_name ${ARGN})
        check_include_file_cxx(${header_name} FOUND${header_name})
        if(NOT FOUND${header_name})
            message(FATAL_ERROR "Header ${header_name} not found!")
        endif()
    endforeach()
endfunction()

# In the name of speed, use this only for headers from libraries for which we don't have some other form of detection.
abort_if_headers_not_found(
    mpdecimal.h)

set(Resolv_LIBRARIES resolv)
set(Rt_LIBRARIES rt)
set(Mpdec_LIBRARIES mpdec)

function(target_link_external_libraries target_name)
    foreach(library_name ${ARGN})
        target_link_libraries(${target_name} "${${linker_info_${library_name}}}")
    endforeach()
endfunction()

macro(store_linker_info key value)
    set(linker_info_${key} "${value}")
endmacro()

store_linker_info(resolv Resolv_LIBRARIES)
store_linker_info(rt Rt_LIBRARIES)
store_linker_info(mpdec Mpdec_LIBRARIES)
store_linker_info(threads Threads_LIBRARIES)
store_linker_info(boost_system Boost_SYSTEM_LIBRARY)
store_linker_info(boost_thread Boost_THREAD_LIBRARY)
store_linker_info(boost_date_time Boost_DATE_TIME_LIBRARY)
store_linker_info(boost_random Boost_RANDOM_LIBRARY)
store_linker_info(boost_regex Boost_REGEX_LIBRARY)
store_linker_info(boost_filesystem Boost_FILESYSTEM_LIBRARY)
store_linker_info(boost_serialization Boost_SERIALIZATION_LIBRARY)
store_linker_info(boost_program_options Boost_PROGRAM_OPTIONS_LIBRARY)
store_linker_info(boost_unit_test_framework Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
store_linker_info(libcrypto LIBCRYPTO_LIBRARIES)
store_linker_info(libcurl LIBCURL_LIBRARIES)
store_linker_info(libidn LIBIDN_LIBRARIES)
store_linker_info(libssh LIBSSH_LIBRARIES)
store_linker_info(libxml2 XML2_LIBRARIES)
store_linker_info(minizip MINIZIP_LIBRARIES)
store_linker_info(omniorb4 OMNIORB4_LIBRARIES)
store_linker_info(omnidynamic4 OMNIDYNAMIC4_LIBRARIES)
store_linker_info(openssl OPENSSL_LIBRARIES)
store_linker_info(zlib ZLIB_LIBRARIES)
store_linker_info(postgres PostgreSQL_LIBRARIES)
store_linker_info(libcurl LIBCURL_LIBRARIES)


macro(create_compile_flags_function some_package its_flags)
    function(compile_targets_with_flags_for_${some_package})
        set(func_ARGN ARGN)
        foreach(target_name ${${func_ARGN}})
            message(STATUS "compile with flags for " ${some_package} ":" ${target_name})
            target_compile_options(${target_name} PUBLIC ${${its_flags}})
        endforeach()
    endfunction()
endmacro()

create_compile_flags_function(libcrypto LIBCRYPTO_CXXFLAGS)
create_compile_flags_function(libcurl LIBCURL_CXXFLAGS)
create_compile_flags_function(libidn LIBIDN_CXXFLAGS)
create_compile_flags_function(libssh LIBSSH_CXXFLAGS)
create_compile_flags_function(libxml2 XML2_CXXFLAGS)
create_compile_flags_function(minizip MINIZIP_CXXFLAGS)
create_compile_flags_function(omniorb4 OMNIORB4_CXXFLAGS)
create_compile_flags_function(openssl OPENSSL_CXXFLAGS)
create_compile_flags_function(zlib ZLIB_CXXFLAGS)

macro(create_include_function some_package its_includes)
    function(include_${some_package}_in_targets)
        set(func_ARGN ARGN)
        foreach(target_name ${${func_ARGN}})
            message(STATUS "include " ${some_package} " in: " ${target_name})
            target_include_directories(${target_name} PUBLIC ${${its_includes}})
        endforeach()
    endfunction()
endmacro()

create_include_function(boost Boost_INCLUDE_DIRS)
create_include_function(libcrypto LIBCRYPTO_INCLUDE_DIRS)
create_include_function(libcurl LIBCURL_LIBRARIES)
create_include_function(libidn LIBIDN_INCLUDE_DIRS)
create_include_function(libssh LIBSSH_INCLUDE_DIRS)
create_include_function(libxml2 XML2_INCLUDE_DIRS)
create_include_function(minizip MINIZIP_LIBRARIES)
create_include_function(omniorb4 OMNIORB4_INCLUDE_DIRS)
create_include_function(omnidynamic4 OMNIDYNAMIC4_DIRS)
create_include_function(openssl OPENSSL_INCLUDE_DIRS)
create_include_function(zlib ZLIB_INCLUDE_DIRS)
create_include_function(postgres PostgreSQL_INCLUDE_DIRS)
create_include_function(libcurl LIBCURL_INCLUDE_DIRS)

function(set_common_properties_on_targets)
    foreach(target_name ${ARGN})
        message(STATUS "setting properties on: " ${target_name})
        set_target_properties(${target_name} PROPERTIES
            CXX_STANDARD 14
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
        target_compile_options(${target_name}
            PUBLIC "-ggdb"
            PUBLIC "-grecord-gcc-switches"
            PUBLIC "-Wall"
            PUBLIC "-Wextra"
            PUBLIC "-Wno-unused-parameter"
            PUBLIC "-fdiagnostics-color=auto")
        target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${target_name} PUBLIC ${CMAKE_BINARY_DIR})
    endforeach()
endfunction()

function(prepare_object_files)
    foreach(src_files ${ARGN})
        if(${src_files} MATCHES "hh$")
            message(FATAL_ERROR ${src_files} " is a header!  There's no point in compiling it.")
        endif()
        if(NOT ${src_files} MATCHES "cc$")
            message(FATAL_ERROR ${src_files} " is not a cc file!  There's no point in compiling it.")
        endif()
        message(STATUS "adding an object file: " ${src_files})
        string(REGEX REPLACE "\.cc$" "_o" name ${src_files})
        string(REGEX REPLACE "\/" "_" name ${name})
        add_library(${name} OBJECT ${src_files})
        set_common_properties_on_targets(${name})
        include_boost_in_targets(${name})
        include_libxml2_in_targets(${name})
        include_postgres_in_targets(${name})
    endforeach()
endfunction()

function(prepare_static_library name)
    set(add_library_arguments "")
    foreach(src_file ${ARGN})
        string(REGEX REPLACE "\.cc$" "_o" object_file_name ${src_file})
        string(REGEX REPLACE "\/" "_" object_file_name ${object_file_name})
        list(APPEND add_library_arguments "$<TARGET_OBJECTS:${object_file_name}>")
    endforeach()
    add_library(${name} STATIC ${add_library_arguments})
    set_common_properties_on_targets(${name})
endfunction()

function(prepare_executable name)
    set(add_executable_arguments "")
    foreach(src_file ${ARGN})
        string(REGEX REPLACE "\.cc$" "_o" object_file_name ${src_file})
        string(REGEX REPLACE "\/" "_" object_file_name ${object_file_name})
        list(APPEND add_executable_arguments "$<TARGET_OBJECTS:${object_file_name}>")
    endforeach()
    add_executable(${name} ${add_executable_arguments})
    set_common_properties_on_targets(${name})
endfunction()

function(compile_idl_files)
    message(STATUS "Compiling idl...")
    if(NOT IDL_PROGRAM)
        message(FATAL_ERROR "omniidl not found")
    endif()
    execute_process(
        COMMAND mkdir -p src/bin/corba
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    foreach(idl_name ${ARGN})
        if(NOT EXISTS ${IDL_DIR}/${idl_name}.idl)
            message(FATAL_ERROR "${IDL_DIR}/${idl_name}.idl does not exist; you need to pass -DIDL_DIR")
        endif()
        execute_process(
            COMMAND ${IDL_PROGRAM} -bcxx -Wbuse_quotes -Wba -C . ${IDL_DIR}/${idl_name}.idl
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/bin/corba
            OUTPUT_VARIABLE OMNIIDL_MESSAGE${idl_name})
        message(STATUS ${OMNIIDL_MESSAGE}${idl_name})
    endforeach()
    message(STATUS "...done")
endfunction()

compile_idl_files(
    Accounting
    Admin
    AdminContactVerification
    AdministrativeBlocking
    AutomaticKeysetManagement
    Buffer
    ContactVerification
    _dataTypes
    DateTime
    DateTime
    DomainBrowser
    EPP
    FileManager
    Filters
    IsoDate
    IsoDateTime
    Logger
    Mailer
    Messages
    MojeID
    Notification
    NullableIsoDate
    NullableIsoDateTime
    NullableTypes
    PublicRequest
    RecordStatement
    Registry
    TechCheck
    Whois
    Whois2
    Zone
    ZoneGenerator)

execute_process(
    COMMAND omniidl -bcxx -Wbuse_quotes -Wba -C . ${CMAKE_CURRENT_SOURCE_DIR}/test/util/corba_conversion_test.idl
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/bin/corba
    OUTPUT_VARIABLE OMNIIDL_MESSAGE${idl_name})
message(STATUS ${OMNIIDL_MESSAGE}${idl_name})

prepare_object_files(
    src/backend/accounting/accounting.cc
    src/backend/accounting/exceptions.cc
    src/backend/accounting/impl/accounting.cc
    src/backend/accounting/impl/exceptions.cc
    src/backend/admin_block/administrativeblocking.cc
    src/backend/admin/contact/merge_contact_auto_procedure.cc
    src/backend/admin/contact/merge_contact.cc
    src/backend/admin/contact/merge_contact_logger.cc
    src/backend/admin/contact/merge_contact_reporting.cc
    src/backend/admin/contact/verification/contact_states/delete_all.cc
    src/backend/admin/contact/verification/create_test_impl_prototypes.cc
    src/backend/admin/contact/verification/delete_domains_of_invalid_contact.cc
    src/backend/admin/contact/verification/enqueue_check.cc
    src/backend/admin/contact/verification/fill_check_queue.cc
    src/backend/admin/contact/verification/list_active_checks.cc
    src/backend/admin/contact/verification/related_records.cc
    src/backend/admin/contact/verification/resolve_check.cc
    src/backend/admin/contact/verification/run_all_enqueued_checks.cc
    src/backend/admin/contact/verification/test_impl/test_contactability.cc
    src/backend/admin/contact/verification/test_impl/test_cz_address_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.cc
    src/backend/admin/contact/verification/test_impl/test_email_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_interface.cc
    src/backend/admin/contact/verification/test_impl/test_name_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_phone_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_send_letter.cc
    src/backend/admin/contact/verification/update_tests.cc
    src/backend/admin/domain/create_expired_domain.cc
    src/backend/admin/notification/notification.cc
    src/backend/admin/zone/zone.cc
    src/backend/automatic_keyset_management/automatic_keyset_management.cc
    src/backend/automatic_keyset_management/dns_key.cc
    src/backend/automatic_keyset_management/impl/logger_request_data.cc
    src/backend/automatic_keyset_management/impl/logger_request_object_type.cc
    src/backend/automatic_keyset_management/impl/logger_request_property.cc
    src/backend/automatic_keyset_management/impl/logger_request_type.cc
    src/backend/automatic_keyset_management/impl/util.cc
    src/backend/buffer.cc
    src/backend/credit.cc
    src/backend/contact_verification/contact_verification_impl.cc
    src/backend/contact_verification/public_request_contact_verification_impl.cc
    src/backend/domain_browser/domain_browser.cc
    src/backend/epp/contact/authinfo_contact.cc
    src/backend/epp/contact/authinfo_contact_localized.cc
    src/backend/epp/contact/check_contact.cc
    src/backend/epp/contact/check_contact_localized.cc
    src/backend/epp/contact/config_data_filter.cc
    src/backend/epp/contact/contact_change.cc
    src/backend/epp/contact/contact_data.cc
    src/backend/epp/contact/create_contact.cc
    src/backend/epp/contact/create_contact_config_data.cc
    src/backend/epp/contact/create_contact_input_data.cc
    src/backend/epp/contact/create_contact_localized.cc
    src/backend/epp/contact/delete_contact.cc
    src/backend/epp/contact/delete_contact_localized.cc
    src/backend/epp/contact/impl/contact_output.cc
    src/backend/epp/contact/impl/cznic/create_contact_data_filter.cc
    src/backend/epp/contact/impl/cznic/specific.cc
    src/backend/epp/contact/impl/cznic/update_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/create_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/config.cc
    src/backend/epp/contact/impl/set_unused/update_contact_data_filter.cc
    src/backend/epp/contact/impl/get_create_contact_data_filter.cc
    src/backend/epp/contact/impl/get_personal_id_union.cc
    src/backend/epp/contact/impl/get_update_contact_data_filter.cc
    src/backend/epp/contact/info_contact.cc
    src/backend/epp/contact/info_contact_localized.cc
    src/backend/epp/contact/transfer_contact.cc
    src/backend/epp/contact/transfer_contact_localized.cc
    src/backend/epp/contact/update_contact.cc
    src/backend/epp/contact/update_contact_config_data.cc
    src/backend/epp/contact/update_contact_localized.cc
    src/backend/epp/contact/update_contact_post_hooks.cc
    src/backend/epp/contact/util.cc
    src/backend/epp/credit/client_credit.cc
    src/backend/epp/credit/client_credit_localized.cc
    src/backend/epp/credit/client_credit_output_data.cc
    src/backend/epp/domain/authinfo_domain.cc
    src/backend/epp/domain/authinfo_domain_localized.cc
    src/backend/epp/domain/check_domain.cc
    src/backend/epp/domain/check_domain_localized.cc
    src/backend/epp/domain/create_domain.cc
    src/backend/epp/domain/create_domain_localized.cc
    src/backend/epp/domain/delete_domain.cc
    src/backend/epp/domain/delete_domain_localized.cc
    src/backend/epp/domain/domain_enum_validation.cc
    src/backend/epp/domain/impl/domain_billing.cc
    src/backend/epp/domain/impl/domain_output.cc
    src/backend/epp/domain/info_domain.cc
    src/backend/epp/domain/info_domain_localized.cc
    src/backend/epp/domain/renew_domain.cc
    src/backend/epp/domain/renew_domain_localized.cc
    src/backend/epp/domain/transfer_domain.cc
    src/backend/epp/domain/transfer_domain_localized.cc
    src/backend/epp/domain/update_domain.cc
    src/backend/epp/domain/update_domain_localized.cc
    src/backend/epp/epp_extended_error.cc
    src/backend/epp/epp_extended_error_localized.cc
    src/backend/epp/epp_response_failure_localized.cc
    src/backend/epp/epp_response_success_localized.cc
    src/backend/epp/epp_result_failure.cc
    src/backend/epp/epp_result_failure_localized.cc
    src/backend/epp/epp_result_success_localized.cc
    src/backend/epp/impl/conditionally_enqueue_notification.cc
    src/backend/epp/impl/registraracl/authentic_registrar.cc
    src/backend/epp/keyset/authinfo_keyset.cc
    src/backend/epp/keyset/authinfo_keyset_localized.cc
    src/backend/epp/keyset/check_keyset.cc
    src/backend/epp/keyset/check_keyset_localized.cc
    src/backend/epp/keyset/create_keyset.cc
    src/backend/epp/keyset/create_keyset_localized.cc
    src/backend/epp/keyset/delete_keyset.cc
    src/backend/epp/keyset/delete_keyset_localized.cc
    src/backend/epp/keyset/dns_key.cc
    src/backend/epp/keyset/impl/keyset_output.cc
    src/backend/epp/keyset/info_keyset.cc
    src/backend/epp/keyset/info_keyset_localized.cc
    src/backend/epp/keyset/transfer_keyset.cc
    src/backend/epp/keyset/transfer_keyset_localized.cc
    src/backend/epp/keyset/update_keyset.cc
    src/backend/epp/keyset/update_keyset_localized.cc
    src/backend/epp/localization.cc
    src/backend/epp/nsset/authinfo_nsset.cc
    src/backend/epp/nsset/authinfo_nsset_localized.cc
    src/backend/epp/nsset/check_nsset.cc
    src/backend/epp/nsset/check_nsset_localized.cc
    src/backend/epp/nsset/create_nsset.cc
    src/backend/epp/nsset/create_nsset_localized.cc
    src/backend/epp/nsset/delete_nsset.cc
    src/backend/epp/nsset/delete_nsset_localized.cc
    src/backend/epp/nsset/impl/nsset.cc
    src/backend/epp/nsset/impl/nsset_output.cc
    src/backend/epp/nsset/info_nsset.cc
    src/backend/epp/nsset/info_nsset_localized.cc
    src/backend/epp/nsset/transfer_nsset.cc
    src/backend/epp/nsset/transfer_nsset_localized.cc
    src/backend/epp/nsset/update_nsset.cc
    src/backend/epp/nsset/update_nsset_localized.cc
    src/backend/epp/poll/poll_acknowledgement.cc
    src/backend/epp/poll/poll_acknowledgement_localized.cc
    src/backend/epp/poll/poll_request.cc
    src/backend/epp/poll/poll_request_get_update_contact_details.cc
    src/backend/epp/poll/poll_request_get_update_contact_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_domain_details.cc
    src/backend/epp/poll/poll_request_get_update_domain_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_keyset_details.cc
    src/backend/epp/poll/poll_request_get_update_keyset_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_nsset_details.cc
    src/backend/epp/poll/poll_request_get_update_nsset_details_localized.cc
    src/backend/epp/poll/poll_request_localized.cc
    src/backend/epp/public_request/create_authinfo_request_registry_email.cc
    src/backend/epp/session_data.cc
    src/backend/mojeid/checkers.cc
    src/backend/mojeid/messages/generate.cc
    src/backend/mojeid/mojeid.cc
    src/backend/mojeid/mojeid_impl_data_conversion.cc
    src/backend/mojeid/mojeid_impl_internal.cc
    src/backend/mojeid/mojeid_public_request.cc
    src/backend/public_request/create_authinfo_request_non_registry_email.cc
    src/backend/public_request/create_authinfo_request_registry_email.cc
    src/backend/public_request/create_block_unblock_request.cc
    src/backend/public_request/create_personal_info_request_non_registry_email.cc
    src/backend/public_request/create_personal_info_request_registry_email.cc
    src/backend/public_request/create_public_request_pdf.cc
    src/backend/public_request/exceptions.cc
    src/backend/public_request/get_default_document_manager.cc
    src/backend/public_request/get_id_of_registered_object.cc
    src/backend/public_request/get_valid_registry_emails_of_registered_object.cc
    src/backend/public_request/process/authinfo.cc
    src/backend/public_request/process/block_unblock.cc
    src/backend/public_request/process/exceptions.cc
    src/backend/public_request/process/personal_info.cc
    src/backend/public_request/type/impl/public_request_authinfo.cc
    src/backend/public_request/type/impl/public_request_block_unblock.cc
    src/backend/public_request/type/impl/public_request_personal_info.cc
    src/backend/public_request/util/send_joined_address_email.cc
    src/backend/record_statement/exceptions.cc
    src/backend/record_statement/impl/europe_prague_impl.cc
    src/backend/record_statement/impl/factory.cc
    src/backend/record_statement/impl/record_statement_xml.cc
    src/backend/record_statement/impl/utc_impl.cc
    src/backend/record_statement/impl/util.cc
    src/backend/record_statement/record_statement.cc
    src/backend/whois/domain_expiration_datetime.cc
    src/backend/whois/is_domain_delete_pending.cc
    src/backend/whois/nameserver_exists.cc
    src/backend/whois/registrar_certification.cc
    src/backend/whois/registrar_group.cc
    src/backend/whois/whois.cc
    src/backend/whois/zone_list.cc
    src/bin/cli/bankclient.cc
    src/bin/cli/chargeclient.cc
    src/bin/cli/commonclient.cc
    src/bin/cli/contactclient.cc
    src/bin/cli/domainclient.cc
    src/bin/cli/enumparamclient.cc
    src/bin/cli/fileclient.cc
    src/bin/cli/fred-admin.cc
    src/bin/cli/invoiceclient.cc
    src/bin/cli/keysetclient.cc
    src/bin/cli/notifyclient.cc
    src/bin/cli/objectclient.cc
    src/bin/cli/optys_get_undelivered.cc
    src/bin/cli/pollclient.cc
    src/bin/cli/public_request_method.cc
    src/bin/cli/registrarclient.cc
    src/bin/cli/remove_delete_candidates.cc
    src/bin/corba/AccountingDynSK.cc
    src/bin/corba/AccountingSK.cc
    src/bin/corba/accounting/accounting_i.cc
    src/bin/corba/accounting/fred-accifd.cc
    src/bin/corba/accounting/impl/corba_conversions.cc
    src/bin/corba/accounting/impl/exceptions.cc
    src/bin/corba/admin/admin_impl.cc
    src/bin/corba/admin/bankinginvoicing_impl.cc
    src/bin/corba/admin_block/server_i.cc
    src/bin/corba/admin/common.cc
    src/bin/corba/AdminContactVerificationDynSK.cc
    src/bin/corba/admin_contact_verification/server_i.cc
    src/bin/corba/AdminContactVerificationSK.cc
    src/bin/corba/AdminDynSK.cc
    src/bin/corba/admin/filter_impl.cc
    src/bin/corba/AdministrativeBlockingDynSK.cc
    src/bin/corba/AdministrativeBlockingSK.cc
    src/bin/corba/admin/pagetable_contacts.cc
    src/bin/corba/admin/pagetable_domains.cc
    src/bin/corba/admin/pagetable_files.cc
    src/bin/corba/admin/pagetable_filters.cc
    src/bin/corba/admin/pagetable_impl.cc
    src/bin/corba/admin/pagetable_invoices.cc
    src/bin/corba/admin/pagetable_keysets.cc
    src/bin/corba/admin/pagetable_logger.cc
    src/bin/corba/admin/pagetable_logsession.cc
    src/bin/corba/admin/pagetable_mails.cc
    src/bin/corba/admin/pagetable_messages.cc
    src/bin/corba/admin/pagetable_nssets.cc
    src/bin/corba/admin/pagetable_publicrequests.cc
    src/bin/corba/admin/pagetable_registrars.cc
    src/bin/corba/admin/pagetable_zones.cc
    src/bin/corba/admin/public_request_mojeid.cc
    src/bin/corba/admin/session_impl.cc
    src/bin/corba/AdminSK.cc
    src/bin/corba/admin/user_impl.cc
    src/bin/corba/AutomaticKeysetManagementDynSK.cc
    src/bin/corba/automatic_keyset_management/fred-akmd.cc
    src/bin/corba/automatic_keyset_management/server_i.cc
    src/bin/corba/AutomaticKeysetManagementSK.cc
    src/bin/corba/BufferDynSK.cc
    src/bin/corba/BufferSK.cc
    src/bin/corba/contact_verification/contact_verification_i.cc
    src/bin/corba/ContactVerificationDynSK.cc
    src/bin/corba/ContactVerificationSK.cc
    src/bin/corba/corba_conversion_testDynSK.cc
    src/bin/corba/corba_conversion_testSK.cc
    src/bin/corba/_dataTypesDynSK.cc
    src/bin/corba/_dataTypesSK.cc
    src/bin/corba/DateTimeDynSK.cc
    src/bin/corba/DateTimeSK.cc
    src/bin/corba/domain_browser/domain_browser_i.cc
    src/bin/corba/DomainBrowserDynSK.cc
    src/bin/corba/domain_browser/fred-dbifd.cc
    src/bin/corba/DomainBrowserSK.cc
    src/bin/corba/epp/contact/contact_corba_conversions.cc
    src/bin/corba/epp_corba_client_impl.cc
    src/bin/corba/epp/corba_conversions.cc
    src/bin/corba/epp/credit/credit_corba_conversions.cc
    src/bin/corba/epp/domain/domain_corba_conversions.cc
    src/bin/corba/EPPDynSK.cc
    src/bin/corba/epp/epp_impl.cc
    src/bin/corba/epp/epp_session.cc
    src/bin/corba/epp/keyset/keyset_corba_conversions.cc
    src/bin/corba/epp/messages.cc
    src/bin/corba/epp/nsset/nsset_corba_conversions.cc
    src/bin/corba/epp/poll/poll_corba_conversions.cc
    src/bin/corba/EPPSK.cc
    src/bin/corba/epp/tech_check.cc
    src/bin/corba/file_manager_client.cc
    src/bin/corba/FileManagerDynSK.cc
    src/bin/corba/FileManagerSK.cc
    src/bin/corba/FiltersDynSK.cc
    src/bin/corba/FiltersSK.cc
    src/bin/corba/fred-adifd.cc
    src/bin/corba/fred-logd.cc
    src/bin/corba/fred-pifd.cc
    src/bin/corba/fred-rifd.cc
    src/bin/corba/IsoDateDynSK.cc
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/logger_client_impl.cc
    src/bin/corba/LoggerDynSK.cc
    src/bin/corba/LoggerSK.cc
    src/bin/corba/log/log_impl_wrap.cc
    src/bin/corba/MailerDynSK.cc
    src/bin/corba/mailer_manager.cc
    src/bin/corba/MailerSK.cc
    src/bin/corba/MessagesDynSK.cc
    src/bin/corba/messages/fred-msgd.cc
    src/bin/corba/messages/messages.cc
    src/bin/corba/messages/messages_filemanager.cc
    src/bin/corba/MessagesSK.cc
    src/bin/corba/MojeIDDynSK.cc
    src/bin/corba/mojeid/fred-mifd.cc
    src/bin/corba/mojeid/messages/client.cc
    src/bin/corba/mojeid/mojeid_corba_conversion.cc
    src/bin/corba/mojeid/server_i.cc
    src/bin/corba/mojeid/service_name.cc
    src/bin/corba/MojeIDSK.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/NotificationDynSK.cc
    src/bin/corba/notification/notification_corba_conversion.cc
    src/bin/corba/notification/server_i.cc
    src/bin/corba/NotificationSK.cc
    src/bin/corba/NullableIsoDateDynSK.cc
    src/bin/corba/NullableIsoDateSK.cc
    src/bin/corba/NullableIsoDateTimeDynSK.cc
    src/bin/corba/NullableIsoDateTimeSK.cc
    src/bin/corba/NullableTypesDynSK.cc
    src/bin/corba/NullableTypesSK.cc
    src/bin/corba/PublicRequestDynSK.cc
    src/bin/corba/public_request/server_i.cc
    src/bin/corba/PublicRequestSK.cc
    src/bin/corba/RecordStatementDynSK.cc
    src/bin/corba/record_statement/fred-rsifd.cc
    src/bin/corba/record_statement/record_statement_i.cc
    src/bin/corba/RecordStatementSK.cc
    src/bin/corba/RegistryDynSK.cc
    src/bin/corba/RegistrySK.cc
    src/bin/corba/TechCheckDynSK.cc
    src/bin/corba/TechCheckSK.cc
    src/bin/corba/util/corba_conversions_buffer.cc
    src/bin/corba/util/corba_conversions_isodate.cc
    src/bin/corba/util/corba_conversions_isodatetime.cc
    src/bin/corba/util/corba_conversions_money.cc
    src/bin/corba/util/corba_conversions_nullable_types.cc
    src/bin/corba/util/corba_conversions_nullableisodate.cc
    src/bin/corba/util/corba_conversions_nullableisodatetime.cc
    src/bin/corba/util/corba_conversions_string.cc
    src/bin/corba/Whois2DynSK.cc
    src/bin/corba/Whois2SK.cc
    src/bin/corba/WhoisDynSK.cc
    src/bin/corba/WhoisSK.cc
    src/bin/corba/whois/whois2_impl.cc
    src/bin/corba/whois/whois_impl.cc
    src/bin/corba/ZoneDynSK.cc
    src/bin/corba/ZoneGeneratorDynSK.cc
    src/bin/corba/ZoneGeneratorSK.cc
    src/bin/corba/ZoneSK.cc
    src/deprecated/model/contact_filter.cc
    src/deprecated/model/domain_filter.cc
    src/deprecated/model/file_filter.cc
    src/deprecated/model/filter_filter.cc
    src/deprecated/model/invoice_filter.cc
    src/deprecated/model/keyset_filter.cc
    src/deprecated/model/log_filter.cc
    src/deprecated/model/mail_filter.cc
    src/deprecated/model/message_filter.cc
    src/deprecated/model/nsset_filter.cc
    src/deprecated/model/object_filter.cc
    src/deprecated/model/object_registry_filter.cc
    src/deprecated/model/object_state_filter.cc
    src/deprecated/model/online_statement_filter.cc
    src/deprecated/model/public_request_filter.cc
    src/deprecated/model/registrar_filter.cc
    src/deprecated/model/test-model.cc
    src/deprecated/model/union_filter.cc
    src/deprecated/model/zone_filter.cc
    src/deprecated/model/zone_ns_filter.cc
    src/deprecated/model/zone_soa_filter.cc
    src/deprecated/util/dbsql.cc
    src/deprecated/util/log.cc
    src/deprecated/util/pqsql.cc
    src/deprecated/util/timeclock.cc
    src/deprecated/util/util.cc
    src/libfred/banking/bank_common.cc
    src/libfred/banking/bank_manager.cc
    src/libfred/banking/exceptions.cc
    src/libfred/banking/get_bank_accounts.cc
    src/libfred/blacklist.cc
    src/libfred/common_diff.cc
    src/libfred/common_impl.cc
    src/libfred/common_impl_new.cc
    src/libfred/contact_verification/cancel_contact_verification.cc
    src/libfred/contact_verification/contact.cc
    src/libfred/contact_verification/contact_conditional_identification_impl.cc
    src/libfred/contact_verification/contact_identification_impl.cc
    src/libfred/contact_verification/contact_verification_checkers.cc
    src/libfred/contact_verification/contact_verification_password.cc
    src/libfred/contact_verification/contact_verification_state.cc
    src/libfred/contact_verification/contact_verification_validators.cc
    src/libfred/credit.cc
    src/libfred/documents.cc
    src/libfred/file.cc
    src/libfred/filter.cc
    src/libfred/info_buffer.cc
    src/libfred/invoicing/invoice.cc
    src/libfred/mail.cc
    src/libfred/messages/messages_impl.cc
    src/libfred/model_registrar_acl.cc
    src/libfred/model_registrar.cc
    src/libfred/notifier/enqueue_notification.cc
    src/libfred/notifier/gather_email_data/gather_email_addresses.cc
    src/libfred/notifier/gather_email_data/gather_email_content.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/contact.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/domain.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/nsset.cc
    src/libfred/notifier/process_one_notification_request.cc
    src/libfred/notify.cc
    src/libfred/object/check_handle.cc
    src/libfred/object/generate_authinfo_password.cc
    src/libfred/object/generated_authinfo_password.cc
    src/libfred/object/get_id_of_registered.cc
    src/libfred/object_impl.cc
    src/libfred/object/object.cc
    src/libfred/object/object_id_handle_pair.cc
    src/libfred/object/object_impl.cc
    src/libfred/object_state/cancel_object_state_request_id.cc
    src/libfred/object_state/clear_admin_object_state_request_id.cc
    src/libfred/object_state/clear_object_state_request_id.cc
    src/libfred/object_state/create_admin_object_block_request_id.cc
    src/libfred/object_state/create_admin_object_state_restore_request_id.cc
    src/libfred/object_state/create_object_state_request_id.cc
    src/libfred/object_state/get_blocking_status_desc_list.cc
    src/libfred/object_state/get_object_state_descriptions.cc
    src/libfred/object_state/get_object_state_id_map.cc
    src/libfred/object_state/get_object_states_by_history_id.cc
    src/libfred/object_state/get_object_states.cc
    src/libfred/object_state/lock_object_state_request_lock.cc
    src/libfred/object_state/object_has_state.cc
    src/libfred/object_state/perform_object_state_request.cc
    src/libfred/object_states.cc
    src/libfred/object/transfer_object.cc
    src/libfred/opcontext.cc
    src/libfred/poll/create_low_credit_messages.cc
    src/libfred/poll/create_poll_message.cc
    src/libfred/poll/create_request_fee_info_message.cc
    src/libfred/poll/create_request_fee_info_messages.cc
    src/libfred/poll/create_state_messages.cc
    src/libfred/poll/create_update_object_poll_message.cc
    src/libfred/poll/get_request_fee_message.cc
    src/libfred/public_request/create_public_request_auth.cc
    src/libfred/public_request/create_public_request.cc
    src/libfred/public_request/get_opened_public_request.cc
    src/libfred/public_request/info_public_request_auth.cc
    src/libfred/public_request/info_public_request.cc
    src/libfred/public_request/public_request_authinfo_impl.cc
    src/libfred/public_request/public_request_block_impl.cc
    src/libfred/public_request/public_request.cc
    src/libfred/public_request/public_request_impl.cc
    src/libfred/public_request/public_request_lock_guard.cc
    src/libfred/public_request/public_request_object_lock_guard.cc
    src/libfred/public_request/public_request_personal_info_impl.cc
    src/libfred/public_request/update_public_request.cc
    src/libfred/registrable_object/contact.cc
    src/libfred/registrable_object/contact/check_contact.cc
    src/libfred/registrable_object/contact/copy_contact.cc
    src/libfred/registrable_object/contact/copy_history_impl.cc
    src/libfred/registrable_object/contact/create_contact.cc
    src/libfred/registrable_object/contact/delete_contact.cc
    src/libfred/registrable_object/contact/find_contact_duplicates.cc
    src/libfred/registrable_object/contact/info_contact.cc
    src/libfred/registrable_object/contact/info_contact_data.cc
    src/libfred/registrable_object/contact/info_contact_diff.cc
    src/libfred/registrable_object/contact/info_contact_impl.cc
    src/libfred/registrable_object/contact/info_contact_output.cc
    src/libfred/registrable_object/contact/merge_contact.cc
    src/libfred/registrable_object/contact/merge_contact_email_notification_data.cc
    src/libfred/registrable_object/contact/merge_contact_selection.cc
    src/libfred/registrable_object/contact/place_address.cc
    src/libfred/registrable_object/contact/transfer_contact.cc
    src/libfred/registrable_object/contact/undisclose_address.cc
    src/libfred/registrable_object/contact/update_contact.cc
    src/libfred/registrable_object/contact/util.cc
    src/libfred/registrable_object/contact/verification/create_check.cc
    src/libfred/registrable_object/contact/verification/create_test.cc
    src/libfred/registrable_object/contact/verification/info_check.cc
    src/libfred/registrable_object/contact/verification/list_checks.cc
    src/libfred/registrable_object/contact/verification/list_enum_objects.cc
    src/libfred/registrable_object/contact/verification/update_check.cc
    src/libfred/registrable_object/contact/verification/update_test.cc
    src/libfred/registrable_object/domain.cc
    src/libfred/registrable_object/domain/check_domain.cc
    src/libfred/registrable_object/domain/copy_history_impl.cc
    src/libfred/registrable_object/domain/create_domain.cc
    src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc
    src/libfred/registrable_object/domain/delete_domain.cc
    src/libfred/registrable_object/domain/domain.cc
    src/libfred/registrable_object/domain/domain_name.cc
    src/libfred/registrable_object/domain/info_domain.cc
    src/libfred/registrable_object/domain/info_domain_data.cc
    src/libfred/registrable_object/domain/info_domain_diff.cc
    src/libfred/registrable_object/domain/info_domain_impl.cc
    src/libfred/registrable_object/domain/info_domain_output.cc
    src/libfred/registrable_object/domain/renew_domain.cc
    src/libfred/registrable_object/domain/transfer_domain.cc
    src/libfred/registrable_object/domain/update_domain.cc
    src/libfred/registrable_object/keyset.cc
    src/libfred/registrable_object/keyset/check_dns_key.cc
    src/libfred/registrable_object/keyset/check_keyset.cc
    src/libfred/registrable_object/keyset/copy_history_impl.cc
    src/libfred/registrable_object/keyset/create_keyset.cc
    src/libfred/registrable_object/keyset/delete_keyset.cc
    src/libfred/registrable_object/keyset/info_keyset.cc
    src/libfred/registrable_object/keyset/info_keyset_data.cc
    src/libfred/registrable_object/keyset/info_keyset_diff.cc
    src/libfred/registrable_object/keyset/info_keyset_impl.cc
    src/libfred/registrable_object/keyset/info_keyset_output.cc
    src/libfred/registrable_object/keyset/transfer_keyset.cc
    src/libfred/registrable_object/keyset/update_keyset.cc
    src/libfred/registrable_object/nsset.cc
    src/libfred/registrable_object/nsset/check_nsset.cc
    src/libfred/registrable_object/nsset/copy_history_impl.cc
    src/libfred/registrable_object/nsset/create_nsset.cc
    src/libfred/registrable_object/nsset/delete_nsset.cc
    src/libfred/registrable_object/nsset/info_nsset.cc
    src/libfred/registrable_object/nsset/info_nsset_data.cc
    src/libfred/registrable_object/nsset/info_nsset_diff.cc
    src/libfred/registrable_object/nsset/info_nsset_impl.cc
    src/libfred/registrable_object/nsset/info_nsset_output.cc
    src/libfred/registrable_object/nsset/transfer_nsset.cc
    src/libfred/registrable_object/nsset/update_nsset.cc
    src/libfred/registrar.cc
    src/libfred/registrar/epp_auth/add_registrar_epp_auth.cc
    src/libfred/registrar/epp_auth/delete_registrar_epp_auth.cc
    src/libfred/registrar/epp_auth/exceptions.cc
    src/libfred/registrar/certification/create_registrar_certification.cc
    src/libfred/registrar/certification/get_registrar_certifications.cc
    src/libfred/registrar/certification/update_registrar_certification.cc
    src/libfred/registrar/check_registrar.cc
    src/libfred/registrar/create_registrar.cc
    src/libfred/registrar/get_registrar_handles.cc
    src/libfred/registrar/get_registrar_zone_credit.cc
    src/libfred/registrar/group/cancel_registrar_group.cc
    src/libfred/registrar/group/create_registrar_group.cc
    src/libfred/registrar/group/get_registrar_groups.cc
    src/libfred/registrar/group/membership/create_registrar_group_membership.cc
    src/libfred/registrar/group/membership/end_registrar_group_membership.cc
    src/libfred/registrar/group/membership/info_group_membership_by_group.cc
    src/libfred/registrar/group/membership/info_group_membership_by_registrar.cc
    src/libfred/registrar/group/update_registrar_group.cc
    src/libfred/registrar/info_registrar.cc
    src/libfred/registrar/info_registrar_data.cc
    src/libfred/registrar/info_registrar_diff.cc
    src/libfred/registrar/info_registrar_impl.cc
    src/libfred/registrar/info_registrar_output.cc
    src/libfred/registrar/registrar_impl.cc
    src/libfred/registrar/registrar_zone_access.cc
    src/libfred/registrar/registrar_zone_credit.cc
    src/libfred/registrar/update_registrar.cc
    src/libfred/registrar/exceptions.cc
    src/libfred/registrar/zone_credit.cc
    src/libfred/registrar/zone_access/add_registrar_zone_access.cc
    src/libfred/registrar/zone_access/exceptions.cc
    src/libfred/registrar/zone_access/update_registrar_zone_access.cc
    src/libfred/registry.cc
    src/libfred/reminder.cc
    src/libfred/requests/model_request.cc
    src/libfred/requests/model_request_property_name.cc
    src/libfred/requests/model_session.cc
    src/libfred/requests/request_cache.cc
    src/libfred/requests/request_list.cc
    src/libfred/requests/request_manager.cc
    src/libfred/requests/request_property_name_cache.cc
    src/libfred/requests/session_cache.cc
    src/libfred/requests/session.cc
    src/libfred/zone.cc
    src/libfred/zone/create_zone.cc
    src/libfred/zone/exceptions.cc
    src/libfred/zone/info_zone.cc
    src/libfred/zone/info_zone_data.cc
    src/libfred/zone/zone_ns/create_zone_ns.cc
    src/libfred/zone/zone_ns/exceptions.cc
    src/libfred/zone/zone_ns/info_zone_ns.cc
    src/libfred/zone/zone_ns/update_zone_ns.cc
    src/libfred/zone/zone_ns/util.cc
    src/libfred/zone/zone_soa/create_zone_soa.cc
    src/libfred/zone/zone_soa/info_zone_soa.cc
    src/libfred/zone/zone_soa/update_zone_soa.cc
    src/libfred/zone/update_zone.cc
    src/libfred/zone/util.cc
    src/libfred/zone/zone.cc
    src/util/cfg/handle_rifd_args.cc
    src/util/corba_conversion.cc
    src/util/csv_parser.cc
    src/util/db/param_query_composition.cc
    src/util/db/query/compound_filter.cc
    src/util/db/query/filter.cc
    src/util/db/query/query_old.cc
    src/util/db/query/simple_filter.cc
    src/util/db/test-db.cc
    src/util/db/test-mockdb.cc
    src/util/db/value.cc
    src/util/dummy_logger.cc
    src/util/hp/crc32.cc
    src/util/hp/hp.cc
    src/util/hp/hpmailbatchstate.cc
    src/util/hp/hpmail.cc
    src/util/idn_utils.cc
    src/util/log/context.cc
    src/util/log/log.cc
    src/util/log/logger.cc
    src/util/log/log_types.cc
    src/util/log/test-log.cc
    src/util/optys/download_client.cc
    src/util/optys/upload_client.cc
    src/util/output_indenter.cc
    src/util/password_storage/base64.cc
    src/util/password_storage.cc
    src/util/password_storage/impl/check_equality.cc
    src/util/password_storage/impl/pbkdf2.cc
    src/util/password_storage/impl/plaintext.cc
    src/util/printable.cc
    src/util/random.cc
    src/util/subprocess.cc
    src/util/types/birthdate.cc
    src/util/types/date.cc
    src/util/types/date_interval.cc
    src/util/types/datetime.cc
    src/util/types/datetime_interval.cc
    src/util/types/id.cc
    src/util/tz/europe/prague.cc
    src/util/tz/local_timestamp.cc
    src/util/tz/utc.cc
    src/util/util.cc
    src/util/xmlgen.cc
    test/backend/accounting/main.cc
    test/backend/accounting/test_get_registrar_by_handle.cc
    test/backend/accounting/test_get_registrar_by_payment.cc
    test/backend/accounting/test_get_registrar_references.cc
    test/backend/accounting/test_get_zone_by_payment.cc
    test/backend/admin/contact/automerge/main.cc
    test/backend/admin/contact/automerge/test-merge-auto-proc.cc
    test/backend/admin/contact/verification/main.cc
    test/backend/admin/contact/verification/test_delete_domains_of_invalid_contact.cc
    test/backend/admin/contact/verification/test_enqueue_check.cc
    test/backend/admin/contact/verification/test_fill_check_queue.cc
    test/backend/admin/contact/verification/test_related_records.cc
    test/backend/admin/contact/verification/test_resolve_check.cc
    test/backend/admin/contact/verification/test_run_all_enqueued_checks.cc
    test/backend/admin/domain/main.cc
    test/backend/admin/domain/test_create_expired_domain.cc
    test/backend/admin/notification/main.cc
    test/backend/admin/notification/test_set_domain_outzone_unguarded_warning_emails.cc
    test/backend/admin/zone/main.cc
    test/backend/admin/zone/test_add_zone.cc
    test/backend/admin/zone/test_add_zone_ns.cc
    test/backend/automatic_keyset_management/main.cc
    test/backend/automatic_keyset_management/test_automatic_keyset_management.cc
    test/backend/domain_browser/test_domain_browser.cc
    test/backend/epp/contact/test_check_contact.cc
    test/backend/epp/contact/test_contact_handle_state_to_check_result.cc
    test/backend/epp/contact/test_create_contact.cc
    test/backend/epp/contact/test_delete_contact.cc
    test/backend/epp/contact/test_info_contact.cc
    test/backend/epp/contact/test_transfer_contact.cc
    test/backend/epp/contact/test_update_contact.cc
    test/backend/epp/credit/test_client_credit.cc
    test/backend/epp/domain/fixture.cc
    test/backend/epp/domain/test_check_domain.cc
    test/backend/epp/domain/test_create_domain.cc
    test/backend/epp/domain/test_delete_domain.cc
    test/backend/epp/domain/test_domain_billing.cc
    test/backend/epp/domain/test_info_domain.cc
    test/backend/epp/domain/test_renew_domain.cc
    test/backend/epp/domain/test_transfer_domain.cc
    test/backend/epp/domain/test_update_domain.cc
    test/backend/epp/keyset/fixture.cc
    test/backend/epp/keyset/test_create_keyset.cc
    test/backend/epp/main.cc
    test/backend/epp/nsset/test_check_nsset.cc
    test/backend/epp/nsset/test_create_nsset.cc
    test/backend/epp/nsset/test_delete_nsset.cc
    test/backend/epp/nsset/test_info_nsset.cc
    test/backend/epp/nsset/test_transfer_nsset.cc
    test/backend/epp/nsset/test_update_nsset.cc
    test/backend/epp/poll/fixture.cc
    test/backend/epp/poll/test_poll_acknowledgement.cc
    test/backend/epp/poll/test_poll_request.cc
    test/backend/epp/poll/test_poll_request_domain_details.cc
    test/backend/epp/poll/test_poll_request_keyset_details.cc
    test/backend/epp/poll/test_poll_request_nsset_details.cc
    test/backend/epp/test_conditionally_enqueue_notification.cc
    test/backend/public_request/fixture_common.cc
    test/backend/public_request/main.cc
    test/backend/public_request/test_create_block_unblock_request.cc
    test/backend/public_request/test_create_public_request_pdf.cc
    test/backend/public_request/test_non_registry_email.cc
    test/backend/public_request/test_registry_email.cc
    test/backend/record_statement/main.cc
    test/backend/record_statement/test_record_statement.cc
    test/backend/whois/main.cc
    test/backend/whois/test_whois_get_contact_by_handle.cc
    test/backend/whois/test_whois_get_domain_by_handle.cc
    test/backend/whois/test_whois_get_domains_by_admin_contact.cc
    test/backend/whois/test_whois_get_domains_by_keyset.cc
    test/backend/whois/test_whois_get_domains_by_nsset.cc
    test/backend/whois/test_whois_get_domains_by_registrant.cc
    test/backend/whois/test_whois_get_keyset_by_handle.cc
    test/backend/whois/test_whois_get_keysets_by_tech_c.cc
    test/backend/whois/test_whois_get_managed_zone_list.cc
    test/backend/whois/test_whois_get_nameserver_by_fqdn.cc
    test/backend/whois/test_whois_get_nsset_by_handle.cc
    test/backend/whois/test_whois_get_nssets_by_ns.cc
    test/backend/whois/test_whois_get_nssets_by_tech_c.cc
    test/backend/whois/test_whois_get_object_status_descriptions.cc
    test/backend/whois/test_whois_get_registrar_by_handle.cc
    test/backend/whois/test_whois_get_registrar_certification_list.cc
    test/backend/whois/test_whois_get_registrar_groups.cc
    test/backend/whois/test_whois_get_registrars.cc
    test/deprecated/test_admin.cc
    test/deprecated/test_cache.cc
    test/deprecated/test_common_registry.cc
    test/deprecated/test_contact_verification.cc
    test/deprecated/test_conv.cc
    test/deprecated/test_corbans.cc
    test/deprecated/test_db.cc
    test/deprecated/test_file_manager_client.cc
    test/deprecated/test_filters_date_time.cc
    test/deprecated/test_invoice2.cc
    test/deprecated/test_invoice_account.cc
    test/deprecated/test_invoice.cc
    test/deprecated/test_invoice_common.cc
    test/deprecated/test_locking_trigger.cc
    test/deprecated/test_logd.cc
    test/deprecated/test_msgd.cc
    test/deprecated/test_registrar_blocking.cc
    test/deprecated/test_registrar_group.cc
    test/deprecated/test_server.cc
    test/deprecated/test_whois.cc
    test/libfred/contact/test_copy_contact.cc
    test/libfred/contact/test_create_contact.cc
    test/libfred/contact/test_delete_contact.cc
    test/libfred/contact/test_info_contact.cc
    test/libfred/contact/test_merge_contact.cc
    test/libfred/contact/test_mojeid_checkers.cc
    test/libfred/contact/test_transfer_contact.cc
    test/libfred/contact/test_undisclose_address.cc
    test/libfred/contact/test_update_contact.cc
    test/libfred/contact/verification/setup_utils.cc
    test/libfred/contact/verification/test_create_check.cc
    test/libfred/contact/verification/test_create_test.cc
    test/libfred/contact/verification/test_django_email_format_check.cc
    test/libfred/contact/verification/test_info_check.cc
    test/libfred/contact/verification/test_list_checks.cc
    test/libfred/contact/verification/test_list_enum_objects.cc
    test/libfred/contact/verification/test_update_check.cc
    test/libfred/contact/verification/test_update_test.cc
    test/libfred/domain/test_check_domain.cc
    test/libfred/domain/test_create_domain.cc
    test/libfred/domain/test_create_domain_name_blacklist_id.cc
    test/libfred/domain/test_delete_domain.cc
    test/libfred/domain/test_domain_name.cc
    test/libfred/domain/test_info_domain.cc
    test/libfred/domain/test_renew_domain.cc
    test/libfred/domain/test_transfer_domain.cc
    test/libfred/domain/test_update_domain.cc
    test/libfred/keyset/test_delete_keyset.cc
    test/libfred/keyset/test_info_keyset.cc
    test/libfred/keyset/test_transfer_keyset.cc
    test/libfred/keyset/test_update_keyset.cc
    test/libfred/main.cc
    test/libfred/notifier/test_enqueue_notification.cc
    test/libfred/notifier/test_event_on_object_enum.cc
    test/libfred/notifier/test_gather_email_addresses_contact_create.cc
    test/libfred/notifier/test_gather_email_addresses_contact_delete.cc
    test/libfred/notifier/test_gather_email_addresses_contact_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_contact_update.cc
    test/libfred/notifier/test_gather_email_addresses_domain_create.cc
    test/libfred/notifier/test_gather_email_addresses_domain_delete.cc
    test/libfred/notifier/test_gather_email_addresses_domain_renew.cc
    test/libfred/notifier/test_gather_email_addresses_domain_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_domain_update.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_create.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_delete.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_update.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_create.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_delete.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_update.cc
    test/libfred/notifier/test_gather_email_content_contact_create.cc
    test/libfred/notifier/test_gather_email_content_contact_delete.cc
    test/libfred/notifier/test_gather_email_content_contact_transfer.cc
    test/libfred/notifier/test_gather_email_content_contact_update.cc
    test/libfred/notifier/test_gather_email_content_domain_create.cc
    test/libfred/notifier/test_gather_email_content_domain_delete.cc
    test/libfred/notifier/test_gather_email_content_domain_renew.cc
    test/libfred/notifier/test_gather_email_content_domain_transfer.cc
    test/libfred/notifier/test_gather_email_content_domain_update.cc
    test/libfred/notifier/test_gather_email_content_keyset_create.cc
    test/libfred/notifier/test_gather_email_content_keyset_delete.cc
    test/libfred/notifier/test_gather_email_content_keyset_transfer.cc
    test/libfred/notifier/test_gather_email_content_keyset_update.cc
    test/libfred/notifier/test_gather_email_content_nsset_create.cc
    test/libfred/notifier/test_gather_email_content_nsset_delete.cc
    test/libfred/notifier/test_gather_email_content_nsset_transfer.cc
    test/libfred/notifier/test_gather_email_content_nsset_update.cc
    test/libfred/notifier/test_process_one_notification_request.cc
    test/libfred/nsset/test_delete_nsset.cc
    test/libfred/nsset/test_info_nsset.cc
    test/libfred/nsset/test_transfer_nsset.cc
    test/libfred/nsset/test_update_nsset.cc
    test/libfred/object_state/test_cancel_object_state_request_id.cc
    test/libfred/object_state/test_create_admin_object_block_request_id.cc
    test/libfred/object_state/test_create_admin_object_state_restore_request_id.cc
    test/libfred/object_state/test_create_object_state_request_id.cc
    test/libfred/object_state/test_object_state.cc
    test/libfred/object/test_generate_authinfo_password.cc
    test/libfred/object/test_generated_authinfo_password.cc
    test/libfred/object/test_registry_object_type.cc
    test/libfred/object/test_transfer_object.cc
    test/libfred/poll/test_create_delete_contact_poll_message.cc
    test/libfred/poll/test_create_delete_domain_poll_message.cc
    test/libfred/poll/test_create_state_messages.cc
    test/libfred/poll/test_create_update_object_poll_message.cc
    test/libfred/poll/test_get_request_fee_info_message.cc
    test/libfred/public_request/test_create_public_request_auth.cc
    test/libfred/public_request/test_create_public_request.cc
    test/libfred/public_request/test_update_public_request.cc
    test/libfred/registrar/epp_auth/test_add_registrar_epp_auth.cc
    test/libfred/registrar/epp_auth/test_delete_registrar_epp_auth.cc
    test/libfred/registrar/certification/test_create_registrar_certification.cc
    test/libfred/registrar/certification/test_get_registrar_certifications.cc
    test/libfred/registrar/certification/test_update_registrar_certification.cc
    test/libfred/registrar/group/membership/test_create.cc
    test/libfred/registrar/group/membership/test_end.cc
    test/libfred/registrar/group/membership/test_info_by_group.cc
    test/libfred/registrar/group/membership/test_info_by_registrar.cc
    test/libfred/registrar/group/test_cancel.cc
    test/libfred/registrar/group/test_create.cc
    test/libfred/registrar/group/test_get.cc
    test/libfred/registrar/group/test_update.cc
    test/libfred/registrar/test_create_registrar.cc
    test/libfred/registrar/test_get_registrar_zone_credit.cc
    test/libfred/registrar/test_info_registrar.cc
    test/libfred/registrar/test_registrar_zone_access.cc
    test/libfred/registrar/test_update_registrar.cc
    test/libfred/registrar/zone_access/test_add_zone_access.cc
    test/libfred/registrar/zone_access/util.cc
    test/libfred/registrar/zone_access/test_update_zone_access.cc
    test/libfred/test_check_handle.cc
    test/libfred/test_opexception.cc
    test/libfred/zone/zone_ns/test_create_zone_ns.cc
    test/libfred/zone/zone_ns/test_info_zone_ns.cc
    test/libfred/zone/zone_ns/test_update_zone_ns.cc
    test/libfred/zone/zone_ns/util.cc
    test/libfred/zone/zone_soa/test_create_zone_soa.cc
    test/libfred/zone/zone_soa/test_info_zone_soa.cc
    test/libfred/zone/zone_soa/test_update_zone_soa.cc
    test/libfred/zone/zone_soa/util.cc
    test/libfred/zone/test_create_zone.cc
    test/libfred/zone/test_info_zone.cc
    test/libfred/zone/test_update_zone.cc
    test/libfred/zone/util.cc
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc
    test/setup/tests_common.cc
    test/test_delete_objects_marked_as_delete_candidate.cc
    test/test_optys_main.cc
    test/util/test_corba_conversion.cc
    test/util/test_corba_conversion_main.cc
    test/util/test_csv_parser.cc
    test/util/test_csv_serializer.cc
    test/util/test_decimal.cc
    test/util/test_exception.cc
    test/util/test_is_equal_optional_nullable.cc
    test/util/test_nullable.cc
    test/util/test_optional.cc
    test/util/test_optys.cc
    test/util/test_param_query.cc
    test/util/test_password_storage.cc
    test/util/test_password_storage_main.cc
    test/util/test_printable.cc
    test/util/test_shellcmd.cc
    tools/contact_migration/mojeid_contact_address_import.cc
    tools/contact_migration/mojeid_contact_ssn_fix.cc
    tools/disclose_flags_updater/contact_search_query.cc
    tools/disclose_flags_updater/disclose_settings.cc
    tools/disclose_flags_updater/disclose_value.cc
    tools/disclose_flags_updater/fred_disclose_flags_updater.cc
    tools/disclose_flags_updater/thread_safe_output.cc
    tools/disclose_flags_updater/worker.cc
    tools/logd_migration/log_if.cc
    tools/logd_migration/m_epp_parser.cc
    tools/logd_migration/migrate.cc)

# end of prepare_object_files

prepare_static_library(util
    src/util/random.cc
    src/util/util.cc
    src/util/output_indenter.cc
    src/util/printable.cc
    src/util/csv_parser.cc
    src/util/xmlgen.cc
    src/util/subprocess.cc
    src/util/idn_utils.cc)

prepare_static_library(log
    src/util/log/logger.cc
    src/util/log/log.cc
    src/util/log/log_types.cc
    src/util/log/context.cc)

prepare_static_library(types
    src/util/types/date.cc
    src/util/types/date_interval.cc
    src/util/types/datetime.cc
    src/util/types/datetime_interval.cc
    src/util/types/id.cc
    src/util/types/birthdate.cc)

prepare_static_library(db
    src/util/db/query/query_old.cc
    src/util/db/query/filter.cc
    src/util/db/query/compound_filter.cc
    src/util/db/query/simple_filter.cc
    src/util/db/param_query_composition.cc
    src/util/db/value.cc)

prepare_static_library(timezones
    src/util/tz/local_timestamp.cc
    src/util/tz/utc.cc
    src/util/tz/europe/prague.cc)

prepare_static_library(hp
    src/util/hp/hp.cc
    src/util/hp/hpmail.cc
    src/util/hp/hpmailbatchstate.cc)

prepare_static_library(corbautil
    src/bin/corba/util/corba_conversions_buffer.cc
    src/bin/corba/util/corba_conversions_isodate.cc
    src/bin/corba/util/corba_conversions_isodatetime.cc
    src/bin/corba/util/corba_conversions_money.cc
    src/bin/corba/util/corba_conversions_nullable_types.cc
    src/bin/corba/util/corba_conversions_nullableisodate.cc
    src/bin/corba/util/corba_conversions_nullableisodatetime.cc
    src/bin/corba/util/corba_conversions_string.cc)

prepare_static_library(oldutils
    src/deprecated/util/dbsql.cc
    src/deprecated/util/log.cc
    src/deprecated/util/pqsql.cc
    src/deprecated/util/timeclock.cc
    src/deprecated/util/util.cc)

prepare_static_library(model
    src/deprecated/model/object_registry_filter.cc
    src/deprecated/model/object_filter.cc
    src/deprecated/model/domain_filter.cc
    src/deprecated/model/contact_filter.cc
    src/deprecated/model/nsset_filter.cc
    src/deprecated/model/registrar_filter.cc
    src/deprecated/model/filter_filter.cc
    src/deprecated/model/object_state_filter.cc
    src/deprecated/model/invoice_filter.cc
    src/deprecated/model/public_request_filter.cc
    src/deprecated/model/mail_filter.cc
    src/deprecated/model/file_filter.cc
    src/deprecated/model/keyset_filter.cc
    src/deprecated/model/online_statement_filter.cc
    src/deprecated/model/log_filter.cc
    src/deprecated/model/union_filter.cc
    src/deprecated/model/zone_filter.cc
    src/deprecated/model/zone_ns_filter.cc
    src/deprecated/model/zone_soa_filter.cc
    src/deprecated/model/message_filter.cc)

prepare_static_library(fred
    src/libfred/registry.cc
    src/libfred/common_impl.cc
    src/libfred/object_impl.cc
    src/libfred/common_impl_new.cc
    src/libfred/model_registrar_acl.cc
    src/libfred/model_registrar.cc
    src/libfred/registrable_object/domain.cc
    src/libfred/registrable_object/contact.cc
    src/libfred/registrable_object/nsset.cc
    src/libfred/registrable_object/keyset.cc
    src/libfred/registrar.cc
    src/libfred/credit.cc
    src/libfred/invoicing/invoice.cc
    src/libfred/zone.cc
    src/libfred/zone/zone_ns/create_zone_ns.cc
    src/libfred/zone/zone_ns/exceptions.cc
    src/libfred/zone/zone_ns/info_zone_ns.cc
    src/libfred/zone/zone_ns/update_zone_ns.cc
    src/libfred/zone/zone_ns/util.cc
    src/libfred/blacklist.cc
    src/libfred/documents.cc
    src/libfred/info_buffer.cc
    src/libfred/mail.cc
    src/libfred/notify.cc
    src/libfred/file.cc
    src/libfred/filter.cc
    src/libfred/object_states.cc
    src/libfred/common_diff.cc
    src/libfred/reminder.cc
    src/libfred/opcontext.cc
    src/libfred/registrable_object/contact/copy_contact.cc
    src/libfred/registrable_object/domain/renew_domain.cc
    src/libfred/registrable_object/domain/update_domain.cc
    src/libfred/registrable_object/domain/info_domain_data.cc
    src/libfred/registrable_object/domain/info_domain_diff.cc
    src/libfred/registrable_object/domain/info_domain.cc
    src/libfred/registrable_object/domain/info_domain_impl.cc
    src/libfred/registrable_object/domain/info_domain_output.cc
    src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc
    src/libfred/registrable_object/domain/transfer_domain.cc
    src/libfred/registrable_object/domain/domain.cc
    src/libfred/registrable_object/nsset/update_nsset.cc
    src/libfred/registrable_object/nsset/info_nsset.cc
    src/libfred/registrable_object/nsset/info_nsset_data.cc
    src/libfred/registrable_object/nsset/info_nsset_diff.cc
    src/libfred/registrable_object/nsset/info_nsset_impl.cc
    src/libfred/registrable_object/nsset/info_nsset_output.cc
    src/libfred/registrable_object/keyset/update_keyset.cc
    src/libfred/registrable_object/keyset/create_keyset.cc
    src/libfred/registrable_object/keyset/info_keyset_data.cc
    src/libfred/registrable_object/keyset/info_keyset_diff.cc
    src/libfred/registrable_object/keyset/info_keyset.cc
    src/libfred/registrable_object/keyset/info_keyset_impl.cc
    src/libfred/registrable_object/keyset/info_keyset_output.cc
    src/libfred/registrable_object/keyset/transfer_keyset.cc
    src/libfred/registrable_object/contact/delete_contact.cc
    src/libfred/registrable_object/contact/info_contact_data.cc
    src/libfred/registrable_object/contact/info_contact_diff.cc
    src/libfred/registrable_object/contact/info_contact.cc
    src/libfred/registrable_object/contact/info_contact_impl.cc
    src/libfred/registrable_object/contact/info_contact_output.cc
    src/libfred/registrable_object/contact/place_address.cc
    src/libfred/registrable_object/contact/transfer_contact.cc
    src/libfred/registrable_object/contact/undisclose_address.cc
    src/backend/mojeid/checkers.cc
    src/libfred/object/object.cc
    src/libfred/object/object_impl.cc
    src/libfred/object/generate_authinfo_password.cc
    src/libfred/object/generated_authinfo_password.cc
    src/libfred/object/get_id_of_registered.cc
    src/libfred/object/transfer_object.cc
    src/libfred/object/object.cc
    src/libfred/poll/create_poll_message.cc
    src/libfred/poll/create_update_object_poll_message.cc
    src/libfred/poll/get_request_fee_message.cc
    src/libfred/poll/create_request_fee_info_message.cc
    src/libfred/poll/create_request_fee_info_messages.cc
    src/libfred/poll/create_low_credit_messages.cc
    src/libfred/poll/create_state_messages.cc
    src/libfred/registrable_object/contact/copy_history_impl.cc
    src/libfred/registrable_object/contact/merge_contact.cc
    src/libfred/registrable_object/contact/create_contact.cc
    src/libfred/registrable_object/contact/update_contact.cc
    src/backend/admin/contact/verification/contact_states/delete_all.cc
    src/libfred/registrable_object/nsset/create_nsset.cc
    src/libfred/registrable_object/nsset/transfer_nsset.cc
    src/libfred/registrable_object/domain/create_domain.cc
    src/libfred/registrable_object/domain/copy_history_impl.cc
    src/libfred/registrable_object/contact/merge_contact_selection.cc
    src/libfred/registrable_object/contact/find_contact_duplicates.cc
    src/libfred/registrable_object/contact/merge_contact_email_notification_data.cc
    src/libfred/registrable_object/domain/delete_domain.cc
    src/libfred/registrable_object/domain/domain_name.cc
    src/libfred/zone/create_zone.cc
    src/libfred/zone/exceptions.cc
    src/libfred/zone/info_zone_data.cc
    src/libfred/zone/info_zone.cc
    src/libfred/zone/update_zone.cc
    src/libfred/zone/util.cc
    src/libfred/zone/zone.cc
    src/libfred/zone/zone_soa/create_zone_soa.cc
    src/libfred/zone/zone_soa/info_zone_soa.cc
    src/libfred/zone/zone_soa/update_zone_soa.cc
    src/libfred/registrable_object/contact/verification/create_check.cc
    src/libfred/registrable_object/contact/verification/update_check.cc
    src/libfred/registrable_object/contact/verification/info_check.cc
    src/libfred/registrable_object/contact/verification/list_checks.cc
    src/libfred/registrable_object/contact/verification/list_enum_objects.cc
    src/libfred/registrable_object/contact/verification/create_test.cc
    src/libfred/registrable_object/contact/verification/update_test.cc
    src/libfred/registrable_object/domain/check_domain.cc
    src/libfred/registrable_object/contact/check_contact.cc
    src/libfred/registrable_object/nsset/check_nsset.cc
    src/libfred/registrable_object/nsset/delete_nsset.cc
    src/libfred/registrable_object/nsset/copy_history_impl.cc
    src/libfred/registrable_object/keyset/check_keyset.cc
    src/libfred/registrable_object/keyset/check_dns_key.cc
    src/libfred/registrable_object/keyset/copy_history_impl.cc
    src/libfred/object/check_handle.cc
    src/libfred/object/transfer_object.cc
    src/libfred/registrable_object/keyset/delete_keyset.cc
    src/libfred/registrar/registrar_impl.cc
    src/libfred/registrar/info_registrar.cc
    src/libfred/registrar/info_registrar_data.cc
    src/libfred/registrar/info_registrar_diff.cc
    src/libfred/registrar/registrar_zone_access.cc
    src/libfred/registrar/create_registrar.cc
    src/libfred/registrar/update_registrar.cc
    src/libfred/registrar/exceptions.cc
    src/libfred/registrar/info_registrar_impl.cc
    src/libfred/registrar/info_registrar_output.cc
    src/libfred/registrar/get_registrar_handles.cc
    src/libfred/registrar/check_registrar.cc
    src/libfred/registrar/zone_credit.cc
    src/libfred/registrar/registrar_zone_credit.cc
    src/libfred/registrar/get_registrar_zone_credit.cc
    src/libfred/registrar/group/membership/create_registrar_group_membership.cc
    src/libfred/registrar/group/membership/end_registrar_group_membership.cc
    src/libfred/registrar/group/membership/info_group_membership_by_group.cc
    src/libfred/registrar/group/membership/info_group_membership_by_registrar.cc
    src/libfred/object/object_id_handle_pair.cc
    src/libfred/object_state/object_has_state.cc
    src/libfred/object_state/lock_object_state_request_lock.cc
    src/libfred/object_state/get_object_state_descriptions.cc
    src/libfred/object_state/get_object_states.cc
    src/libfred/object_state/get_object_states_by_history_id.cc
    src/libfred/object_state/get_object_state_id_map.cc
    src/libfred/object_state/get_blocking_status_desc_list.cc
    src/libfred/object_state/perform_object_state_request.cc
    src/libfred/object_state/cancel_object_state_request_id.cc
    src/libfred/object_state/create_object_state_request_id.cc
    src/libfred/object_state/create_admin_object_block_request_id.cc
    src/libfred/object_state/create_admin_object_state_restore_request_id.cc
    src/libfred/object_state/clear_admin_object_state_request_id.cc
    src/libfred/object_state/clear_object_state_request_id.cc
    src/libfred/object/object_id_handle_pair.cc
    src/libfred/notifier/enqueue_notification.cc
    src/libfred/notifier/gather_email_data/gather_email_addresses.cc
    src/libfred/notifier/gather_email_data/gather_email_content.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/contact.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/domain.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.cc
    src/libfred/notifier/gather_email_data/objecttype_specific_impl/nsset.cc
    src/libfred/notifier/process_one_notification_request.cc
    src/libfred/registrar/group/create_registrar_group.cc
    src/libfred/registrar/group/update_registrar_group.cc
    src/libfred/registrar/group/cancel_registrar_group.cc
    src/libfred/registrar/group/get_registrar_groups.cc
    src/libfred/registrar/epp_auth/add_registrar_epp_auth.cc
    src/libfred/registrar/epp_auth/delete_registrar_epp_auth.cc
    src/libfred/registrar/epp_auth/exceptions.cc
    src/libfred/registrar/certification/create_registrar_certification.cc
    src/libfred/registrar/certification/update_registrar_certification.cc
    src/libfred/registrar/certification/get_registrar_certifications.cc
    src/libfred/registrar/zone_access/add_registrar_zone_access.cc
    src/libfred/registrar/zone_access/exceptions.cc
    src/libfred/registrar/zone_access/update_registrar_zone_access.cc)

prepare_static_library(password_storage
    src/util/password_storage.cc
    src/util/password_storage/base64.cc
    src/util/password_storage/impl/pbkdf2.cc
    src/util/password_storage/impl/plaintext.cc
    src/util/password_storage/impl/check_equality.cc)

prepare_static_library(authentic_registrar
    src/backend/epp/impl/registraracl/authentic_registrar.cc)

prepare_static_library(public_request
    src/backend/buffer.cc
    src/backend/public_request/create_authinfo_request_non_registry_email.cc
    src/backend/public_request/create_authinfo_request_registry_email.cc
    src/backend/public_request/create_block_unblock_request.cc
    src/backend/public_request/create_personal_info_request_non_registry_email.cc
    src/backend/public_request/create_personal_info_request_registry_email.cc
    src/backend/public_request/create_public_request_pdf.cc
    src/backend/public_request/exceptions.cc
    src/backend/public_request/get_default_document_manager.cc
    src/backend/public_request/get_id_of_registered_object.cc
    src/backend/public_request/get_valid_registry_emails_of_registered_object.cc
    src/backend/public_request/process/authinfo.cc
    src/backend/public_request/process/block_unblock.cc
    src/backend/public_request/process/exceptions.cc
    src/backend/public_request/process/personal_info.cc
    src/backend/public_request/type/impl/public_request_authinfo.cc
    src/backend/public_request/type/impl/public_request_block_unblock.cc
    src/backend/public_request/type/impl/public_request_personal_info.cc
    src/backend/public_request/util/send_joined_address_email.cc
    src/libfred/public_request/create_public_request.cc
    src/libfred/public_request/public_request.cc
    src/libfred/public_request/public_request_authinfo_impl.cc
    src/libfred/public_request/public_request_block_impl.cc
    src/libfred/public_request/public_request_impl.cc
    src/libfred/public_request/public_request_personal_info_impl.cc
    src/libfred/public_request/create_public_request_auth.cc
    src/libfred/public_request/public_request_lock_guard.cc
    src/libfred/public_request/public_request_object_lock_guard.cc
    src/libfred/public_request/update_public_request.cc
    src/libfred/public_request/info_public_request.cc
    src/libfred/public_request/info_public_request_auth.cc
    src/libfred/public_request/get_opened_public_request.cc)

prepare_static_library(banking
    src/libfred/banking/bank_common.cc
    src/libfred/banking/bank_manager.cc
    src/libfred/banking/exceptions.cc
    src/libfred/banking/get_bank_accounts.cc)

prepare_static_library(messages
    src/libfred/messages/messages_impl.cc)

prepare_static_library(requests
    src/libfred/requests/model_request.cc
    src/libfred/requests/model_session.cc
    src/libfred/requests/model_request_property_name.cc
    src/libfred/requests/session.cc
    src/libfred/requests/request_list.cc
    src/libfred/requests/request_manager.cc
    src/libfred/requests/request_cache.cc
    src/libfred/requests/request_property_name_cache.cc
    src/libfred/requests/session_cache.cc)

prepare_static_library(contact_verification
    src/backend/contact_verification/public_request_contact_verification_impl.cc
    src/libfred/contact_verification/contact.cc
    src/libfred/contact_verification/contact_verification_checkers.cc
    src/libfred/contact_verification/contact_conditional_identification_impl.cc
    src/libfred/contact_verification/contact_identification_impl.cc
    src/libfred/contact_verification/contact_verification_state.cc
    src/libfred/contact_verification/contact_verification_validators.cc
    src/libfred/contact_verification/contact_verification_password.cc)

prepare_static_library(admin_contact_verification
    src/backend/admin/contact/verification/create_test_impl_prototypes.cc
    src/backend/admin/contact/verification/delete_domains_of_invalid_contact.cc
    src/backend/admin/contact/verification/enqueue_check.cc
    src/backend/admin/contact/verification/fill_check_queue.cc
    src/backend/admin/contact/verification/list_active_checks.cc
    src/backend/admin/contact/verification/related_records.cc
    src/backend/admin/contact/verification/resolve_check.cc
    src/backend/admin/contact/verification/run_all_enqueued_checks.cc
    src/backend/admin/contact/verification/update_tests.cc
    src/backend/admin/contact/verification/contact_states/delete_all.cc
    src/backend/admin/contact/verification/test_impl/test_contactability.cc
    src/backend/admin/contact/verification/test_impl/test_cz_address_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_interface.cc
    src/backend/admin/contact/verification/test_impl/test_name_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_phone_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_send_letter.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.cc)

prepare_static_library(admin_notification
    src/backend/admin/notification/notification.cc)

prepare_static_library(corba
    src/bin/corba/nameservice.cc
    src/bin/corba/mailer_manager.cc
    src/bin/corba/file_manager_client.cc
    src/bin/corba/logger_client_impl.cc
    src/bin/corba/epp_corba_client_impl.cc)

prepare_static_library(corbamessages
    src/bin/corba/MessagesSK.cc
    src/bin/corba/MessagesDynSK.cc)

prepare_static_library(corbamojeid
    src/bin/corba/BufferSK.cc
    src/bin/corba/BufferDynSK.cc
    src/bin/corba/NullableIsoDateSK.cc
    src/bin/corba/NullableIsoDateDynSK.cc
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/NullableIsoDateTimeSK.cc
    src/bin/corba/NullableIsoDateTimeDynSK.cc
    src/bin/corba/MojeIDSK.cc
    src/bin/corba/MojeIDDynSK.cc)

prepare_static_library(filemanager
    src/bin/corba/FileManagerSK.cc
    src/bin/corba/FileManagerDynSK.cc)

prepare_static_library(mailer
    src/bin/corba/MailerSK.cc
    src/bin/corba/MailerDynSK.cc)

prepare_static_library(corbazone
    src/bin/corba/ZoneSK.cc
    src/bin/corba/ZoneDynSK.cc)

prepare_static_library(corbadatatypes
    src/bin/corba/_dataTypesSK.cc
    src/bin/corba/_dataTypesDynSK.cc
    src/bin/corba/DateTimeSK.cc
    src/bin/corba/DateTimeDynSK.cc)

prepare_static_library(corbanullabletypes
    src/bin/corba/NullableTypesSK.cc
    src/bin/corba/NullableTypesDynSK.cc)

prepare_static_library(corbafilters
    src/bin/corba/FiltersSK.cc
    src/bin/corba/FiltersDynSK.cc)

prepare_static_library(corbaregistry
    src/bin/corba/RegistrySK.cc
    src/bin/corba/RegistryDynSK.cc)

prepare_static_library(corbawhois
    src/bin/corba/WhoisSK.cc
    src/bin/corba/WhoisDynSK.cc)

prepare_static_library(corbawhois2
    src/bin/corba/Whois2SK.cc
    src/bin/corba/Whois2DynSK.cc)

prepare_static_library(corbaadmin
    src/bin/corba/AdminSK.cc
    src/bin/corba/AdminDynSK.cc)

prepare_static_library(corbalogger
    src/bin/corba/LoggerSK.cc
    src/bin/corba/LoggerDynSK.cc)

prepare_static_library(corbatechcheck
    src/bin/corba/TechCheckSK.cc
    src/bin/corba/TechCheckDynSK.cc)

prepare_static_library(corbaepp
    src/bin/corba/EPPSK.cc
    src/bin/corba/EPPDynSK.cc)

prepare_static_library(corbazonegenerator
    src/bin/corba/ZoneGeneratorSK.cc
    src/bin/corba/ZoneGeneratorDynSK.cc)

prepare_static_library(corbacontactverification
    src/bin/corba/ContactVerificationSK.cc
    src/bin/corba/ContactVerificationDynSK.cc)

prepare_static_library(corbaadmincontactverification
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/NullableIsoDateTimeSK.cc
    src/bin/corba/NullableIsoDateTimeDynSK.cc
    src/bin/corba/AdminContactVerificationSK.cc
    src/bin/corba/AdminContactVerificationDynSK.cc)

prepare_static_library(corbaadministrativeblocking
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateDynSK.cc
    src/bin/corba/NullableIsoDateSK.cc
    src/bin/corba/NullableIsoDateDynSK.cc
    src/bin/corba/AdministrativeBlockingSK.cc
    src/bin/corba/AdministrativeBlockingDynSK.cc)

prepare_static_library(corbadomainbrowser
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateDynSK.cc
    src/bin/corba/NullableIsoDateSK.cc
    src/bin/corba/NullableIsoDateDynSK.cc
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/NullableIsoDateTimeSK.cc
    src/bin/corba/NullableIsoDateTimeDynSK.cc
    src/bin/corba/DomainBrowserSK.cc
    src/bin/corba/DomainBrowserDynSK.cc)

prepare_static_library(corbapublicrequest
    src/bin/corba/BufferSK.cc
    src/bin/corba/BufferDynSK.cc
    src/bin/corba/PublicRequestSK.cc
    src/bin/corba/PublicRequestDynSK.cc)

prepare_static_library(corba_conversion_test
    src/bin/corba/corba_conversion_testSK.cc
    src/bin/corba/corba_conversion_testDynSK.cc)

prepare_static_library(corbanotification
    src/bin/corba/NotificationSK.cc
    src/bin/corba/NotificationDynSK.cc)

prepare_static_library(corbaautomatickeysetmanagement
    src/bin/corba/AutomaticKeysetManagementSK.cc
    src/bin/corba/AutomaticKeysetManagementDynSK.cc)

prepare_static_library(corbarecordstatement
    src/bin/corba/BufferSK.cc
    src/bin/corba/BufferDynSK.cc
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/RecordStatementSK.cc
    src/bin/corba/RecordStatementDynSK.cc)

prepare_static_library(admin_impl
    src/bin/corba/admin/admin_impl.cc
    src/bin/corba/admin/session_impl.cc
    src/bin/corba/admin/user_impl.cc
    src/bin/corba/admin/filter_impl.cc
    src/bin/corba/admin/pagetable_impl.cc
    src/bin/corba/admin/pagetable_publicrequests.cc
    src/bin/corba/admin/pagetable_contacts.cc
    src/bin/corba/admin/pagetable_domains.cc
    src/bin/corba/admin/pagetable_filters.cc
    src/bin/corba/admin/pagetable_invoices.cc
    src/bin/corba/admin/pagetable_mails.cc
    src/bin/corba/admin/pagetable_nssets.cc
    src/bin/corba/admin/pagetable_registrars.cc
    src/bin/corba/admin/pagetable_files.cc
    src/bin/corba/admin/pagetable_keysets.cc
    src/bin/corba/admin/pagetable_logger.cc
    src/bin/corba/admin/pagetable_logsession.cc
    src/bin/corba/admin/pagetable_zones.cc
    src/bin/corba/admin/pagetable_messages.cc
    src/bin/corba/admin/common.cc
    src/bin/corba/admin/bankinginvoicing_impl.cc)

prepare_static_library(whois_impl
    src/bin/corba/whois/whois_impl.cc)

prepare_static_library(whois2_impl
    src/bin/corba/whois/whois2_impl.cc
    src/backend/whois/nameserver_exists.cc
    src/backend/whois/is_domain_delete_pending.cc
    src/backend/whois/domain_expiration_datetime.cc
    src/backend/whois/registrar_group.cc
    src/backend/whois/registrar_certification.cc
    src/backend/whois/zone_list.cc
    src/backend/whois/whois.cc)

prepare_static_library(epp_impl
    src/libfred/contact_verification/cancel_contact_verification.cc
    src/libfred/contact_verification/contact_verification_state.cc
    src/libfred/registrable_object/domain/domain.cc
    src/bin/corba/epp/epp_session.cc
    src/bin/corba/epp/epp_impl.cc
    src/bin/corba/epp/contact/contact_corba_conversions.cc
    src/bin/corba/epp/credit/credit_corba_conversions.cc
    src/bin/corba/epp/domain/domain_corba_conversions.cc
    src/bin/corba/epp/poll/poll_corba_conversions.cc
    src/bin/corba/epp/keyset/keyset_corba_conversions.cc
    src/bin/corba/epp/nsset/nsset_corba_conversions.cc
    src/bin/corba/epp/corba_conversions.cc
    src/bin/corba/epp/messages.cc
    src/bin/corba/epp/tech_check.cc
    src/bin/corba/util/corba_conversions_string.cc
    src/backend/epp/localization.cc
    src/backend/epp/epp_response_failure_localized.cc
    src/backend/epp/epp_response_success_localized.cc
    src/backend/epp/epp_result_failure.cc
    src/backend/epp/epp_result_failure_localized.cc
    src/backend/epp/epp_result_success_localized.cc
    src/backend/epp/epp_extended_error.cc
    src/backend/epp/epp_extended_error_localized.cc
    src/backend/epp/session_data.cc
    src/backend/epp/impl/conditionally_enqueue_notification.cc
    src/backend/epp/contact/authinfo_contact.cc
    src/backend/epp/contact/authinfo_contact_localized.cc
    src/backend/epp/contact/create_contact_localized.cc
    src/backend/epp/contact/create_contact.cc
    src/backend/epp/contact/util.cc
    src/backend/epp/contact/contact_change.cc
    src/backend/epp/contact/contact_data.cc
    src/backend/epp/contact/impl/contact_output.cc
    src/backend/epp/contact/impl/get_personal_id_union.cc
    src/backend/epp/contact/impl/cznic/create_contact_data_filter.cc
    src/backend/epp/contact/impl/cznic/update_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/create_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/update_contact_data_filter.cc
    src/backend/epp/contact/impl/get_create_contact_data_filter.cc
    src/backend/epp/contact/impl/get_update_contact_data_filter.cc
    src/backend/epp/contact/update_contact_localized.cc
    src/backend/epp/contact/update_contact.cc
    src/backend/epp/contact/update_contact_config_data.cc
    src/backend/epp/contact/info_contact_localized.cc
    src/backend/epp/contact/info_contact.cc
    src/backend/epp/contact/delete_contact_localized.cc
    src/backend/epp/contact/delete_contact.cc
    src/backend/epp/contact/check_contact_localized.cc
    src/backend/epp/contact/check_contact.cc
    src/backend/epp/contact/transfer_contact_localized.cc
    src/backend/epp/contact/transfer_contact.cc
    src/backend/epp/contact/update_contact_post_hooks.cc
    src/backend/epp/contact/create_contact_input_data.cc
    src/backend/epp/contact/create_contact_config_data.cc
    src/backend/epp/credit/client_credit_output_data.cc
    src/backend/epp/credit/client_credit.cc
    src/backend/epp/credit/client_credit_localized.cc
    src/backend/epp/domain/authinfo_domain.cc
    src/backend/epp/domain/authinfo_domain_localized.cc
    src/backend/epp/domain/check_domain_localized.cc
    src/backend/epp/domain/check_domain.cc
    src/backend/epp/poll/poll_acknowledgement_localized.cc
    src/backend/epp/poll/poll_acknowledgement.cc
    src/backend/epp/poll/poll_request_localized.cc
    src/backend/epp/poll/poll_request.cc
    src/backend/epp/poll/poll_request_get_update_contact_details.cc
    src/backend/epp/poll/poll_request_get_update_domain_details.cc
    src/backend/epp/poll/poll_request_get_update_keyset_details.cc
    src/backend/epp/poll/poll_request_get_update_nsset_details.cc
    src/backend/epp/poll/poll_request_get_update_contact_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_domain_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_keyset_details_localized.cc
    src/backend/epp/poll/poll_request_get_update_nsset_details_localized.cc
    src/backend/epp/domain/delete_domain_localized.cc
    src/backend/epp/domain/delete_domain.cc
    src/backend/epp/domain/info_domain_localized.cc
    src/backend/epp/domain/info_domain.cc
    src/backend/epp/domain/transfer_domain_localized.cc
    src/backend/epp/domain/transfer_domain.cc
    src/backend/epp/domain/create_domain_localized.cc
    src/backend/epp/domain/domain_enum_validation.cc
    src/backend/epp/domain/create_domain.cc
    src/backend/epp/domain/renew_domain.cc
    src/backend/epp/domain/renew_domain_localized.cc
    src/backend/epp/domain/impl/domain_billing.cc
    src/backend/epp/domain/impl/domain_output.cc
    src/backend/epp/domain/update_domain_localized.cc
    src/backend/epp/domain/update_domain.cc
    src/backend/epp/keyset/authinfo_keyset.cc
    src/backend/epp/keyset/authinfo_keyset_localized.cc
    src/backend/epp/keyset/dns_key.cc
    src/backend/epp/keyset/info_keyset.cc
    src/backend/epp/keyset/info_keyset_localized.cc
    src/backend/epp/keyset/create_keyset.cc
    src/backend/epp/keyset/create_keyset_localized.cc
    src/backend/epp/keyset/update_keyset.cc
    src/backend/epp/keyset/update_keyset_localized.cc
    src/backend/epp/keyset/impl/keyset_output.cc
    src/backend/epp/keyset/check_keyset.cc
    src/backend/epp/keyset/check_keyset_localized.cc
    src/backend/epp/keyset/delete_keyset.cc
    src/backend/epp/keyset/delete_keyset_localized.cc
    src/backend/epp/keyset/transfer_keyset.cc
    src/backend/epp/keyset/transfer_keyset_localized.cc
    src/backend/epp/nsset/authinfo_nsset.cc
    src/backend/epp/nsset/authinfo_nsset_localized.cc
    src/backend/epp/nsset/check_nsset_localized.cc
    src/backend/epp/nsset/check_nsset.cc
    src/backend/epp/nsset/info_nsset_localized.cc
    src/backend/epp/nsset/info_nsset.cc
    src/backend/epp/nsset/delete_nsset_localized.cc
    src/backend/epp/nsset/delete_nsset.cc
    src/backend/epp/nsset/create_nsset_localized.cc
    src/backend/epp/nsset/create_nsset.cc
    src/backend/epp/nsset/update_nsset_localized.cc
    src/backend/epp/nsset/update_nsset.cc
    src/backend/epp/nsset/transfer_nsset_localized.cc
    src/backend/epp/nsset/transfer_nsset.cc
    src/backend/epp/nsset/impl/nsset.cc
    src/backend/epp/nsset/impl/nsset_output.cc
    src/backend/epp/public_request/create_authinfo_request_registry_email.cc
    src/backend/buffer.cc)

prepare_static_library(log_impl
    src/bin/corba/log/log_impl_wrap.cc)

prepare_static_library(admin_create_expired_domain
    src/backend/admin/domain/create_expired_domain.cc)

prepare_static_library(corbabuffer
    src/bin/corba/BufferSK.cc
    src/bin/corba/BufferDynSK.cc)

prepare_static_library(corbaisodate
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateDynSK.cc)

prepare_static_library(corbanullableisodate
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateDynSK.cc
    src/bin/corba/NullableIsoDateSK.cc
    src/bin/corba/NullableIsoDateDynSK.cc)

prepare_static_library(corbaisodatetime
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc)

prepare_static_library(corbanullableisodatetime
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/NullableIsoDateTimeSK.cc
    src/bin/corba/NullableIsoDateTimeDynSK.cc)

prepare_static_library(handle_rifd_args
    src/util/cfg/handle_rifd_args.cc
    src/backend/epp/contact/config_data_filter.cc
    src/backend/epp/contact/impl/cznic/specific.cc
    src/backend/epp/contact/impl/set_unused/config.cc)

prepare_static_library(corbaaccounting
    src/bin/corba/IsoDateSK.cc
    src/bin/corba/IsoDateDynSK.cc
    src/bin/corba/IsoDateTimeSK.cc
    src/bin/corba/IsoDateTimeDynSK.cc
    src/bin/corba/AccountingSK.cc
    src/bin/corba/AccountingDynSK.cc)


# fred-rifd
prepare_executable(fred-rifd
    src/bin/corba/fred-rifd.cc
    src/util/corba_conversion.cc)
target_link_libraries(fred-rifd
    epp_impl
    password_storage
    authentic_registrar
    password_storage
    admin_contact_verification
    corbaepp
    corbatechcheck
    mailer
    corbadatatypes
    corbaisodatetime
    corba
    public_request
    fred
    messages
    model
    timezones
    db
    types
    log
    handle_rifd_args
    oldutils
    util)
target_link_external_libraries(fred-rifd
    boost_thread
    boost_random
    boost_date_time
    boost_regex
    boost_serialization
    boost_program_options
    boost_system
    libidn
    omniorb4
    omnidynamic4
    postgres
    threads
    openssl
    mpdec)

# fred-adifd
prepare_executable(fred-adifd
    src/bin/corba/admin_contact_verification/server_i.cc
    src/libfred/registrable_object/contact/util.cc
    src/bin/corba/fred-adifd.cc
    src/backend/mojeid/mojeid_public_request.cc
    src/bin/corba/admin/public_request_mojeid.cc
    src/backend/admin_block/administrativeblocking.cc
    src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc
    src/libfred/registrable_object/domain/update_domain.cc
    src/libfred/registrable_object/contact/copy_contact.cc
    src/libfred/registrable_object/contact/info_contact_data.cc
    src/libfred/registrable_object/contact/info_contact.cc
    src/libfred/registrable_object/contact/create_contact.cc
    src/libfred/registrable_object/contact/place_address.cc
    src/backend/mojeid/checkers.cc
    src/libfred/opcontext.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/mailer_manager.cc
    src/bin/corba/admin_block/server_i.cc
    src/backend/admin/contact/verification/create_test_impl_prototypes.cc
    src/backend/admin/contact/verification/delete_domains_of_invalid_contact.cc
    src/backend/admin/contact/verification/enqueue_check.cc
    src/backend/admin/contact/verification/fill_check_queue.cc
    src/backend/admin/contact/verification/list_active_checks.cc
    src/backend/admin/contact/verification/related_records.cc
    src/backend/admin/contact/verification/resolve_check.cc
    src/backend/admin/contact/verification/run_all_enqueued_checks.cc
    src/backend/admin/contact/verification/update_tests.cc
    src/backend/admin/contact/verification/contact_states/delete_all.cc
    src/backend/admin/contact/verification/test_impl/test_contactability.cc
    src/backend/admin/contact/verification/test_impl/test_cz_address_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists.cc
    src/backend/admin/contact/verification/test_impl/test_email_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_interface.cc
    src/backend/admin/contact/verification/test_impl/test_name_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_phone_syntax.cc
    src/backend/admin/contact/verification/test_impl/test_send_letter.cc
    src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.cc
    src/bin/corba/notification/server_i.cc
    src/bin/corba/notification/notification_corba_conversion.cc
    src/backend/admin/notification/notification.cc
    src/util/corba_conversion.cc)
target_link_libraries(fred-adifd
    admin_create_expired_domain
    admin_impl
    authentic_registrar
    banking
    contact_verification
    corba
    corbaadmincontactverification
    corbaadministrativeblocking
    corbaisodatetime
    corbacontactverification
    corbadatatypes
    corbaepp
    corbafilters
    corbalogger
    corbanotification
    corbanullabletypes
    corbapublicrequest
    corbaregistry
    corbautil
    corbawhois
    corbawhois2
    corbazone
    corbaadmin
    db
    timezones
    filemanager
    fred
    log
    mailer
    messages
    model
    oldutils
    password_storage
    public_request
    requests
    types
    util
    whois2_impl
    whois_impl)
target_link_external_libraries(fred-adifd
    boost_random
    boost_system
    boost_regex
    boost_date_time
    boost_thread
    boost_program_options
    boost_serialization
    libidn
    postgres
    omniorb4
    omnidynamic4
    libxml2
    openssl
    threads
    resolv
    mpdec)

# fred-pifd
prepare_executable(fred-pifd
    src/bin/corba/contact_verification/contact_verification_i.cc
    src/backend/mojeid/mojeid_public_request.cc
    src/backend/contact_verification/contact_verification_impl.cc
    src/backend/buffer.cc
    src/bin/corba/public_request/server_i.cc
    src/util/corba_conversion.cc
    src/bin/corba/admin/common.cc
    src/bin/corba/util/corba_conversions_buffer.cc
    src/bin/corba/fred-pifd.cc)
target_link_libraries(fred-pifd
    whois_impl
    whois2_impl
    corbawhois
    corbawhois2
    corbaregistry
    corbafilters
    corbadatatypes
    corba
    corbaepp
    mailer
    filemanager
    corbalogger
    corbautil
    timezones
    corbanullabletypes
    corbaisodate
    corbaisodatetime
    corbanullableisodate
    corbanullableisodatetime
    contact_verification
    corbacontactverification
    corbabuffer
    corbapublicrequest
    requests
    banking
    messages
    public_request
    fred
    authentic_registrar
    password_storage
    model
    db
    types
    oldutils
    log
    util)
target_link_external_libraries(fred-pifd
    boost_random
    boost_thread
    boost_regex
    boost_date_time
    boost_serialization
    boost_program_options
    boost_system
    threads
    postgres
    openssl
    libidn
    omniorb4
    omnidynamic4
    mpdec)

# fred-logd
prepare_executable(fred-logd
    src/bin/corba/fred-logd.cc
    src/bin/corba/admin/common.cc
    src/bin/corba/admin/filter_impl.cc
    src/bin/corba/admin/pagetable_impl.cc
    src/bin/corba/admin/pagetable_logger.cc)
target_link_libraries(fred-logd
    log_impl
    corbalogger
    corbaregistry
    corbafilters
    corbadatatypes
    corba
    fred
    requests
    model
    db
    types
    log
    util
    oldutils)
target_link_external_libraries(fred-logd
    boost_system
    boost_regex
    boost_date_time
    boost_serialization
    boost_thread
    boost_program_options
    omniorb4
    omnidynamic4
    postgres
    mpdec)

# fred-msgd
prepare_executable(fred-msgd
    src/bin/corba/messages/fred-msgd.cc
    src/bin/corba/messages/messages_filemanager.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/messages/messages.cc
    src/bin/corba/file_manager_client.cc
    src/bin/corba/MessagesSK.cc
    src/bin/corba/FileManagerSK.cc)
target_link_libraries(fred-msgd
    messages
    model
    log
    types
    db
    oldutils
    util
    corbadatatypes)
target_link_external_libraries(fred-msgd
    boost_thread
    boost_system
    boost_regex
    boost_date_time
    boost_program_options
    postgres
    omniorb4)

# fred-mifd
prepare_executable(fred-mifd
    src/backend/buffer.cc
    src/backend/mojeid/mojeid.cc
    src/backend/mojeid/mojeid_impl_data_conversion.cc
    src/backend/mojeid/mojeid_impl_internal.cc
    src/backend/mojeid/mojeid_public_request.cc
    src/backend/mojeid/messages/generate.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/mojeid/server_i.cc
    src/util/corba_conversion.cc
    src/bin/corba/mojeid/mojeid_corba_conversion.cc
    src/bin/corba/mojeid/service_name.cc
    src/bin/corba/mojeid/fred-mifd.cc
    src/bin/corba/MojeIDSK.cc
    src/bin/corba/MailerSK.cc)
target_link_libraries(fred-mifd
    corbadatatypes
    fred
    messages
    model
    corbabuffer
    corbaisodate
    corbaisodatetime
    corbanullableisodate
    corbanullableisodatetime
    corbautil
    contact_verification
    public_request
    timezones
    util
    log
    types
    db
    corba)
target_link_external_libraries(fred-mifd
    boost_random
    boost_system
    boost_regex
    boost_thread
    boost_date_time
    boost_program_options
    omniorb4
    omnidynamic4
    postgres
    threads
    libidn)

# fred-mojeid-msgen
prepare_executable(fred-mojeid-msgen
    src/bin/corba/mojeid/service_name.cc
    src/bin/corba/mojeid/messages/client.cc
    src/bin/corba/MojeIDSK.cc)
target_link_libraries(fred-mojeid-msgen
    corbabuffer
    corbanullableisodate
    corbaisodatetime)
target_link_external_libraries(fred-mojeid-msgen
    boost_program_options
    boost_date_time
    omniorb4
    omnidynamic4)

# fred-dbifd
prepare_executable(fred-dbifd
    src/backend/domain_browser/domain_browser.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/domain_browser/domain_browser_i.cc
    src/bin/corba/domain_browser/fred-dbifd.cc)
target_link_libraries(fred-dbifd
    corbadomainbrowser
    fred
    corbautil
    timezones
    util
    log
    db)
target_link_external_libraries(fred-dbifd
    boost_system
    boost_random
    boost_regex
    boost_thread
    boost_date_time
    boost_program_options
    omniorb4
    omnidynamic4
    postgres
    threads)

# fred-akmd
prepare_executable(fred-akmd
    src/backend/epp/keyset/dns_key.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/logger_client_impl.cc
    src/util/dummy_logger.cc
    src/backend/automatic_keyset_management/automatic_keyset_management.cc
    src/backend/automatic_keyset_management/dns_key.cc
    src/backend/automatic_keyset_management/impl/logger_request_data.cc
    src/backend/automatic_keyset_management/impl/logger_request_object_type.cc
    src/backend/automatic_keyset_management/impl/logger_request_property.cc
    src/backend/automatic_keyset_management/impl/logger_request_type.cc
    src/backend/automatic_keyset_management/impl/util.cc
    src/bin/corba/automatic_keyset_management/server_i.cc
    src/bin/corba/automatic_keyset_management/fred-akmd.cc)
target_link_libraries(fred-akmd
    corbaautomatickeysetmanagement
    fred
    corbautil
    timezones
    corba
    corbalogger
    corbadatatypes
    corbaadmin
    corbaregistry
    corbafilters
    util
    log
    db)
target_link_external_libraries(fred-akmd
    omnidynamic4
    omniorb4
    boost_system
    boost_random
    boost_thread
    boost_regex
    boost_date_time
    boost_program_options
    postgres)

# fred-rsifd
prepare_executable(fred-rsifd
    src/backend/buffer.cc
    src/backend/record_statement/record_statement.cc
    src/backend/record_statement/exceptions.cc
    src/backend/record_statement/impl/record_statement_xml.cc
    src/backend/record_statement/impl/factory.cc
    src/backend/record_statement/impl/utc_impl.cc
    src/backend/record_statement/impl/europe_prague_impl.cc
    src/backend/record_statement/impl/util.cc
    src/bin/corba/nameservice.cc
    src/bin/corba/record_statement/record_statement_i.cc
    src/bin/corba/record_statement/fred-rsifd.cc)
target_link_libraries(fred-rsifd
    fred
    corbautil
    util
    log
    db
    timezones
    corba
    mailer
    corbadatatypes
    corbarecordstatement
    corbabuffer
    corbaisodate
    corbaisodatetime
    corbanullableisodate)
target_link_external_libraries(fred-rsifd
    omniorb4
    threads
    postgres
    boost_date_time
    boost_program_options
    boost_thread
    boost_system
    boost_regex)

# fred-accifd
prepare_executable(fred-accifd
    src/backend/accounting/accounting.cc
    src/backend/accounting/exceptions.cc
    src/backend/accounting/impl/accounting.cc
    src/backend/accounting/impl/exceptions.cc
    src/backend/credit.cc
    src/bin/corba/accounting/accounting_i.cc
    src/bin/corba/accounting/fred-accifd.cc
    src/bin/corba/accounting/impl/corba_conversions.cc
    src/bin/corba/accounting/impl/exceptions.cc
    src/bin/corba/nameservice.cc)
target_link_libraries(fred-accifd
    banking
    corba
    corbaaccounting
    corbautil
    timezones
    fred
    log
    model
    db
    oldutils
    password_storage
    types
    util)
target_link_external_libraries(fred-accifd
    boost_random
    boost_system
    boost_regex
    boost_date_time
    boost_thread
    boost_program_options
    libidn
    postgres
    omniorb4
    openssl
    threads
    mpdec)

# fred-admin
prepare_executable(fred-admin
    src/backend/admin/contact/merge_contact.cc
    src/backend/admin/contact/merge_contact_auto_procedure.cc
    src/backend/admin/contact/merge_contact_logger.cc
    src/backend/admin/contact/merge_contact_reporting.cc
    src/backend/admin/domain/create_expired_domain.cc
    src/backend/admin/notification/notification.cc
    src/backend/admin/zone/zone.cc
    src/backend/buffer.cc
    src/bin/cli/bankclient.cc
    src/bin/cli/chargeclient.cc
    src/bin/cli/commonclient.cc
    src/bin/cli/contactclient.cc
    src/bin/cli/domainclient.cc
    src/bin/cli/enumparamclient.cc
    src/bin/cli/fileclient.cc
    src/bin/cli/fred-admin.cc
    src/bin/cli/invoiceclient.cc
    src/bin/cli/keysetclient.cc
    src/bin/cli/notifyclient.cc
    src/bin/cli/objectclient.cc
    src/bin/cli/optys_get_undelivered.cc
    src/bin/cli/pollclient.cc
    src/bin/cli/public_request_method.cc
    src/bin/cli/registrarclient.cc
    src/bin/cli/remove_delete_candidates.cc
    src/libfred/object_state/cancel_object_state_request_id.cc
    src/libfred/object_state/create_object_state_request_id.cc
    src/libfred/object_state/get_object_state_id_map.cc
    src/util/optys/download_client.cc
    src/util/optys/upload_client.cc)
target_link_libraries(fred-admin
    admin_contact_verification
    fred
    authentic_registrar
    password_storage
    model
    banking
    corba
    corbaepp
    corbalogger
    corbaisodatetime
    corbaadmin
    corbaregistry
    corbafilters
    corbadatatypes
    mailer
    filemanager
    messages
    requests
    log
    types
    db
    oldutils
    util
    hp
    public_request)
target_link_external_libraries(fred-admin
    boost_system
    boost_filesystem
    boost_regex
    boost_program_options
    boost_date_time
    boost_random
    boost_thread
    boost_filesystem
    boost_serialization
    omniorb4
    omnidynamic4
    postgres
    threads
    openssl
    libidn
    libcurl
    libxml2
    minizip
    libssh
    zlib
    resolv
    mpdec
    rt)

# migrate_log
prepare_executable(migrate_log
    tools/logd_migration/migrate.cc
    tools/logd_migration/log_if.cc
    tools/logd_migration/m_epp_parser.cc)
target_link_libraries(migrate_log
    requests
    fred
    model
    banking
    log_impl
    log
    types
    db
    util
    oldutils)
target_link_external_libraries(migrate_log
    boost_system
    boost_regex
    boost_thread
    boost_date_time
    postgres
    libxml2)

# mojeid_contact_address_import
prepare_executable(mojeid_contact_address_import
    tools/contact_migration/mojeid_contact_address_import.cc)
target_link_libraries(mojeid_contact_address_import
    fred
    log
    db
    util)
target_link_external_libraries(mojeid_contact_address_import
    boost_system
    boost_regex
    boost_date_time
    boost_thread
    boost_random
    postgres
    threads)

# mojeid_contact_ssn_fix
prepare_executable(mojeid_contact_ssn_fix
    tools/contact_migration/mojeid_contact_ssn_fix.cc)
target_link_libraries(mojeid_contact_ssn_fix
    fred
    log
    db
    corbautil
    timezones
    corbaisodatetime
    corbanullableisodate
    util)
target_link_external_libraries(mojeid_contact_ssn_fix
    boost_system
    boost_thread
    boost_regex
    boost_random
    boost_date_time
    postgres
    threads)

# fred-disclose-flag-updater
prepare_executable(fred-disclose-flag-updater
    tools/disclose_flags_updater/thread_safe_output.cc
    tools/disclose_flags_updater/worker.cc
    tools/disclose_flags_updater/disclose_value.cc
    tools/disclose_flags_updater/disclose_settings.cc
    tools/disclose_flags_updater/contact_search_query.cc
    tools/disclose_flags_updater/fred_disclose_flags_updater.cc)
target_link_libraries(fred-disclose-flag-updater
    fred
    log
    db
    util)
target_link_external_libraries(fred-disclose-flag-updater
    postgres
    boost_regex
    boost_date_time
    boost_thread
    boost_program_options
    boost_random
    boost_system
    threads)

# crc32
prepare_executable(crc32
    src/util/hp/crc32.cc)

# test-mockdb
prepare_executable(test-mockdb
    src/util/db/test-mockdb.cc)
target_link_libraries(test-mockdb
    log
    types)
target_link_external_libraries(test-mockdb
    boost_thread
    boost_regex
    boost_date_time
    boost_system)

# test-model
prepare_executable(test-model
    src/deprecated/model/test-model.cc)
target_link_libraries(test-model
    model
    banking
    log
    types
    db)
target_link_external_libraries(test-model
    boost_system
    boost_regex
    boost_thread
    boost_date_time
    postgres)

# test-log
prepare_executable(test-log
    src/util/log/test-log.cc)
target_link_libraries(test-log
    log)
target_link_external_libraries(test-log
    boost_thread
    boost_date_time
    boost_system)

# test-db
prepare_executable(test-db
    src/util/db/test-db.cc)
target_link_libraries(test-db
    log
    types
    db)
target_link_external_libraries(test-db
    boost_thread
    boost_system
    boost_regex
    boost_date_time
    postgres)

# test-admin-contact-automerge
prepare_executable(test-admin-contact-automerge
    test/backend/admin/contact/automerge/main.cc
    test/backend/admin/contact/automerge/test-merge-auto-proc.cc
    src/backend/admin/contact/merge_contact.cc
    src/backend/admin/contact/merge_contact_logger.cc
    src/backend/admin/contact/merge_contact_auto_procedure.cc
    src/backend/admin/contact/merge_contact_reporting.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-admin-contact-automerge
    corbadatatypes
    fred
    model
    corba
    corbaregistry
    corbafilters
    mailer
    corbadatatypes
    filemanager
    requests
    log
    types
    db
    oldutils
    util
    timezones
    handle_rifd_args)
target_link_external_libraries(test-admin-contact-automerge
    boost_system
    boost_thread
    boost_regex
    boost_date_time
    boost_random
    boost_unit_test_framework
    boost_program_options
    omniorb4
    postgres
    threads)

# test-admin-contact-verification
prepare_executable(test-admin-contact-verification
    test/backend/admin/contact/verification/main.cc
    test/libfred/contact/verification/setup_utils.cc
    test/backend/admin/contact/verification/test_run_all_enqueued_checks.cc
    test/backend/admin/contact/verification/test_fill_check_queue.cc
    test/backend/admin/contact/verification/test_resolve_check.cc
    test/backend/admin/contact/verification/test_related_records.cc
    test/backend/admin/contact/verification/test_enqueue_check.cc
    test/backend/admin/contact/verification/test_delete_domains_of_invalid_contact.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-admin-contact-verification
    admin_contact_verification
    fred
    log
    db
    util
    timezones
    handle_rifd_args)
target_link_external_libraries(test-admin-contact-verification
    boost_system
    boost_unit_test_framework
    boost_thread
    boost_regex
    boost_random
    boost_program_options
    boost_date_time
    postgres
    threads)

# test-admin-notification
prepare_executable(test-admin-notification
    test/backend/admin/notification/main.cc
    test/backend/admin/notification/test_set_domain_outzone_unguarded_warning_emails.cc
    test/setup/fixtures.cc)
target_link_libraries(test-admin-notification
    admin_notification
    corba
    fred
    log
    db
    util
    timezones)
target_link_external_libraries(test-admin-notification
    boost_system
    boost_random
    boost_regex
    boost_thread
    boost_date_time
    boost_unit_test_framework
    boost_program_options
    omniorb4
    libidn
    postgres
    threads)

# test-automatic-keyset-management
prepare_executable(test-automatic-keyset-management
    src/bin/corba/nameservice.cc
    src/bin/corba/logger_client_impl.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc
    src/backend/epp/keyset/dns_key.cc
    src/backend/automatic_keyset_management/dns_key.cc
    src/backend/automatic_keyset_management/automatic_keyset_management.cc
    src/backend/automatic_keyset_management/impl/logger_request_data.cc
    src/backend/automatic_keyset_management/impl/logger_request_object_type.cc
    src/backend/automatic_keyset_management/impl/logger_request_property.cc
    src/backend/automatic_keyset_management/impl/logger_request_type.cc
    src/backend/automatic_keyset_management/impl/util.cc
    test/backend/automatic_keyset_management/main.cc
    test/backend/automatic_keyset_management/test_automatic_keyset_management.cc)
target_link_libraries(test-automatic-keyset-management
    corbaautomatickeysetmanagement
    fred
    corbautil
    corba
    corbalogger
    corbadatatypes
    corbaadmin
    corbaregistry
    corbafilters
    util
    log
    db
    timezones)
target_link_external_libraries(test-automatic-keyset-management
    boost_system
    boost_thread
    boost_random
    boost_regex
    boost_date_time
    boost_program_options
    boost_unit_test_framework
    omniorb4
    omnidynamic4
    postgres)

# test-blocking
prepare_executable(test-blocking
    test/deprecated/test_server.cc
    src/bin/corba/admin/filter_impl.cc
    test/setup/tests_common.cc
    src/backend/contact_verification/contact_verification_impl.cc
    src/libfred/contact_verification/cancel_contact_verification.cc
    src/libfred/contact_verification/contact_verification_state.cc
    src/libfred/object_state/clear_admin_object_state_request_id.cc
    src/libfred/object_state/clear_object_state_request_id.cc
    src/libfred/object_state/create_object_state_request_id.cc
    src/libfred/object_state/create_admin_object_block_request_id.cc
    src/libfred/object_state/create_admin_object_state_restore_request_id.cc
    src/libfred/object_state/cancel_object_state_request_id.cc
    src/libfred/object_state/get_object_state_id_map.cc
    src/libfred/object_state/get_blocking_status_desc_list.cc
    src/libfred/registrable_object/domain/create_domain_name_blacklist_id.cc)
target_link_libraries(test-blocking
    contact_verification
    admin_impl
    epp_impl
    password_storage
    corba
    whois_impl
    public_request
    fred
    authentic_registrar
    password_storage
    messages
    model
    db
    types
    oldutils
    requests
    banking
    log
    util
    handle_rifd_args
    corbamessages
    corbamessages
    corbazone
    filemanager
    mailer
    corbazonegenerator
    corbaadministrativeblocking
    corbaepp
    corbatechcheck
    corbalogger
    corbaadmin
    corbawhois
    corbaregistry
    corbafilters
    corbadatatypes
    mailer)
target_link_external_libraries(test-blocking
    omniorb4
    threads
    postgres
    openssl
    boost_random
    boost_regex
    boost_thread
    boost_date_time
    boost_system
    boost_serialization
    boost_program_options
    boost_thread
    libidn
    mpdec)

# test-corba-conversion
prepare_executable(test-corba-conversion
    src/backend/buffer.cc
    src/bin/corba/util/corba_conversions_buffer.cc
    src/bin/corba/util/corba_conversions_isodatetime.cc
    src/util/corba_conversion.cc
    src/bin/corba/mojeid/mojeid_corba_conversion.cc
    test/util/test_corba_conversion_main.cc
    test/util/test_corba_conversion.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-corba-conversion
    corba_conversion_test
    corbabuffer
    corbaisodate
    corbaisodatetime
    corbanullableisodate
    corbanullableisodatetime
    corbautil
    corbamojeid
    fred
    util
    log
    timezones)
target_link_external_libraries(test-corba-conversion
    postgres
    boost_system
    boost_random
    boost_thread
    boost_unit_test_framework
    boost_program_options
    boost_regex
    boost_date_time
    omniorb4
    omnidynamic4)

# test-epp
prepare_executable(test-epp
    test/backend/epp/main.cc
    test/backend/epp/test_conditionally_enqueue_notification.cc
    test/backend/epp/contact/test_create_contact.cc
    test/backend/epp/contact/test_delete_contact.cc
    test/backend/epp/contact/test_info_contact.cc
    test/backend/epp/contact/test_update_contact.cc
    test/backend/epp/contact/test_check_contact.cc
    test/backend/epp/contact/test_transfer_contact.cc
    test/backend/epp/contact/test_contact_handle_state_to_check_result.cc
    test/backend/epp/credit/test_client_credit.cc
    test/backend/epp/domain/fixture.cc
    test/backend/epp/domain/test_check_domain.cc
    test/backend/epp/domain/test_delete_domain.cc
    test/backend/epp/domain/test_info_domain.cc
    test/backend/epp/domain/test_transfer_domain.cc
    test/backend/epp/domain/test_update_domain.cc
    test/backend/epp/domain/test_create_domain.cc
    test/backend/epp/domain/test_renew_domain.cc
    test/backend/epp/domain/test_domain_billing.cc
    test/backend/epp/keyset/fixture.cc
    test/backend/epp/keyset/test_create_keyset.cc
    test/backend/epp/nsset/test_check_nsset.cc
    test/backend/epp/nsset/test_info_nsset.cc
    test/backend/epp/nsset/test_delete_nsset.cc
    test/backend/epp/nsset/test_create_nsset.cc
    test/backend/epp/nsset/test_update_nsset.cc
    test/backend/epp/nsset/test_transfer_nsset.cc
    test/setup/fixtures_utils.cc
    test/backend/epp/poll/fixture.cc
    test/backend/epp/poll/test_poll_request.cc
    test/backend/epp/poll/test_poll_acknowledgement.cc
    test/backend/epp/poll/test_poll_request_nsset_details.cc
    test/backend/epp/poll/test_poll_request_domain_details.cc
    test/backend/epp/poll/test_poll_request_keyset_details.cc
    test/setup/fixtures.cc)
target_link_libraries(test-epp
    epp_impl
    fred
    timezones
    password_storage
    authentic_registrar
    types
    db
    log
    util
    handle_rifd_args)
target_link_external_libraries(test-epp
    boost_system
    boost_unit_test_framework
    boost_regex
    boost_program_options
    boost_random
    boost_thread
    boost_date_time
    postgres
    omnidynamic4
    mpdec
    threads)

# test-exception
prepare_executable(test-exception
    test/util/test_exception.cc)
target_link_external_libraries(test-exception
    boost_system
    mpdec)

# test-admin-domain
prepare_executable(test-admin-domain
    test/backend/admin/domain/main.cc
    test/backend/admin/domain/test_create_expired_domain.cc
    src/backend/admin/domain/create_expired_domain.cc
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc)
target_link_libraries(test-admin-domain
    admin_notification
    corba
    fred
    log
    db
    util
    timezones)
target_link_external_libraries(test-admin-domain
    boost_system
    boost_unit_test_framework
    boost_regex
    boost_program_options
    boost_random
    boost_thread
    boost_date_time
    postgres
    omniorb4
    threads)

prepare_executable(test-admin-zone
    src/backend/admin/zone/zone.cc
    test/backend/admin/zone/main.cc
    test/backend/admin/zone/test_add_zone.cc
    test/backend/admin/zone/test_add_zone_ns.cc
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc)
target_link_libraries(test-admin-zone
    corba
    corbafilters
    corbadatatypes
    fred
    log
    db
    util
    types
    timezones)
target_link_external_libraries(test-admin-zone
    boost_system
    boost_unit_test_framework
    boost_program_options
    boost_date_time
    boost_regex
    boost_random
    omniorb4
    boost_thread
    threads
    postgres
    libidn)

# test-libfred
prepare_executable(test-libfred
    test/libfred/main.cc
    test/libfred/registrar/test_registrar_zone_access.cc
    test/libfred/registrar/test_get_registrar_zone_credit.cc
    test/libfred/registrar/test_info_registrar.cc
    test/libfred/registrar/test_create_registrar.cc
    test/libfred/registrar/test_update_registrar.cc
    test/libfred/registrar/group/test_create.cc
    test/libfred/registrar/group/test_update.cc
    test/libfred/registrar/group/test_cancel.cc
    test/libfred/registrar/group/test_get.cc
    test/libfred/registrar/group/membership/test_create.cc
    test/libfred/registrar/group/membership/test_end.cc
    test/libfred/registrar/group/membership/test_info_by_group.cc
    test/libfred/registrar/group/membership/test_info_by_registrar.cc
    test/libfred/contact/test_delete_contact.cc
    test/libfred/contact/test_create_contact.cc
    test/libfred/contact/test_info_contact.cc
    test/libfred/contact/test_merge_contact.cc
    test/libfred/contact/test_update_contact.cc
    test/libfred/contact/test_copy_contact.cc
    test/libfred/contact/test_transfer_contact.cc
    test/libfred/contact/test_mojeid_checkers.cc
    test/libfred/contact/test_undisclose_address.cc
    src/util/types/birthdate.cc
    test/libfred/domain/test_update_domain.cc
    test/libfred/domain/test_renew_domain.cc
    test/libfred/domain/test_info_domain.cc
    test/libfred/domain/test_delete_domain.cc
    test/libfred/domain/test_create_domain.cc
    test/libfred/domain/test_check_domain.cc
    test/libfred/domain/test_domain_name.cc
    test/libfred/domain/test_create_domain_name_blacklist_id.cc
    test/libfred/domain/test_transfer_domain.cc
    test/libfred/keyset/test_delete_keyset.cc
    test/libfred/keyset/test_info_keyset.cc
    test/libfred/keyset/test_update_keyset.cc
    test/libfred/keyset/test_transfer_keyset.cc
    test/libfred/nsset/test_update_nsset.cc
    test/libfred/nsset/test_delete_nsset.cc
    test/libfred/nsset/test_info_nsset.cc
    test/libfred/nsset/test_transfer_nsset.cc
    test/libfred/public_request/test_create_public_request.cc
    test/libfred/public_request/test_create_public_request_auth.cc
    test/libfred/public_request/test_update_public_request.cc
    test/libfred/contact/verification/setup_utils.cc
    test/libfred/contact/verification/test_create_check.cc
    test/libfred/contact/verification/test_update_check.cc
    test/libfred/contact/verification/test_create_test.cc
    test/libfred/contact/verification/test_update_test.cc
    test/libfred/contact/verification/test_info_check.cc
    test/libfred/contact/verification/test_list_checks.cc
    test/libfred/contact/verification/test_list_enum_objects.cc
    test/libfred/contact/verification/test_django_email_format_check.cc
    test/libfred/poll/test_create_delete_contact_poll_message.cc
    test/libfred/poll/test_create_delete_domain_poll_message.cc
    test/libfred/poll/test_create_update_object_poll_message.cc
    test/libfred/poll/test_get_request_fee_info_message.cc
    test/libfred/poll/test_create_state_messages.cc
    test/libfred/object/test_generate_authinfo_password.cc
    test/libfred/object/test_generated_authinfo_password.cc
    test/libfred/object/test_transfer_object.cc
    test/libfred/object_state/test_object_state.cc
    test/libfred/object_state/test_create_object_state_request_id.cc
    test/libfred/object_state/test_create_admin_object_block_request_id.cc
    test/libfred/object_state/test_create_admin_object_state_restore_request_id.cc
    test/libfred/object_state/test_cancel_object_state_request_id.cc
    test/libfred/object/test_registry_object_type.cc
    test/libfred/notifier/test_enqueue_notification.cc
    test/libfred/notifier/test_event_on_object_enum.cc
    test/libfred/notifier/test_gather_email_addresses_contact_create.cc
    test/libfred/notifier/test_gather_email_addresses_contact_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_contact_delete.cc
    test/libfred/notifier/test_gather_email_addresses_contact_update.cc
    test/libfred/notifier/test_gather_email_addresses_domain_create.cc
    test/libfred/notifier/test_gather_email_addresses_domain_update.cc
    test/libfred/notifier/test_gather_email_addresses_domain_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_domain_delete.cc
    test/libfred/notifier/test_gather_email_addresses_domain_renew.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_create.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_update.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_keyset_delete.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_create.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_update.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_transfer.cc
    test/libfred/notifier/test_gather_email_addresses_nsset_delete.cc
    test/libfred/notifier/test_gather_email_content_contact_create.cc
    test/libfred/notifier/test_gather_email_content_contact_delete.cc
    test/libfred/notifier/test_gather_email_content_contact_transfer.cc
    test/libfred/notifier/test_gather_email_content_contact_update.cc
    test/libfred/notifier/test_gather_email_content_domain_create.cc
    test/libfred/notifier/test_gather_email_content_domain_delete.cc
    test/libfred/notifier/test_gather_email_content_domain_renew.cc
    test/libfred/notifier/test_gather_email_content_domain_transfer.cc
    test/libfred/notifier/test_gather_email_content_domain_update.cc
    test/libfred/notifier/test_gather_email_content_keyset_create.cc
    test/libfred/notifier/test_gather_email_content_keyset_delete.cc
    test/libfred/notifier/test_gather_email_content_keyset_transfer.cc
    test/libfred/notifier/test_gather_email_content_keyset_update.cc
    test/libfred/notifier/test_gather_email_content_nsset_create.cc
    test/libfred/notifier/test_gather_email_content_nsset_delete.cc
    test/libfred/notifier/test_gather_email_content_nsset_transfer.cc
    test/libfred/notifier/test_gather_email_content_nsset_update.cc
    test/libfred/notifier/test_process_one_notification_request.cc
    test/libfred/registrar/epp_auth/test_add_registrar_epp_auth.cc
    test/libfred/registrar/epp_auth/test_delete_registrar_epp_auth.cc
    test/libfred/registrar/certification/test_create_registrar_certification.cc
    test/libfred/registrar/certification/test_update_registrar_certification.cc
    test/libfred/registrar/certification/test_get_registrar_certifications.cc
    test/libfred/registrar/zone_access/test_add_zone_access.cc
    test/libfred/registrar/zone_access/util.cc
    test/libfred/registrar/zone_access/test_update_zone_access.cc
    test/libfred/test_opexception.cc
    test/libfred/test_check_handle.cc
    test/libfred/zone/zone_ns/test_create_zone_ns.cc
    test/libfred/zone/zone_ns/test_info_zone_ns.cc
    test/libfred/zone/zone_ns/test_update_zone_ns.cc
    test/libfred/zone/zone_ns/util.cc
    test/libfred/zone/zone_soa/test_create_zone_soa.cc
    test/libfred/zone/zone_soa/test_info_zone_soa.cc
    test/libfred/zone/zone_soa/test_update_zone_soa.cc
    test/libfred/zone/zone_soa/util.cc
    test/libfred/zone/test_create_zone.cc
    test/libfred/zone/test_info_zone.cc
    test/libfred/zone/test_update_zone.cc
    test/libfred/zone/util.cc
    test/util/test_param_query.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc
    src/util/random.cc)
target_link_libraries(test-libfred
    fred
    util
    log
    db
    timezones
    handle_rifd_args
    corba
    mailer
    corbadatatypes
    password_storage
    public_request)
target_link_external_libraries(test-libfred
    postgres
    boost_program_options
    boost_regex
    boost_date_time
    boost_random
    boost_thread
    boost_system
    boost_unit_test_framework
    omniorb4
    libidn
    mpdec
    threads
    openssl)

# test-optys
prepare_executable(test-optys
    test/test_optys_main.cc
    test/util/test_optys.cc
    src/util/optys/download_client.cc
    src/bin/cli/optys_get_undelivered.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-optys
    fred
    util
    log
    types
    timezones)
target_link_external_libraries(test-optys
    postgres
    boost_unit_test_framework
    boost_program_options
    boost_filesystem
    boost_regex
    boost_thread
    boost_random
    boost_date_time
    boost_system
    threads)

# test-password-storage
prepare_executable(test-password-storage
    test/util/test_password_storage.cc
    test/util/test_password_storage_main.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-password-storage
    fred
    corbaisodatetime
    corbanullableisodate
    password_storage
    util
    log)
target_link_external_libraries(test-password-storage
    boost_random
    boost_date_time
    boost_system
    boost_regex
    boost_thread
    boost_program_options
    boost_unit_test_framework
    postgres
    openssl)

# test-public-request
prepare_executable(test-public-request
    test/backend/public_request/main.cc
    src/backend/buffer.cc
    test/backend/public_request/test_registry_email.cc
    test/backend/public_request/test_non_registry_email.cc
    test/backend/public_request/test_create_block_unblock_request.cc
    test/backend/public_request/test_create_public_request_pdf.cc
    test/backend/public_request/fixture_common.cc
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc)
target_link_libraries(test-public-request
    public_request
    fred
    corba
    mailer
    corbadatatypes
    requests
    log
    db
    util
    timezones
    handle_rifd_args)
target_link_external_libraries(test-public-request
    boost_regex
    boost_thread
    boost_date_time
    boost_program_options
    boost_random
    boost_system
    boost_unit_test_framework
    omniorb4
    postgres
    libidn
    threads)

# test-record-statement
prepare_executable(test-record-statement
    src/backend/buffer.cc
    src/backend/record_statement/record_statement.cc
    src/backend/record_statement/exceptions.cc
    src/backend/record_statement/impl/record_statement_xml.cc
    src/backend/record_statement/impl/factory.cc
    src/backend/record_statement/impl/utc_impl.cc
    src/backend/record_statement/impl/europe_prague_impl.cc
    src/backend/record_statement/impl/util.cc
    test/backend/record_statement/main.cc
    test/backend/record_statement/test_record_statement.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-record-statement
    corbabuffer
    corbaisodate
    corbaisodatetime
    fred
    util
    log
    db
    timezones
    corba
    mailer
    corbadatatypes)
target_link_external_libraries(test-record-statement
    boost_regex
    boost_thread
    boost_date_time
    boost_program_options
    boost_random
    boost_unit_test_framework
    boost_system
    postgres
    omniorb4
    threads)

# test-admin-contact-automerge
prepare_executable(test-accounting
    src/backend/accounting/accounting.cc
    src/backend/accounting/exceptions.cc
    src/backend/accounting/impl/accounting.cc
    src/backend/accounting/impl/exceptions.cc
    src/backend/credit.cc
    test/backend/accounting/main.cc
    test/backend/accounting/test_get_registrar_by_handle.cc
    test/backend/accounting/test_get_registrar_by_payment.cc
    test/backend/accounting/test_get_registrar_references.cc
    test/backend/accounting/test_get_zone_by_payment.cc
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc)
target_link_libraries(test-accounting
    banking
    corba
    corbaaccounting
    corbautil
    timezones
    fred
    log
    model
    db
    oldutils
    password_storage
    types
    util)
target_link_external_libraries(test-accounting
    boost_random
    boost_system
    boost_regex
    boost_date_time
    boost_thread
    boost_unit_test_framework
    boost_program_options
    libidn
    postgres
    omniorb4
    openssl
    threads
    mpdec)

# test-server
prepare_executable(test-server
    test/deprecated/test_server.cc
    test/deprecated/test_conv.cc
    test/deprecated/test_file_manager_client.cc
    test/deprecated/test_db.cc
    test/deprecated/test_registrar_group.cc
    test/deprecated/test_registrar_blocking.cc
    src/bin/corba/admin/filter_impl.cc
    test/deprecated/test_logd.cc
    test/deprecated/test_msgd.cc
    test/deprecated/test_admin.cc
    test/deprecated/test_corbans.cc
    test/deprecated/test_whois.cc
    test/deprecated/test_cache.cc
    test/setup/tests_common.cc
    test/deprecated/test_filters_date_time.cc
    test/deprecated/test_invoice_common.cc
    test/deprecated/test_invoice.cc
    test/deprecated/test_invoice_account.cc
    test/deprecated/test_common_registry.cc
    test/deprecated/test_invoice2.cc
    test/util/test_decimal.cc
    test/util/test_shellcmd.cc
    test/util/test_csv_serializer.cc
    test/deprecated/test_contact_verification.cc
    test/deprecated/test_locking_trigger.cc
    test/util/test_nullable.cc
    test/util/test_optional.cc
    src/backend/contact_verification/contact_verification_impl.cc
    src/libfred/contact_verification/cancel_contact_verification.cc
    src/libfred/contact_verification/contact_verification_state.cc
    test/util/test_printable.cc
    test/util/test_csv_parser.cc
    src/util/optys/download_client.cc
    src/util/corba_conversion.cc
    test/util/test_is_equal_optional_nullable.cc
    src/backend/admin/contact/merge_contact.cc
    src/backend/admin/contact/merge_contact_logger.cc
    src/backend/admin/contact/merge_contact_auto_procedure.cc
    src/backend/admin/contact/merge_contact_reporting.cc
    src/libfred/object_state/create_object_state_request_id.cc
    src/libfred/object_state/get_object_state_id_map.cc
    src/libfred/object_state/cancel_object_state_request_id.cc
    src/backend/domain_browser/domain_browser.cc
    test/backend/domain_browser/test_domain_browser.cc
    test/setup/fixtures.cc)
target_link_libraries(test-server
    contact_verification
    admin_impl
    epp_impl
    password_storage
    corba
    whois_impl
    public_request
    admin_contact_verification
    admin_notification
    admin_create_expired_domain
    fred
    authentic_registrar
    password_storage
    messages
    model
    timezones
    db
    types
    oldutils
    requests
    banking
    log
    util
    handle_rifd_args
    corbamessages
    corbazone
    filemanager
    mailer
    corbazonegenerator
    corbaadministrativeblocking
    corbaepp
    corbatechcheck
    corbalogger
    corbaadmin
    corbawhois
    corbaregistry
    corbafilters
    corbadatatypes)
target_link_external_libraries(test-server
    boost_system
    boost_serialization
    boost_regex
    boost_thread
    boost_filesystem
    boost_date_time
    boost_program_options
    boost_random
    omniorb4
    omnidynamic4
    openssl
    libidn
    postgres
    libxml2
    threads
    mpdec)

# test-whois
prepare_executable(test-whois
    test/backend/whois/main.cc
    test/backend/whois/test_whois_get_registrar_by_handle.cc
    test/backend/whois/test_whois_get_contact_by_handle.cc
    test/backend/whois/test_whois_get_domain_by_handle.cc
    test/backend/whois/test_whois_get_domains_by_admin_contact.cc
    test/backend/whois/test_whois_get_domains_by_keyset.cc
    test/backend/whois/test_whois_get_domains_by_nsset.cc
    test/backend/whois/test_whois_get_domains_by_registrant.cc
    test/backend/whois/test_whois_get_keyset_by_handle.cc
    test/backend/whois/test_whois_get_keysets_by_tech_c.cc
    test/backend/whois/test_whois_get_managed_zone_list.cc
    test/backend/whois/test_whois_get_nameserver_by_fqdn.cc
    test/backend/whois/test_whois_get_nsset_by_handle.cc
    test/backend/whois/test_whois_get_nssets_by_ns.cc
    test/backend/whois/test_whois_get_nssets_by_tech_c.cc
    test/backend/whois/test_whois_get_registrars.cc
    test/backend/whois/test_whois_get_registrar_groups.cc
    test/backend/whois/test_whois_get_registrar_certification_list.cc
    test/backend/whois/test_whois_get_object_status_descriptions.cc
    test/setup/fixtures_utils.cc
    test/setup/fixtures.cc)
target_link_libraries(test-whois
    corbadatatypes
    whois2_impl
    fred
    model
    corba
    corbaregistry
    corbafilters
    mailer
    corbanullableisodate
    corbanullableisodatetime
    corbadatatypes
    filemanager
    requests
    log
    types
    db
    oldutils
    util
    timezones
    handle_rifd_args)
target_link_external_libraries(test-whois
    boost_regex
    boost_thread
    boost_date_time
    boost_program_options
    boost_random
    boost_system
    boost_unit_test_framework
    postgres
    omniorb4
    mpdec
    threads)

prepare_executable(test-delete-objects-marked-as-delete-candidate
    test/test_delete_objects_marked_as_delete_candidate.cc)
target_link_libraries(test-delete-objects-marked-as-delete-candidate
    fred
    log
    db
    oldutils
    util)
target_link_external_libraries(test-delete-objects-marked-as-delete-candidate
    boost_program_options
    boost_date_time
    boost_regex
    boost_random
    boost_filesystem
    boost_thread
    boost_system
    postgres
    libidn
    threads
    mpdec
    rt
    resolv)

install(TARGETS fred-accifd DESTINATION ${SBINDIR})
install(TARGETS fred-adifd DESTINATION ${SBINDIR})
install(TARGETS fred-admin DESTINATION ${SBINDIR})
install(TARGETS fred-akmd DESTINATION ${SBINDIR})
install(TARGETS fred-dbifd DESTINATION ${SBINDIR})
install(TARGETS fred-logd DESTINATION ${SBINDIR})
install(TARGETS fred-mifd DESTINATION ${SBINDIR})
install(TARGETS fred-mojeid-msgen DESTINATION ${SBINDIR})
install(TARGETS fred-msgd DESTINATION ${SBINDIR})
install(TARGETS fred-pifd DESTINATION ${SBINDIR})
install(TARGETS fred-rifd DESTINATION ${SBINDIR})
install(TARGETS fred-rsifd DESTINATION ${SBINDIR})
install(TARGETS migrate_log DESTINATION ${SBINDIR})
install(TARGETS mojeid_contact_address_import DESTINATION ${SBINDIR})
install(TARGETS mojeid_contact_ssn_fix DESTINATION ${SBINDIR})
install(TARGETS fred-disclose-flag-updater DESTINATION ${SBINDIR})

install(DIRECTORY ${CMAKE_BINARY_DIR}/config/ DESTINATION etc/fred FILES_MATCHING PATTERN "*.conf")
install(DIRECTORY ${CMAKE_BINARY_DIR}/init-scripts/ DESTINATION etc/init.d FILES_MATCHING PATTERN "fred-server"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/test/deprecated/ DESTINATION ${DATAROOTDIR}/doc/fred-server FILES_MATCHING PATTERN "example_payments.xml")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/test/deprecated/ DESTINATION ${DATAROOTDIR}/doc/fred-server FILES_MATCHING PATTERN "test_file.pdf")

if(NOT DO_NOT_INSTALL_TESTS)
    install(TARGETS test-admin-contact-automerge DESTINATION ${BINDIR})
    install(TARGETS test-admin-contact-verification DESTINATION ${BINDIR})
    install(TARGETS test-admin-notification DESTINATION ${BINDIR})
    install(TARGETS test-automatic-keyset-management DESTINATION ${BINDIR})
    install(TARGETS test-blocking DESTINATION ${BINDIR})
    install(TARGETS test-corba-conversion DESTINATION ${BINDIR})
    install(TARGETS test-epp DESTINATION ${BINDIR})
    install(TARGETS test-exception DESTINATION ${BINDIR})
    install(TARGETS test-libfred DESTINATION ${BINDIR})
    install(TARGETS test-optys DESTINATION ${BINDIR})
    install(TARGETS test-password-storage DESTINATION ${BINDIR})
    install(TARGETS test-public-request DESTINATION ${BINDIR})
    install(TARGETS test-record-statement DESTINATION ${BINDIR})
    install(TARGETS test-server DESTINATION ${BINDIR})
    install(TARGETS test-whois DESTINATION ${BINDIR})
    install(TARGETS test-admin-domain DESTINATION ${BINDIR})
    install(TARGETS test-admin-zone DESTINATION ${BINDIR})
    install(TARGETS test-delete-objects-marked-as-delete-candidate DESTINATION ${BINDIR})
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_PROGRAM)
    if(NOT TARGET dist)
        add_custom_target(dist
            COMMAND ${GIT_PROGRAM} archive --format=tar.gz --prefix=${TARBALL_FILE_NAME}/ HEAD -o ${CMAKE_BINARY_DIR}/${TARBALL_FILE_NAME}.tar.gz
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif()

    if(NOT TARGET distcheck)
        set(DISTCHECK_BUILD_DIR "_build")
        set(DISTCHECK_INSTALL_DIR "_inst")
        if(${CMAKE_GENERATOR} STREQUAL "Ninja")
            set(DISTCHECK_BUILD_CMD "ninja")
            set(DISTCHECK_MAKEFLAGS "")
        else()
            set(DISTCHECK_BUILD_CMD "make")
            execute_process(COMMAND "grep" "-c" "model name" "/proc/cpuinfo"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NOCPUS)
            string(REGEX REPLACE "\n" "" NOCPUS ${NOCPUS})
            set(DISTCHECK_MAKEFLAGS "MAKEFLAGS=-j${NOCPUS}")
        endif()
        add_custom_target(distcheck
            COMMAND
            rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && mkdir ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR}
            && (tar xzf ${TARBALL_FILE_NAME}.tar.gz
                || (echo 'Error: failed to unpack the archive' 2>&1\; false))
            && (cd ${DISTCHECK_BUILD_DIR}
                && (${CMAKE_COMMAND} -G${CMAKE_GENERATOR} -DCMAKE_INSTALL_PREFIX=../${DISTCHECK_INSTALL_DIR} -DIDL_DIR=${IDL_DIR} ../${TARBALL_FILE_NAME}
                    || (echo 'Error: failed to configure the archive' 2>&1\; false))
                && (${DISTCHECK_MAKEFLAGS} ${DISTCHECK_BUILD_CMD}
                    || (echo 'Error: the build target failed ' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} install
                    || (echo 'Error: the install target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} uninstall
                    || (echo 'Error: the uninstall target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} clean
                    || (echo 'Error: the clean target failed' 2>&1\; false)))
            && rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && echo 'PASS: ' ${TARBALL_FILE_NAME}.tar.gz 'is ready for distribution.'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Checking the distribution archive...")
        add_dependencies(distcheck dist)
    endif()
else()
    message(STATUS "no git -- not including dist and distcheck targets")
endif()
