macro(set_package_version version full_version)
    string(REGEX MATCH "^([0-9]+.[0-9]+.[0-9]+)" ${version} ${full_version})
endmacro()

macro(package __NAME)
    # <options> <one_value_args> <multi_value_args>
    cmake_parse_arguments(_ "" "FULL_VERSION;BUGREPORT" "" ${ARGN})
    if(__UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION}: unknown argument(s): ${__UNPARSED_ARGUMENTS}")
    endif()

    # required
    set(PACKAGE_NAME "${__NAME}")
    set(PACKAGE_FULL_VERSION "${__FULL_VERSION}")

    # optional
    if(__BUGREPORT)
        set(PACKAGE_BUGREPORT "${__BUGREPORT}")
    endif()

    set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_FULL_VERSION}")
    set(PACKAGE_TARNAME "${PACKAGE_NAME}-${PACKAGE_FULL_VERSION}")
    set_package_version(PACKAGE_VERSION "${PACKAGE_FULL_VERSION}")

    string(TOUPPER "${PACKAGE_NAME}" _package_name_upper_case)
    set(${_package_name_upper_case}_VERSION ${PACKAGE_FULL_VERSION})
    get_directory_property(_has_parent "PARENT_DIRECTORY")
    if(_has_parent)
        set(${_package_name_upper_case}_VERSION ${${_package_name_upper_case}_VERSION} PARENT_SCOPE)
    endif()

    message(STATUS "${PACKAGE_NAME} version: ${PACKAGE_FULL_VERSION}")
endmacro()
