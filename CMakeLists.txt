cmake_minimum_required(VERSION 3.5)

# macros

macro(set_default variable_name variable_value)
    if(NOT ${variable_name})
        set(${variable_name} ${variable_value})
    endif()
endmacro()

# project details

include("cmake/package.cmake")

package(fred-server
    FULL_VERSION "2.52.0")

project(FredServer
    VERSION ${PACKAGE_VERSION}
    LANGUAGES CXX)

# build options

option(CMAKE_EXPORT_COMPILE_COMMANDS "If enabled, generates a compile_commands.json file containing the exact compiler calls." ON)

# compile options

set_default(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

option(LIBFRED_BUILD_TESTS "Build libfred tests" YES)

# dependencies

include("cmake/package_dependencies.cmake")

set(DB_BUILD FALSE)
set(IDL_BUILD FALSE)
set(LIBFRED_BUILD_TESTS TRUE)
add_package_dependencies(FROM_FILE "dependencies.txt")
provide_package_dependency_dist(db)
provide_package_dependency_dist(idl)

# build

# Set CMake policies to support later version behaviour
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # option() honors variables already set

#Make sure that custom modules like FindOMNIORB are found
set_default(IDL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/idl)
set(IDL_SOURCE_DIR ${IDL_PROJECT_DIR}/idl)
set(IDL_DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/corba)
list(INSERT CMAKE_MODULE_PATH 0 ${IDL_PROJECT_DIR}/cmake)

##############################################
# Declare dependencies
find_package(OMNIORB4 REQUIRED)
find_program(GIT_PROGRAM git)

macro(normalize_path variable_name path)
    string(REGEX REPLACE "//+" "/" ${variable_name} "${path}")
    if(NOT "${${variable_name}}" STREQUAL "/")
        string(REGEX REPLACE "/$" "" ${variable_name} "${${variable_name}}")
    endif()
endmacro()

macro(set_default_path variable_name path)
    if(NOT ${variable_name})
        normalize_path(${variable_name} "${path}")
    endif()
endmacro()

set(USR_PREFIX "${CMAKE_INSTALL_PREFIX}")
normalize_path(NORMALIZED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if("${NORMALIZED_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set_default_path(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share")
else()
    set_default_path(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/usr/share")
    if(USE_USR_PREFIX)
        set(USR_PREFIX "${CMAKE_INSTALL_PREFIX}/usr")
    endif()
endif()
normalize_path(DATAROOTDIR ${DATAROOTDIR})
normalize_path(USR_PREFIX ${USR_PREFIX})

set_default_path(BINDIR ${USR_PREFIX}/bin)
normalize_path(BINDIR ${BINDIR})
set_default_path(SBINDIR ${USR_PREFIX}/sbin)
normalize_path(SBINDIR ${SBINDIR})

set_default_path(LOCALSTATEDIR ${CMAKE_INSTALL_PREFIX}/var)
normalize_path(LOCALSTATEDIR ${LOCALSTATEDIR})
set_default_path(MONITORINGIPS ${CMAKE_INSTALL_PREFIX}/etc/fred/monitoring_hosts.conf)
normalize_path(MONITORINGIPS ${MONITORINGIPS})
set_default_path(SYSCONFDIR ${CMAKE_INSTALL_PREFIX}/etc)
normalize_path(SYSCONFDIR ${SYSCONFDIR})
set_default_path(PREFIX ${CMAKE_INSTALL_PREFIX})
normalize_path(PREFIX ${PREFIX})

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/hpmail.conf.in ${CMAKE_CURRENT_BINARY_DIR}/config/hpmail.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/optysmail.conf.in ${CMAKE_CURRENT_BINARY_DIR}/config/optysmail.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/server.conf.in ${CMAKE_CURRENT_BINARY_DIR}/config/server.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/init-scripts/fred-server.in ${CMAKE_CURRENT_BINARY_DIR}/init-scripts/fred-server @ONLY)

find_package(Threads REQUIRED)

#due to src/deprecated/util/pqsql.hh
set(PostgreSQL_ADDITIONAL_VERSIONS "9.6")
find_package(PostgreSQL ${PostgreSQL_ADDITIONAL_VERSIONS} REQUIRED)
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxy.conf.in ${CMAKE_CURRENT_BINARY_DIR}/doxy.conf)

find_package(PkgConfig REQUIRED)

pkg_search_module(LIBSSH REQUIRED libssh)
pkg_search_module(LIBCURL REQUIRED libcurl)
pkg_search_module(MINIZIP REQUIRED minizip)
pkg_search_module(XML2 REQUIRED libxml-2.0>=2.5.0)

set(CORBA_LIBRARIES ${OMNIDYNAMIC4_LIBRARIES} ${OMNIDYNAMIC4_LIBRARIES})


##############################################
# How to generate C++ files from IDL
OMNIORB4_IDL_TO_CPP(${IDL_SOURCE_DIR} ${IDL_DESTINATION_DIR} IDL_CC_FILES IDL_HH_FILES
    Accounting.idl
    Admin.idl
    AdminContactVerification.idl
    AdministrativeBlocking.idl
    AutomaticKeysetManagement.idl
    Buffer.idl
    ContactVerification.idl
    _dataTypes.idl
    DateTime.idl
    DomainBrowser.idl
    EPP.idl
    FileManager.idl
    Filters.idl
    IsoDate.idl
    IsoDateTime.idl
    Logger.idl
    Mailer.idl
    Messages.idl
    MojeID.idl
    Notification.idl
    NullableIsoDate.idl
    NullableIsoDateTime.idl
    NullableTypes.idl
    PublicRequest.idl
    RecordStatement.idl
    Registry.idl
    TechCheck.idl
    Whois.idl
    Whois2.idl
    Zone.idl
    ZoneGenerator.idl)

OMNIORB4_IDL_TO_CPP(${CMAKE_CURRENT_SOURCE_DIR}/test/util ${IDL_DESTINATION_DIR} TEST_IDL_CC_FILES TEST_IDL_HH_FILE
    corba_conversion_test.idl)

function(set_common_target_compile_options target_name)
    target_compile_options(${target_name}
        PRIVATE
            "-ggdb"
            "-grecord-gcc-switches"
            "-Wall"
            "-Wextra"
            "-fdiagnostics-color=always")
endfunction()

function(set_common_library_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    set_common_target_compile_options(${target_name})
    target_include_directories(${target_name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
endfunction()

function(add_common_library_target target_name)
    message(STATUS "add library: " ${target_name})
    add_library(${target_name} STATIC EXCLUDE_FROM_ALL ${ARGN})
    set_common_library_target_properties(${target_name})
endfunction()

function(add_corba_library_target target_name)
    message(STATUS "add corba library: " ${target_name})
    add_library(${target_name} STATIC EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    target_compile_options(${target_name}
        PRIVATE
            "-ggdb"
            "-grecord-gcc-switches"
            "-Wall"
            "-Wextra"
            "-Wno-unused-variable"
            "-Wno-unused-parameter"
            "-fdiagnostics-color=always")
    target_include_directories(${target_name} PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>)
    target_link_libraries(${target_name} PRIVATE ${CORBA_LIBRARIES})
endfunction()

function(set_common_executable_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    set_common_target_compile_options(${target_name})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

function(add_common_executable_target target_name)
    message(STATUS "add executable: " ${target_name})
    add_executable(${target_name} ${ARGN})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    set_common_executable_target_properties(${target_name})
endfunction()

add_corba_library_target(corba_messages
    ${IDL_DESTINATION_DIR}/MessagesSK.cc
    ${IDL_DESTINATION_DIR}/MessagesDynSK.cc)

add_corba_library_target(corba_buffer
    ${IDL_DESTINATION_DIR}/BufferSK.cc
    ${IDL_DESTINATION_DIR}/BufferDynSK.cc)

add_corba_library_target(corba_isodate
    ${IDL_DESTINATION_DIR}/IsoDateSK.cc
    ${IDL_DESTINATION_DIR}/IsoDateDynSK.cc)

add_corba_library_target(corba_nullableisodate
    ${IDL_DESTINATION_DIR}/NullableIsoDateSK.cc
    ${IDL_DESTINATION_DIR}/NullableIsoDateDynSK.cc)
target_link_libraries(corba_nullableisodate PRIVATE corba_isodate)

add_corba_library_target(corba_isodatetime
    ${IDL_DESTINATION_DIR}/IsoDateTimeSK.cc
    ${IDL_DESTINATION_DIR}/IsoDateTimeDynSK.cc)

add_corba_library_target(corba_nullableisodatetime
    ${IDL_DESTINATION_DIR}/NullableIsoDateTimeSK.cc
    ${IDL_DESTINATION_DIR}/NullableIsoDateTimeDynSK.cc)
target_link_libraries(corba_nullableisodatetime PRIVATE corba_isodatetime)

add_corba_library_target(corba_mojeid
    ${IDL_DESTINATION_DIR}/MojeIDSK.cc
    ${IDL_DESTINATION_DIR}/MojeIDDynSK.cc)
target_link_libraries(corba_mojeid PRIVATE
    corba_buffer
    corba_nullableisodate
    corba_isodatetime)

add_corba_library_target(corba_datetime
    ${IDL_DESTINATION_DIR}/DateTimeSK.cc
    ${IDL_DESTINATION_DIR}/DateTimeDynSK.cc)

add_corba_library_target(corba_datatypes
    ${IDL_DESTINATION_DIR}/_dataTypesSK.cc
    ${IDL_DESTINATION_DIR}/_dataTypesDynSK.cc)
target_link_libraries(corba_datatypes PRIVATE corba_datetime)

add_corba_library_target(corba_filemanager
    ${IDL_DESTINATION_DIR}/FileManagerSK.cc
    ${IDL_DESTINATION_DIR}/FileManagerDynSK.cc)
target_link_libraries(corba_filemanager PRIVATE corba_datatypes)

add_corba_library_target(corba_mailer
    ${IDL_DESTINATION_DIR}/MailerSK.cc
    ${IDL_DESTINATION_DIR}/MailerDynSK.cc)
target_link_libraries(corba_mailer PRIVATE corba_datatypes)

add_corba_library_target(corba_zone
    ${IDL_DESTINATION_DIR}/ZoneSK.cc
    ${IDL_DESTINATION_DIR}/ZoneDynSK.cc)

add_corba_library_target(corba_nullabletypes
    ${IDL_DESTINATION_DIR}/NullableTypesSK.cc
    ${IDL_DESTINATION_DIR}/NullableTypesDynSK.cc)

add_corba_library_target(corba_filters
    ${IDL_DESTINATION_DIR}/FiltersSK.cc
    ${IDL_DESTINATION_DIR}/FiltersDynSK.cc)
target_link_libraries(corba_filters PRIVATE corba_datatypes)

add_corba_library_target(corba_registry
    ${IDL_DESTINATION_DIR}/RegistrySK.cc
    ${IDL_DESTINATION_DIR}/RegistryDynSK.cc)
target_link_libraries(corba_registry PRIVATE corba_filters)

add_corba_library_target(corba_whois
    ${IDL_DESTINATION_DIR}/WhoisSK.cc
    ${IDL_DESTINATION_DIR}/WhoisDynSK.cc)
target_link_libraries(corba_whois PRIVATE corba_registry)

add_corba_library_target(corba_admin
    ${IDL_DESTINATION_DIR}/AdminSK.cc
    ${IDL_DESTINATION_DIR}/AdminDynSK.cc)
target_link_libraries(corba_admin PRIVATE corba_registry)

add_corba_library_target(corba_whois2
    ${IDL_DESTINATION_DIR}/Whois2SK.cc
    ${IDL_DESTINATION_DIR}/Whois2DynSK.cc)
target_link_libraries(corba_whois2 PRIVATE
    corba_nullableisodate
    corba_nullableisodatetime
    corba_nullabletypes)

add_corba_library_target(corba_logger
    ${IDL_DESTINATION_DIR}/LoggerSK.cc
    ${IDL_DESTINATION_DIR}/LoggerDynSK.cc)
target_link_libraries(corba_logger PRIVATE corba_admin)

add_corba_library_target(corba_techcheck
    ${IDL_DESTINATION_DIR}/TechCheckSK.cc
    ${IDL_DESTINATION_DIR}/TechCheckDynSK.cc)
target_link_libraries(corba_techcheck PRIVATE corba_datatypes)

add_corba_library_target(corba_epp
    ${IDL_DESTINATION_DIR}/EPPSK.cc
    ${IDL_DESTINATION_DIR}/EPPDynSK.cc)
target_link_libraries(corba_epp PRIVATE corba_datatypes)

add_corba_library_target(corba_zonegenerator
    ${IDL_DESTINATION_DIR}/ZoneGeneratorSK.cc
    ${IDL_DESTINATION_DIR}/ZoneGeneratorDynSK.cc)
target_link_libraries(corba_zonegenerator PRIVATE corba_datatypes)

add_corba_library_target(corba_contactverification
    ${IDL_DESTINATION_DIR}/ContactVerificationSK.cc
    ${IDL_DESTINATION_DIR}/ContactVerificationDynSK.cc)

add_corba_library_target(corba_admincontactverification
    ${IDL_DESTINATION_DIR}/AdminContactVerificationSK.cc
    ${IDL_DESTINATION_DIR}/AdminContactVerificationDynSK.cc)
target_link_libraries(corba_admincontactverification PRIVATE
    corba_nullableisodatetime
    corba_nullabletypes)

add_corba_library_target(corba_administrativeblocking
    ${IDL_DESTINATION_DIR}/AdministrativeBlockingSK.cc
    ${IDL_DESTINATION_DIR}/AdministrativeBlockingDynSK.cc)
target_link_libraries(corba_administrativeblocking PRIVATE corba_nullableisodate)

add_corba_library_target(corba_domainbrowser
    ${IDL_DESTINATION_DIR}/DomainBrowserSK.cc
    ${IDL_DESTINATION_DIR}/DomainBrowserDynSK.cc)
target_link_libraries(corba_domainbrowser PRIVATE
    corba_nullableisodate
    corba_nullableisodatetime)

add_corba_library_target(corba_publicrequest
    ${IDL_DESTINATION_DIR}/PublicRequestSK.cc
    ${IDL_DESTINATION_DIR}/PublicRequestDynSK.cc)
target_link_libraries(corba_publicrequest PRIVATE
    corba_buffer
    corba_nullabletypes)

add_corba_library_target(corba_conversion_test
    ${IDL_DESTINATION_DIR}/corba_conversion_testSK.cc
    ${IDL_DESTINATION_DIR}/corba_conversion_testDynSK.cc)

add_corba_library_target(corba_notification
    ${IDL_DESTINATION_DIR}/NotificationSK.cc
    ${IDL_DESTINATION_DIR}/NotificationDynSK.cc)

add_corba_library_target(corba_automatickeysetmanagement
    ${IDL_DESTINATION_DIR}/AutomaticKeysetManagementSK.cc
    ${IDL_DESTINATION_DIR}/AutomaticKeysetManagementDynSK.cc)

add_corba_library_target(corba_recordstatement
    ${IDL_DESTINATION_DIR}/RecordStatementSK.cc
    ${IDL_DESTINATION_DIR}/RecordStatementDynSK.cc)
target_link_libraries(corba_recordstatement PRIVATE
    corba_buffer
    corba_isodatetime)

add_corba_library_target(corba_accounting
    ${IDL_DESTINATION_DIR}/AccountingSK.cc
    ${IDL_DESTINATION_DIR}/AccountingDynSK.cc)
target_link_libraries(corba_accounting PRIVATE
    corba_isodate
    corba_isodatetime
    corba_nullableisodate)

add_common_library_target(corba
    src/bin/corba/nameservice.cc
    src/bin/corba/mailer_manager.cc
    src/bin/corba/file_manager_client.cc
    src/bin/corba/logger_client_impl.cc
    src/bin/corba/epp_corba_client_impl.cc)
target_link_libraries(corba PRIVATE
    corba_mailer
    corba_filemanager
    corba_logger
    corba_epp
    Fred::library)

add_common_library_target(corba_mojeid_util
    src/bin/corba/mojeid/mojeid_corba_conversion.cc
    src/bin/corba/mojeid/service_name.cc)
target_link_libraries(corba_mojeid_util PRIVATE
    corba_isodate
    corba_nullableisodate
    corba_mojeid
    Fred::library)

add_common_library_target(util
    src/util/output_indenter.cc
    src/util/csv_parser.cc
    src/util/xmlgen.cc
    src/util/subprocess.cc)
target_link_libraries(util PRIVATE Fred::library)

add_common_library_target(types
    src/util/types/birthdate.cc)

add_common_library_target(db
    src/util/db/query/query_old.cc
    src/util/db/query/filter.cc
    src/util/db/query/compound_filter.cc
    src/util/db/query/simple_filter.cc)
target_link_libraries(db PRIVATE
    ${PostgreSQL_LIBRARIES}
    Fred::library)

add_common_library_target(timezones
    src/util/tz/local_timestamp.cc
    src/util/tz/utc.cc
    src/util/tz/europe/prague.cc)

add_common_library_target(hp
    src/util/hp/hp.cc
    src/util/hp/hpmail.cc
    src/util/hp/hpmailbatchstate.cc)

add_common_library_target(corbautil
    src/bin/corba/util/corba_conversions_buffer.cc
    src/bin/corba/util/corba_conversions_isodate.cc
    src/bin/corba/util/corba_conversions_isodatetime.cc
    src/bin/corba/util/corba_conversions_money.cc
    src/bin/corba/util/corba_conversions_nullable_types.cc
    src/bin/corba/util/corba_conversions_nullableisodate.cc
    src/bin/corba/util/corba_conversions_nullableisodatetime.cc
    src/bin/corba/util/corba_conversions_string.cc)
target_link_libraries(corbautil PRIVATE
    corba_buffer
    corba_nullabletypes
    corba_nullableisodate
    corba_nullableisodatetime
    Fred::library)

add_common_library_target(oldutils
    src/deprecated/util/dbsql.cc
    src/deprecated/util/log.cc
    src/deprecated/util/pqsql.cc
    src/deprecated/util/timeclock.cc
    src/deprecated/util/util.cc)
target_link_libraries(oldutils PRIVATE
    ${PostgreSQL_LIBRARIES}
    Fred::library)

add_common_library_target(model
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
target_link_libraries(model PRIVATE
    Fred::library)

add_common_library_target(fred-deprecated
    src/deprecated/libfred/registry.cc
    src/deprecated/libfred/common_impl.cc
    src/deprecated/libfred/object_impl.cc
    src/deprecated/libfred/common_impl_new.cc
    src/deprecated/libfred/registrable_object/domain.cc
    src/deprecated/libfred/registrable_object/contact.cc
    src/deprecated/libfred/registrable_object/nsset.cc
    src/deprecated/libfred/registrable_object/keyset.cc
    src/deprecated/libfred/registrar.cc
    src/deprecated/libfred/credit.cc
    src/deprecated/libfred/invoicing/exceptions.cc
    src/deprecated/libfred/invoicing/invoice.cc
    src/deprecated/libfred/zone.cc
    src/deprecated/libfred/blacklist.cc
    src/deprecated/libfred/documents.cc
    src/deprecated/libfred/info_buffer.cc
    src/deprecated/libfred/mail.cc
    src/deprecated/libfred/notify.cc
    src/deprecated/libfred/file.cc
    src/deprecated/libfred/filter.cc
    src/deprecated/libfred/object_states.cc
    src/deprecated/libfred/common_diff.cc
    src/deprecated/libfred/reminder.cc
    src/deprecated/libfred/poll/create_request_fee_info_messages.cc
    src/deprecated/libfred/registrar/registrar_impl.cc)
target_link_libraries(fred-deprecated PRIVATE
    corba_buffer
    corba_nullabletypes
    corba_nullableisodate
    corba_nullableisodatetime
    oldutils
    model
    Boost::serialization
    Fred::library)

add_common_library_target(authentic_registrar
    src/backend/epp/impl/registraracl/authentic_registrar.cc)
target_link_libraries(authentic_registrar PRIVATE Fred::library)

add_common_library_target(buffer
    src/backend/buffer.cc)

add_common_library_target(messages
    src/deprecated/libfred/messages/messages_impl.cc)
target_link_libraries(messages PRIVATE
    db
    Fred::library)

add_common_library_target(contact_verification
    src/backend/contact_verification/contact_verification_impl.cc
    src/backend/contact_verification/public_request_contact_verification_impl.cc
    src/deprecated/libfred/contact_verification/contact.cc
    src/deprecated/libfred/contact_verification/contact_verification_checkers.cc
    src/deprecated/libfred/contact_verification/contact_conditional_identification_impl.cc
    src/deprecated/libfred/contact_verification/contact_identification_impl.cc
    src/deprecated/libfred/contact_verification/contact_verification_state.cc
    src/deprecated/libfred/contact_verification/contact_verification_validators.cc
    src/deprecated/libfred/contact_verification/contact_verification_password.cc)
target_link_libraries(contact_verification PRIVATE
    Fred::library
    util)

add_common_library_target(public_request
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
    src/backend/public_request/util/make_object_type.cc
    src/backend/public_request/util/get_public_request_uuid.cc
    src/backend/public_request/util/send_joined_address_email.cc
    src/deprecated/libfred/public_request/public_request.cc
    src/deprecated/libfred/public_request/public_request_authinfo_impl.cc
    src/deprecated/libfred/public_request/public_request_block_impl.cc
    src/deprecated/libfred/public_request/public_request_impl.cc
    src/deprecated/libfred/public_request/public_request_personal_info_impl.cc
    src/bin/corba/admin/public_request_mojeid.cc)
target_link_libraries(public_request PRIVATE
    buffer
    corba_mailer
    messages
    contact_verification
    Fred::library
    LibHermes::library
    LibFiled::library)

add_common_library_target(banking
    src/deprecated/libfred/banking/bank_common.cc
    src/deprecated/libfred/banking/bank_manager.cc
    src/deprecated/libfred/banking/exceptions.cc
    src/deprecated/libfred/banking/get_bank_accounts.cc)
target_link_libraries(banking PRIVATE
    Fred::library
    ${XML2_LIBRARIES})
target_include_directories(banking PUBLIC ${XML2_INCLUDE_DIRS})

add_common_library_target(requests
    src/deprecated/libfred/requests/model_request.cc
    src/deprecated/libfred/requests/model_session.cc
    src/deprecated/libfred/requests/model_request_property_name.cc
    src/deprecated/libfred/requests/session.cc
    src/deprecated/libfred/requests/request_list.cc
    src/deprecated/libfred/requests/request_manager.cc
    src/deprecated/libfred/requests/request_cache.cc
    src/deprecated/libfred/requests/request_property_name_cache.cc
    src/deprecated/libfred/requests/session_cache.cc)
target_link_libraries(requests PRIVATE
    fred-deprecated
    Fred::library)

add_common_library_target(admin_contact_verification
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
target_link_libraries(admin_contact_verification PRIVATE
    Fred::library
    messages
    ${XML2_LIBRARIES}
    resolv)
target_include_directories(admin_contact_verification PUBLIC ${XML2_INCLUDE_DIRS})

add_common_library_target(admin_notification
    src/backend/admin/notification/notification.cc)
target_link_libraries(admin_notification PRIVATE
    Fred::library
    corba_notification)

add_common_library_target(admin_impl
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
target_link_libraries(admin_impl PRIVATE
    corba_admin
    Fred::library)

add_common_library_target(whois_impl
    src/bin/corba/whois/whois_impl.cc)
target_link_libraries(whois_impl PRIVATE
    admin_impl
    corba_whois
    Fred::library)

add_common_library_target(whois2_impl
    src/bin/corba/whois/whois2_impl.cc
    src/backend/whois/nameserver_exists.cc
    src/backend/whois/is_domain_delete_pending.cc
    src/backend/whois/domain_expiration_datetime.cc
    src/backend/whois/registrar_group.cc
    src/backend/whois/registrar_certification.cc
    src/backend/whois/zone_list.cc
    src/backend/whois/whois.cc)
target_link_libraries(whois2_impl PRIVATE
    corba_whois2
    Fred::library)

add_common_library_target(epp_keyset_dns_key
    src/backend/epp/keyset/dns_key.cc)
target_link_libraries(epp_keyset_dns_key PRIVATE Fred::library)

# TODO FIXME move messenger/fileman
add_common_library_target(handle_database_args
    src/util/cfg/config_handler.cc
    src/util/cfg/handle_database_args.cc
    src/util/cfg/handle_messenger_args.cc
    src/util/cfg/handle_fileman_args.cc
    src/util/cfg/validate_args.cc)
target_link_libraries(handle_database_args PRIVATE Fred::library)

add_common_library_target(handle_rifd_args
    src/util/cfg/handle_rifd_args.cc
    src/backend/epp/contact/config_data_filter.cc
    src/backend/epp/contact/impl/info_contact.cc
    src/backend/epp/contact/impl/cznic/specific.cc
    src/backend/epp/contact/impl/set_unused/config.cc)
target_link_libraries(handle_rifd_args PRIVATE
    handle_database_args
    Fred::library)

add_common_library_target(epp_impl
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
    src/backend/epp/localization.cc
    src/backend/epp/epp_response_failure_localized.cc
    src/backend/epp/epp_response_success_localized.cc
    src/backend/epp/epp_result_failure.cc
    src/backend/epp/epp_result_failure_localized.cc
    src/backend/epp/epp_result_success_localized.cc
    src/backend/epp/epp_extended_error.cc
    src/backend/epp/epp_extended_error_localized.cc
    src/backend/epp/password.cc
    src/backend/epp/session_data.cc
    src/backend/epp/impl/conditionally_enqueue_notification.cc
    src/backend/epp/contact/authinfo_contact.cc
    src/backend/epp/contact/authinfo_contact_localized.cc
    src/backend/epp/contact/create_contact_localized.cc
    src/backend/epp/contact/create_contact.cc
    src/backend/epp/contact/util.cc
    src/backend/epp/contact/contact_change.cc
    src/backend/epp/contact/contact_data.cc
    src/backend/epp/contact/impl/contact_data_share_policy_rules.cc
    src/backend/epp/contact/impl/contact_output.cc
    src/backend/epp/contact/impl/get_personal_id_union.cc
    src/backend/epp/contact/impl/cznic/create_contact_data_filter.cc
    src/backend/epp/contact/impl/cznic/update_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/create_contact_data_filter.cc
    src/backend/epp/contact/impl/set_unused/update_contact_data_filter.cc
    src/backend/epp/contact/impl/get_create_contact_data_filter.cc
    src/backend/epp/contact/impl/get_contact_data_share_policy_rules.cc
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
    src/backend/epp/public_request/create_authinfo_request_registry_email.cc)
target_link_libraries(epp_impl PRIVATE
    corba_epp
    corbautil
    contact_verification
    epp_keyset_dns_key
    handle_rifd_args
    buffer
    Fred::library
    LibHermes::library)

add_common_library_target(log_impl
    src/bin/corba/log/log_impl_wrap.cc)
target_link_libraries(log_impl PRIVATE
    admin_impl
    corba_logger
    Fred::library)

add_common_library_target(admin_create_expired_domain
    src/backend/admin/domain/create_expired_domain.cc)
target_link_libraries(admin_create_expired_domain PRIVATE Fred::library)

add_common_library_target(util_corba_conversion
    src/util/corba_conversion.cc)
target_link_libraries(util_corba_conversion PRIVATE ${CORBA_LIBRARIES})

add_common_library_target(optys_util
    src/util/optys/download_client.cc
    src/bin/cli/optys_get_undelivered.cc)
target_link_libraries(optys_util PRIVATE
    util
    Fred::library)

add_common_library_target(admin_registrar
    src/backend/admin/registrar/create_registrar.cc
    src/backend/admin/registrar/update_epp_auth.cc
    src/backend/admin/registrar/update_registrar.cc
    src/backend/admin/registrar/update_zone_access.cc)
target_link_libraries(admin_registrar PRIVATE Fred::library)

add_common_library_target(admin_contact_merge
    src/backend/admin/contact/merge_contact.cc
    src/backend/admin/contact/merge_contact_auto_procedure.cc
    src/backend/admin/contact/merge_contact_logger.cc
    src/backend/admin/contact/merge_contact_reporting.cc)
target_link_libraries(admin_contact_merge PRIVATE Fred::library)

add_common_library_target(record_statement_impl
    src/backend/record_statement/record_statement.cc
    src/backend/record_statement/exceptions.cc
    src/backend/record_statement/impl/record_statement_xml.cc
    src/backend/record_statement/impl/factory.cc
    src/backend/record_statement/impl/utc_impl.cc
    src/backend/record_statement/impl/europe_prague_impl.cc
    src/backend/record_statement/impl/util.cc)
target_link_libraries(record_statement_impl PRIVATE
    buffer
    Fred::library)

add_common_library_target(domain_browser_impl
    src/backend/domain_browser/domain_browser.cc)
target_link_libraries(domain_browser_impl PRIVATE Fred::library)

add_common_library_target(accounting_impl
    src/backend/accounting/accounting.cc
    src/backend/accounting/exceptions.cc
    src/backend/accounting/impl/accounting.cc
    src/backend/accounting/impl/exceptions.cc
    src/backend/credit.cc)
target_link_libraries(accounting_impl PRIVATE Fred::library)

add_common_library_target(akm_impl
    src/backend/automatic_keyset_management/automatic_keyset_management.cc
    src/backend/automatic_keyset_management/dns_key.cc
    src/backend/automatic_keyset_management/impl/logger_request_data.cc
    src/backend/automatic_keyset_management/impl/logger_request_object_type.cc
    src/backend/automatic_keyset_management/impl/logger_request_property.cc
    src/backend/automatic_keyset_management/impl/logger_request_type.cc
    src/backend/automatic_keyset_management/impl/util.cc)
target_link_libraries(akm_impl PRIVATE Fred::library)

add_common_library_target(admin_zone_impl
    src/backend/admin/zone/zone.cc)
target_link_libraries(admin_zone_impl PRIVATE Fred::library)

add_common_library_target(administrativeblocking_impl
    src/backend/admin_block/administrativeblocking.cc)
target_link_libraries(administrativeblocking_impl PRIVATE Fred::library)

add_common_library_target(test-setup-fixtures
    test/setup/fixtures.cc
    test/setup/fixtures_utils.cc
    test/setup/tests_common.cc)
target_link_libraries(test-setup-fixtures PUBLIC
    Fred::library
    Boost::program_options
    Boost::unit_test_framework)

add_common_library_target(mojeid-checkers
    src/backend/mojeid/checkers.cc)
target_link_libraries(mojeid-checkers PUBLIC
    types
    Fred::library)

# fred-rifd
add_common_executable_target(fred-rifd
    src/bin/corba/fred-rifd.cc)
target_link_libraries(fred-rifd
    epp_impl
    util_corba_conversion
    authentic_registrar
    admin_contact_verification
    corba_techcheck
    corba_mailer
    corba
    public_request
    fred-deprecated
    messages
    model
    timezones
    db
    types
    handle_rifd_args
    oldutils
    util
    Fred::library
    Threads::Threads
    Boost::thread
    Boost::random
    Boost::date_time
    Boost::regex
    Boost::serialization
    Boost::program_options
    Boost::system)

# fred-adifd
add_common_executable_target(fred-adifd
    src/bin/corba/admin_contact_verification/server_i.cc
    src/deprecated/libfred/registrable_object/contact/util.cc
    src/bin/corba/fred-adifd.cc
    src/bin/corba/admin_block/server_i.cc
    src/bin/corba/notification/server_i.cc
    src/bin/corba/notification/notification_corba_conversion.cc)
target_link_libraries(fred-adifd
    admin_create_expired_domain
    admin_impl
    admin_notification
    admin_registrar
    admin_contact_verification
    administrativeblocking_impl
    authentic_registrar
    banking
    contact_verification
    corba
    corba_admincontactverification
    corba_administrativeblocking
    corba_isodatetime
    corba_contactverification
    corba_datatypes
    corba_epp
    corba_filters
    corba_logger
    corba_notification
    corba_nullabletypes
    corba_publicrequest
    corba_registry
    corbautil
    corba_whois
    corba_whois2
    corba_zone
    corba_admin
    db
    timezones
    corba_filemanager
    handle_database_args
    fred-deprecated
    corba_mailer
    messages
    model
    oldutils
    public_request
    requests
    types
    util
    util_corba_conversion
    whois2_impl
    whois_impl
    Fred::library
    Threads::Threads
    Boost::random
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::thread
    Boost::program_options
    Boost::serialization)

# fred-pifd
add_common_executable_target(fred-pifd
    src/bin/corba/contact_verification/contact_verification_i.cc
    src/bin/corba/public_request/server_i.cc
    src/bin/corba/fred-pifd.cc)
target_link_libraries(fred-pifd
    admin_impl
    whois_impl
    whois2_impl
    corba_whois
    corba_whois2
    corba_registry
    corba_filters
    corba_datatypes
    corba
    corba_epp
    corba_mailer
    corba_filemanager
    corba_logger
    corbautil
    timezones
    corba_nullabletypes
    corba_isodate
    corba_isodatetime
    corba_nullableisodate
    corba_nullableisodatetime
    contact_verification
    corba_contactverification
    corba_buffer
    corba_publicrequest
    contact_verification
    requests
    banking
    messages
    public_request
    handle_database_args
    fred-deprecated
    authentic_registrar
    model
    db
    types
    oldutils
    util
    util_corba_conversion
    Fred::library
    Threads::Threads
    Boost::random
    Boost::thread
    Boost::regex
    Boost::date_time
    Boost::serialization
    Boost::program_options
    Boost::system)

# fred-logd
add_common_executable_target(fred-logd
    src/bin/corba/fred-logd.cc)
target_link_libraries(fred-logd
    admin_impl
    log_impl
    corba_logger
    corba_registry
    corba_filters
    corba
    handle_database_args
    fred-deprecated
    requests
    model
    db
    types
    util
    oldutils
    Fred::library
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::serialization
    Boost::thread
    Boost::program_options)

# fred-msgd
add_common_executable_target(fred-msgd
    src/bin/corba/messages/fred-msgd.cc
    src/bin/corba/messages/messages_filemanager.cc
    src/bin/corba/messages/messages.cc)
target_link_libraries(fred-msgd
    handle_database_args
    messages
    model
    types
    db
    oldutils
    util
    corba
    corba_datatypes
    corba_messages
    corba_filemanager
    Fred::library
    Boost::thread
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::program_options)

# fred-mifd
add_common_executable_target(fred-mifd
    src/backend/mojeid/mojeid.cc
    src/backend/mojeid/mojeid_impl_data_conversion.cc
    src/backend/mojeid/mojeid_impl_internal.cc
    src/backend/mojeid/mojeid_public_request.cc
    src/backend/mojeid/messages/generate.cc
    src/bin/corba/mojeid/server_i.cc
    src/bin/corba/mojeid/fred-mifd.cc)
target_link_libraries(fred-mifd
    corba_datatypes
    handle_database_args
    fred-deprecated
    messages
    model
    corba_buffer
    corba_isodate
    corba_isodatetime
    corba_nullableisodate
    corba_nullableisodatetime
    corba_mojeid
    corba_mojeid_util
    corba_mailer
    corbautil
    contact_verification
    public_request
    timezones
    util
    util_corba_conversion
    db
    corba
    mojeid-checkers
    Threads::Threads
    Boost::random
    Boost::system
    Boost::regex
    Boost::thread
    Boost::date_time
    Boost::program_options)

# fred-mojeid-msggen
add_common_executable_target(fred-mojeid-msggen
    src/bin/corba/mojeid/messages/client.cc)
target_link_libraries(fred-mojeid-msggen
    corba_buffer
    corba_nullableisodate
    corba_isodatetime
    corba_mojeid
    corba_mojeid_util
    handle_database_args
    Fred::library
    Boost::program_options
    Boost::date_time)

# fred-dbifd
add_common_executable_target(fred-dbifd
    src/bin/corba/domain_browser/domain_browser_i.cc
    src/bin/corba/domain_browser/fred-dbifd.cc)
target_link_libraries(fred-dbifd
    corba_domainbrowser
    domain_browser_impl
    handle_database_args
    fred-deprecated
    corbautil
    corba
    timezones
    util
    db
    Fred::library
    Threads::Threads
    Boost::system
    Boost::random
    Boost::regex
    Boost::thread
    Boost::date_time
    Boost::program_options)

# fred-akmd
add_common_executable_target(fred-akmd
    src/util/dummy_logger.cc
    src/bin/corba/automatic_keyset_management/corba_conversions.cc
    src/bin/corba/automatic_keyset_management/server_i.cc
    src/bin/corba/automatic_keyset_management/fred-akmd.cc)
target_link_libraries(fred-akmd
    akm_impl
    corba_automatickeysetmanagement
    epp_keyset_dns_key
    handle_database_args
    fred-deprecated
    corbautil
    timezones
    corba
    corba_logger
    corba_datatypes
    corba_admin
    corba_registry
    corba_filters
    util
    db
    Fred::library
    Boost::system
    Boost::random
    Boost::thread
    Boost::regex
    Boost::date_time
    Boost::program_options)

# fred-rsifd
add_common_executable_target(fred-rsifd
    src/bin/corba/record_statement/record_statement_i.cc
    src/bin/corba/record_statement/fred-rsifd.cc)
target_link_libraries(fred-rsifd
    handle_database_args
    fred-deprecated
    buffer
    corbautil
    record_statement_impl
    util
    db
    timezones
    corba
    corba_mailer
    corba_datatypes
    corba_recordstatement
    corba_buffer
    corba_isodate
    corba_isodatetime
    corba_nullableisodate
    Fred::library
    Threads::Threads
    Boost::date_time
    Boost::program_options
    Boost::thread
    Boost::system
    Boost::regex)

# fred-accifd
add_common_executable_target(fred-accifd
    src/bin/corba/accounting/accounting_i.cc
    src/bin/corba/accounting/fred-accifd.cc
    src/bin/corba/accounting/impl/corba_conversions.cc
    src/bin/corba/accounting/impl/exceptions.cc)
target_link_libraries(fred-accifd
    accounting_impl
    banking
    corba
    corba_accounting
    corbautil
    timezones
    handle_database_args
    fred-deprecated
    model
    db
    oldutils
    types
    util
    Fred::library
    Threads::Threads
    Boost::random
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::thread
    Boost::program_options)

# fred-admin
add_common_executable_target(fred-admin
    src/bin/cli/bankclient.cc
    src/bin/cli/chargeclient.cc
    src/bin/cli/charge_registry_access_fee.cc
    src/bin/cli/commonclient.cc
    src/bin/cli/contactclient.cc
    src/bin/cli/domain_lifecycle_params.cc
    src/bin/cli/domainclient.cc
    src/bin/cli/enumparamclient.cc
    src/bin/cli/fileclient.cc
    src/bin/cli/fred-admin.cc
    src/bin/cli/handle_admin_client_domain_lifecycle_parameters_args_grp.cc
    src/bin/cli/manage_domain_lifecycle_parameters.cc
    src/bin/cli/invoiceclient.cc
    src/bin/cli/keysetclient.cc
    src/bin/cli/notifyclient.cc
    src/bin/cli/objectclient.cc
    src/bin/cli/pollclient.cc
    src/bin/cli/public_request_method.cc
    src/bin/cli/registrarclient.cc
    src/bin/cli/remove_delete_candidates.cc
    src/util/optys/upload_client.cc)
target_link_libraries(fred-admin
    admin_contact_merge
    admin_contact_verification
    admin_create_expired_domain
    admin_notification
    admin_registrar
    admin_zone_impl
    handle_database_args
    fred-deprecated
    authentic_registrar
    model
    banking
    corba
    corba_epp
    corba_logger
    corba_isodatetime
    corba_notification
    corba_admin
    corba_registry
    corba_filters
    corba_datatypes
    corba_mailer
    corba_filemanager
    messages
    requests
    types
    db
    oldutils
    util
    hp
    public_request
    optys_util
    Fred::library
    Threads::Threads
    ${LIBCURL_LIBRARIES}
    ${MINIZIP_LIBRARIES}
    ${LIBSSH_LIBRARIES}
    Boost::system
    Boost::filesystem
    Boost::regex
    Boost::program_options
    Boost::date_time
    Boost::random
    Boost::thread
    Boost::filesystem
    Boost::serialization)
target_include_directories(fred-admin PRIVATE ${LIBCURL_INCLUDE_DIRS})
target_include_directories(fred-admin PRIVATE ${MINIZIP_INCLUDE_DIRS})
target_include_directories(fred-admin PRIVATE ${LIBSSH_INCLUDE_DIRS})
target_compile_options(fred-admin PRIVATE ${LIBCURL_CXXFLAGS})
target_compile_options(fred-admin PRIVATE ${MINIZIP_CXXFLAGS})
target_compile_options(fred-admin PRIVATE ${LIBSSH_CXXFLAGS})

# migrate_log
add_common_executable_target(migrate_log
    tools/logd_migration/migrate.cc
    tools/logd_migration/log_if.cc
    tools/logd_migration/m_epp_parser.cc)
target_link_libraries(migrate_log
    requests
    fred-deprecated
    model
    banking
    log_impl
    types
    db
    util
    oldutils
    Fred::library
    Boost::system
    Boost::regex
    Boost::thread
    Boost::date_time)

# mojeid_contact_address_import
add_common_executable_target(mojeid_contact_address_import
    tools/contact_migration/mojeid_contact_address_import.cc)
target_link_libraries(mojeid_contact_address_import
    fred-deprecated
    db
    util
    Fred::library
    Threads::Threads
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::thread
    Boost::random)

# mojeid_contact_ssn_fix
add_common_executable_target(mojeid_contact_ssn_fix
    tools/contact_migration/mojeid_contact_ssn_fix.cc)
target_link_libraries(mojeid_contact_ssn_fix
    fred-deprecated
    db
    corbautil
    timezones
    corba_isodatetime
    corba_nullableisodate
    util
    Fred::library
    Threads::Threads
    Boost::system
    Boost::thread
    Boost::regex
    Boost::random
    Boost::date_time)

# fred-disclose-flags-updater
add_common_executable_target(fred-disclose-flags-updater
    tools/disclose_flags_updater/thread_safe_output.cc
    tools/disclose_flags_updater/worker.cc
    tools/disclose_flags_updater/disclose_value.cc
    tools/disclose_flags_updater/disclose_settings.cc
    tools/disclose_flags_updater/contact_search_query.cc
    tools/disclose_flags_updater/fred_disclose_flags_updater.cc)
target_link_libraries(fred-disclose-flags-updater
    fred-deprecated
    db
    util
    Fred::library
    Threads::Threads
    Boost::regex
    Boost::date_time
    Boost::thread
    Boost::program_options
    Boost::random
    Boost::system)

# crc32
add_common_executable_target(crc32
    src/util/hp/crc32.cc)

# test-mockdb
add_common_executable_target(test-mockdb
    src/util/db/test-mockdb.cc)
target_link_libraries(test-mockdb
    types
    Fred::library
    Boost::thread
    Boost::regex
    Boost::date_time
    Boost::system)

# test-model
add_common_executable_target(test-model
    src/deprecated/model/test-model.cc)
target_link_libraries(test-model
    model
    banking
    types
    db
    Fred::library
    Boost::system
    Boost::regex
    Boost::thread
    Boost::date_time)

# test-db
add_common_executable_target(test-db
    src/util/db/test-db.cc)
target_link_libraries(test-db
    types
    db
    Fred::library
    Boost::thread
    Boost::system
    Boost::regex
    Boost::date_time)

# test-admin-contact-automerge
add_common_executable_target(test-admin-contact-automerge
    test/backend/admin/contact/automerge/main.cc
    test/backend/admin/contact/automerge/test-merge-auto-proc.cc)
target_link_libraries(test-admin-contact-automerge
    admin_contact_merge
    corba_datatypes
    fred-deprecated
    model
    corba
    corba_registry
    corba_filters
    corba_mailer
    corba_datatypes
    corba_filemanager
    requests
    types
    db
    oldutils
    util
    timezones
    handle_rifd_args
    test-setup-fixtures
    fred-test-common-static::library
    Threads::Threads
    Boost::system
    Boost::thread
    Boost::regex
    Boost::date_time
    Boost::random)

# test-admin-contact-verification
add_common_executable_target(test-admin-contact-verification
    test/backend/admin/contact/verification/main.cc
    test/backend/admin/contact/verification/test_run_all_enqueued_checks.cc
    test/backend/admin/contact/verification/test_fill_check_queue.cc
    test/backend/admin/contact/verification/test_resolve_check.cc
    test/backend/admin/contact/verification/test_related_records.cc
    test/backend/admin/contact/verification/test_enqueue_check.cc
    test/backend/admin/contact/verification/test_delete_domains_of_invalid_contact.cc)
target_link_libraries(test-admin-contact-verification
    admin_contact_verification
    fred-deprecated
    db
    util
    timezones
    handle_rifd_args
    test-setup-fixtures
    fred-test-common-static::library
    Threads::Threads
    Boost::system
    Boost::unit_test_framework
    Boost::thread
    Boost::regex
    Boost::random
    Boost::program_options
    Boost::date_time)

# test-admin-notification
add_common_executable_target(test-admin-notification
    test/backend/admin/notification/main.cc
    test/backend/admin/notification/test_set_domain_outzone_unguarded_warning_emails.cc)
target_link_libraries(test-admin-notification
    admin_notification
    corba
    corba_notification
    handle_database_args
    fred-deprecated
    db
    util
    timezones
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::random
    Boost::regex
    Boost::thread
    Boost::date_time)

# test-automatic-keyset-management
add_common_executable_target(test-automatic-keyset-management
    test/backend/automatic_keyset_management/main.cc
    test/backend/automatic_keyset_management/test_automatic_keyset_management.cc)
target_link_libraries(test-automatic-keyset-management
    akm_impl
    corba_automatickeysetmanagement
    epp_keyset_dns_key
    handle_database_args
    fred-deprecated
    corbautil
    corba
    corba_logger
    corba_datatypes
    corba_admin
    corba_registry
    corba_filters
    util
    db
    timezones
    test-setup-fixtures
    Boost::system
    Boost::thread
    Boost::random
    Boost::regex
    Boost::date_time)

# test-corba-conversion
add_common_executable_target(test-corba-conversion
    test/util/test_corba_conversion_main.cc
    test/util/test_corba_conversion.cc)
target_link_libraries(test-corba-conversion
    buffer
    corba_conversion_test
    corba_buffer
    corba_isodate
    corba_isodatetime
    corba_nullableisodate
    corba_nullableisodatetime
    corbautil
    corba_mojeid
    corba_mojeid_util
    handle_database_args
    fred-deprecated
    util
    util_corba_conversion
    timezones
    test-setup-fixtures
    Boost::system
    Boost::random
    Boost::thread
    Boost::regex
    Boost::date_time)

# test-epp
add_common_executable_target(test-epp
    test/backend/epp/main.cc
    test/backend/epp/test_conditionally_enqueue_notification.cc
    test/backend/epp/contact/test_create_contact.cc
    test/backend/epp/contact/test_delete_contact.cc
    test/backend/epp/contact/test_info_contact.cc
    test/backend/epp/contact/test_update_contact.cc
    test/backend/epp/contact/test_check_contact.cc
    test/backend/epp/contact/test_transfer_contact.cc
    test/backend/epp/contact/test_contact_handle_state_to_check_result.cc
    test/backend/epp/contact/util.cc
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
#    test/backend/epp/poll/fixture.cc
    test/backend/epp/poll/test_poll_request.cc
    test/backend/epp/poll/test_poll_acknowledgement.cc
    test/backend/epp/poll/test_poll_request_contact_details.cc
    test/backend/epp/poll/test_poll_request_domain_details.cc
    test/backend/epp/poll/test_poll_request_keyset_details.cc
    test/backend/epp/poll/util.cc)
target_link_libraries(test-epp
    epp_impl
    fred-deprecated
    timezones
    authentic_registrar
    types
    db
    util
    handle_rifd_args
    corba_mailer
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::regex
    Boost::random
    Boost::thread
    Boost::date_time)

# test-exception
add_common_executable_target(test-exception
    test/util/test_exception.cc)
target_link_libraries(test-exception
    Fred::library
    Boost::system)

# test-admin-domain
add_common_executable_target(test-admin-domain
    test/backend/admin/domain/main.cc
    test/backend/admin/domain/test_create_expired_domain.cc)
target_link_libraries(test-admin-domain
    admin_create_expired_domain
    admin_notification
    corba
    handle_database_args
    fred-deprecated
    db
    util
    timezones
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::regex
    Boost::random
    Boost::thread
    Boost::date_time)

add_common_executable_target(test-admin-registrar
    test/backend/admin/registrar/main.cc
    test/backend/admin/registrar/test_create_registrar.cc
    test/backend/admin/registrar/test_update_epp_auth.cc
    test/backend/admin/registrar/test_update_registrar.cc
    test/backend/admin/registrar/test_update_zone_access.cc)
target_link_libraries(test-admin-registrar
    admin_registrar
    corba
    corba_filters
    corba_datatypes
    db
    util
    types
    timezones
    handle_database_args
    fred-deprecated
    test-setup-fixtures
    fred-test-common-static::library
    Threads::Threads
    Boost::system
    Boost::date_time
    Boost::regex
    Boost::random
    Boost::thread)

add_common_executable_target(test-admin-zone
    test/backend/admin/zone/main.cc
    test/backend/admin/zone/test_add_zone.cc
    test/backend/admin/zone/test_add_zone_ns.cc)
target_link_libraries(test-admin-zone
    admin_zone_impl
    corba
    corba_filters
    corba_datatypes
    handle_database_args
    fred-deprecated
    db
    util
    types
    timezones
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::date_time
    Boost::regex
    Boost::random
    Boost::thread)

add_common_executable_target(test-admin-block
    test/backend/admin_block/main.cc
    test/backend/admin_block/test_block_domains.cc
    test/backend/admin_block/test_restore_prev_state_of_domains.cc
    test/backend/admin_block/test_unblock_domains.cc
    test/backend/admin_block/util.cc)
target_link_libraries(test-admin-block
    administrativeblocking_impl
    corba
    corba_filters
    corba_datatypes
    handle_database_args
    fred-deprecated
    db
    util
    types
    timezones
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::date_time
    Boost::regex
    Boost::random
    Boost::thread)

# test-optys
add_common_executable_target(test-optys
    test/test_optys_main.cc
    test/util/test_optys.cc)
target_link_libraries(test-optys
    handle_database_args
    fred-deprecated
    optys_util
    util
    types
    timezones
    test-setup-fixtures
    Threads::Threads
    Boost::filesystem
    Boost::regex
    Boost::thread
    Boost::random
    Boost::date_time
    Boost::system)

# test-public-request
add_common_executable_target(test-public-request
    test/backend/public_request/main.cc
    test/backend/public_request/test_registry_email.cc
    test/backend/public_request/test_non_registry_email.cc
    test/backend/public_request/test_create_block_unblock_request.cc
    test/backend/public_request/test_create_public_request_pdf.cc
    test/backend/public_request/fixture_common.cc)
target_link_libraries(test-public-request
    public_request
    fred-deprecated
    corba
    corba_mailer
    corba_datatypes
    requests
    db
    util
    timezones
    handle_rifd_args
    test-setup-fixtures
    Threads::Threads
    Boost::regex
    Boost::thread
    Boost::date_time
    Boost::random
    Boost::system)

# test-record-statement
add_common_executable_target(test-record-statement
    test/backend/record_statement/main.cc
    test/backend/record_statement/test_record_statement.cc)
target_link_libraries(test-record-statement
    buffer
    corba_buffer
    corba_isodate
    corba_isodatetime
    handle_database_args
    fred-deprecated
    record_statement_impl
    util
    db
    timezones
    corba
    corba_mailer
    corba_datatypes
    test-setup-fixtures
    Threads::Threads
    Boost::regex
    Boost::thread
    Boost::date_time
    Boost::random
    Boost::system)

# test-admin-contact-automerge
add_common_executable_target(test-accounting
    test/backend/accounting/main.cc
    test/backend/accounting/test_get_registrar_by_handle.cc
    test/backend/accounting/test_get_registrar_by_payment.cc
    test/backend/accounting/test_get_registrar_references.cc
    test/backend/accounting/test_get_zone_by_payment.cc)
target_link_libraries(test-accounting
    accounting_impl
    banking
    corba
    corba_accounting
    corbautil
    timezones
    handle_database_args
    fred-deprecated
    model
    db
    oldutils
    types
    util
    test-setup-fixtures
    Threads::Threads
    Boost::random
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::thread)

# test-server
add_common_executable_target(test-server
    test/deprecated/test_server.cc
    test/deprecated/test_conv.cc
    test/deprecated/test_file_manager_client.cc
    test/deprecated/test_db.cc
    test/deprecated/test_registrar_group.cc
    test/deprecated/test_registrar_blocking.cc
    test/deprecated/test_logd.cc
    test/deprecated/test_msgd.cc
    test/deprecated/test_admin.cc
    test/deprecated/test_corbans.cc
    test/deprecated/test_whois.cc
    test/deprecated/test_cache.cc
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
    test/util/test_printable.cc
    test/util/test_csv_parser.cc
    test/util/test_is_equal_optional_nullable.cc
    test/backend/domain_browser/test_domain_browser.cc)
target_link_libraries(test-server
    admin_contact_merge
    admin_impl
    contact_verification
    domain_browser_impl
    epp_impl
    corba
    whois_impl
    public_request
    admin_contact_verification
    admin_notification
    admin_create_expired_domain
    fred-deprecated
    authentic_registrar
    messages
    model
    optys_util
    timezones
    db
    types
    oldutils
    requests
    banking
    util
    util_corba_conversion
    handle_rifd_args
    corba_messages
    corba_zone
    corba_filemanager
    corba_mailer
    corba_zonegenerator
    corba_administrativeblocking
    corba_epp
    corba_techcheck
    corba_logger
    corba_admin
    corba_whois
    corba_registry
    corba_filters
    corba_datatypes
    test-setup-fixtures
    Threads::Threads
    Boost::system
    Boost::serialization
    Boost::regex
    Boost::thread
    Boost::filesystem
    Boost::date_time
    Boost::random)

# test-whois
add_common_executable_target(test-whois
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
    test/backend/whois/test_whois_get_object_status_descriptions.cc)
target_link_libraries(test-whois
    corba_datatypes
    whois2_impl
    fred-deprecated
    model
    corba
    corba_registry
    corba_filters
    corba_mailer
    corba_nullableisodate
    corba_nullableisodatetime
    corba_datatypes
    corba_filemanager
    requests
    types
    db
    oldutils
    util
    timezones
    handle_rifd_args
    test-setup-fixtures
    Threads::Threads
    Boost::regex
    Boost::thread
    Boost::date_time
    Boost::random
    Boost::system)

add_common_executable_target(test-delete-objects-marked-as-delete-candidate
    test/test_delete_objects_marked_as_delete_candidate.cc)
target_link_libraries(test-delete-objects-marked-as-delete-candidate
    handle_database_args
    fred-deprecated
    db
    oldutils
    util
    Fred::library
    Threads::Threads
    Boost::program_options
    Boost::date_time
    Boost::regex
    Boost::random
    Boost::filesystem
    Boost::thread
    Boost::system)

# test mojeid
add_common_executable_target(test-mojeid
    test/backend/mojeid/main.cc
    test/backend/mojeid/test_mojeid_checkers.cc)
target_link_libraries(test-mojeid
    handle_database_args
    db
    fred-deprecated
    util
    test-setup-fixtures
    mojeid-checkers
    fred-test-common-static::library
    Boost::random
    Boost::system
    Boost::regex
    Boost::date_time
    Boost::unit_test_framework
    Boost::program_options)


install(TARGETS fred-accifd DESTINATION ${SBINDIR})
install(TARGETS fred-adifd DESTINATION ${SBINDIR})
install(TARGETS fred-admin DESTINATION ${SBINDIR})
install(TARGETS fred-akmd DESTINATION ${SBINDIR})
install(TARGETS fred-dbifd DESTINATION ${SBINDIR})
install(TARGETS fred-logd DESTINATION ${SBINDIR})
install(TARGETS fred-mifd DESTINATION ${SBINDIR})
install(TARGETS fred-mojeid-msggen DESTINATION ${SBINDIR})
install(TARGETS fred-msgd DESTINATION ${SBINDIR})
install(TARGETS fred-pifd DESTINATION ${SBINDIR})
install(TARGETS fred-rifd DESTINATION ${SBINDIR})
install(TARGETS fred-rsifd DESTINATION ${SBINDIR})
install(TARGETS migrate_log DESTINATION ${SBINDIR})
install(TARGETS mojeid_contact_address_import DESTINATION ${SBINDIR})
install(TARGETS mojeid_contact_ssn_fix DESTINATION ${SBINDIR})
install(TARGETS fred-disclose-flags-updater DESTINATION ${SBINDIR})

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config/ DESTINATION etc/fred FILES_MATCHING PATTERN "*.conf")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/init-scripts/ DESTINATION etc/init.d FILES_MATCHING PATTERN "fred-server"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/deprecated/ DESTINATION ${DATAROOTDIR}/doc/fred-server FILES_MATCHING PATTERN "example_payments.xml")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/deprecated/ DESTINATION ${DATAROOTDIR}/doc/fred-server FILES_MATCHING PATTERN "test_file.pdf")

if(NOT DO_NOT_INSTALL_TESTS)
    install(TARGETS test-admin-contact-automerge DESTINATION ${BINDIR})
    install(TARGETS test-admin-contact-verification DESTINATION ${BINDIR})
    install(TARGETS test-admin-notification DESTINATION ${BINDIR})
    install(TARGETS test-automatic-keyset-management DESTINATION ${BINDIR})
    install(TARGETS test-corba-conversion DESTINATION ${BINDIR})
    install(TARGETS test-epp DESTINATION ${BINDIR})
    install(TARGETS test-exception DESTINATION ${BINDIR})
    install(TARGETS test-optys DESTINATION ${BINDIR})
    install(TARGETS test-public-request DESTINATION ${BINDIR})
    install(TARGETS test-record-statement DESTINATION ${BINDIR})
    install(TARGETS test-server DESTINATION ${BINDIR})
    install(TARGETS test-whois DESTINATION ${BINDIR})
    install(TARGETS test-admin-domain DESTINATION ${BINDIR})
    install(TARGETS test-admin-registrar DESTINATION ${BINDIR})
    install(TARGETS test-admin-zone DESTINATION ${BINDIR})
    install(TARGETS test-admin-block DESTINATION ${BINDIR})
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

# dist

include("cmake/package_dist.cmake")
package_dist(PACKAGE_NAME "${PACKAGE_NAME}"
             PACKAGE_TARNAME "${PACKAGE_TARNAME}")

list_package_dependencies_recursive("${PACKAGE_NAME}")
