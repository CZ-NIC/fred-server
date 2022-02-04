function(package_dist)
    # <options> <one_value_args> <multi_value_args>
    cmake_parse_arguments(_ "" "PACKAGE_NAME;PACKAGE_PREFIX;PACKAGE_TARNAME" "" ${ARGN})
    if(__UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION}: unknown argument(s): ${__UNPARSED_ARGUMENTS}")
    endif()
    find_program(GIT_PROGRAM git)
    get_directory_property(HAS_PARENT "PARENT_DIRECTORY")
    if(NOT HAS_PARENT)

        include(GNUInstallDirs)

        if(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_PROGRAM)
            if(NOT TARGET dist)
                message(STATUS "${PACKAGE_NAME}: Creating target dist")
                add_custom_target(dist
                    COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${__PACKAGE_TARNAME}/ HEAD > ${CMAKE_BINARY_DIR}/.${__PACKAGE_TARNAME}.tar
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
                    string(TOUPPER "${PACKAGE_NAME}" package_name_upper_case)
                    foreach(dependency ${${package_name_upper_case}_DEPENDENCIES})
                        string(TOLOWER "${dependency}" dependency_lower_case)
                        message(STATUS "${PACKAGE_NAME}: Adding to target dist dependent target dist_${dependency_lower_case}")
                        add_custom_command(
                            TARGET dist
                            COMMAND [ ! -f ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar ] || (tar --concatenate --file=${CMAKE_BINARY_DIR}/.${__PACKAGE_TARNAME}.tar ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar && rm ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar)
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
                        add_dependencies(dist "dist_${dependency_lower_case}")
                    endforeach()
                    add_custom_command(
                        TARGET dist
                        COMMAND cat ${CMAKE_BINARY_DIR}/.${__PACKAGE_TARNAME}.tar | gzip > ${CMAKE_BINARY_DIR}/${__PACKAGE_TARNAME}.tar.gz && rm ${CMAKE_BINARY_DIR}/.${__PACKAGE_TARNAME}.tar
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
            endif()

            if(DEFINED ENV{DISTCHECK_CMAKE_FLAGS})
                set(distcheck_flags $ENV{DISTCHECK_CMAKE_FLAGS})
            endif()

            if(NOT TARGET distcheck)
                set(DISTCHECK_BUILD_DIR "_build")
                set(DISTCHECK_INSTALL_DIR "_inst")
                if(${CMAKE_GENERATOR} STREQUAL "Ninja")
                    set(DISTCHECK_BUILD_CMD "ninja")
                else()
                    set(DISTCHECK_BUILD_CMD "$(MAKE)")
                endif()
                add_custom_target(clean_before_distcheck
                    COMMAND rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${__PACKAGE_TARNAME}
                    COMMENT "cleaning before distcheck start")
                add_custom_target(distcheck_make_dirs
                    COMMAND mkdir ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR}
                    COMMENT "make distcheck build and install directories"
                    DEPENDS clean_before_distcheck)
                add_custom_target(distcheck_unpack_archive
                    COMMAND tar xzf ${__PACKAGE_TARNAME}.tar.gz
                    COMMENT "Checking the distribution archive..."
                    DEPENDS dist distcheck_make_dirs)
                add_custom_target(distcheck_configure
                    COMMAND ${CMAKE_COMMAND} -G${CMAKE_GENERATOR} ${distcheck_flags} -DCMAKE_INSTALL_PREFIX=../${DISTCHECK_INSTALL_DIR} ../${__PACKAGE_TARNAME}
                    COMMENT "configure distcheck"
                    DEPENDS distcheck_unpack_archive
                    WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                add_custom_target(distcheck_make
                    COMMAND ${DISTCHECK_BUILD_CMD}
                    COMMENT "make"
                    DEPENDS distcheck_configure
                    WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                if (TARGET install)
                    add_custom_target(distcheck_install
                        COMMAND ${DISTCHECK_BUILD_CMD} install
                        COMMENT "make install"
                        DEPENDS distcheck_make
                        WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                    add_custom_target(distcheck_uninstall
                        COMMAND ${DISTCHECK_BUILD_CMD} uninstall
                        COMMENT "make uninstall"
                        DEPENDS distcheck_install
                        WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                    add_custom_target(distcheck_clean
                        COMMAND ${DISTCHECK_BUILD_CMD} clean
                        COMMENT "make clean"
                        DEPENDS distcheck_uninstall
                        WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                else()
                    add_custom_target(distcheck_clean
                        COMMAND ${DISTCHECK_BUILD_CMD} clean
                        COMMENT "make clean"
                        DEPENDS distcheck_make
                        WORKING_DIRECTORY ${DISTCHECK_BUILD_DIR})
                endif()
                add_custom_target(distcheck
                    COMMAND rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${__PACKAGE_TARNAME}
                    COMMENT "PASS: '${__PACKAGE_TARNAME}.tar.gz' is ready for distribution."
                    DEPENDS distcheck_clean)
            endif()
        else()
            message(STATUS "no git -- not including dist and distcheck targets")
        endif()
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_PROGRAM)
        string(TOLOWER "${__PACKAGE_NAME}" package_name_lower_case)
        if(NOT TARGET dist_${package_name_lower_case})
            message(STATUS "${PACKAGE_NAME}: Creating target dist_${package_name_lower_case}")
            add_custom_target(dist_${package_name_lower_case}
                COMMAND ${GIT_PROGRAM} -C ${CMAKE_CURRENT_SOURCE_DIR} archive --format=tar --prefix=${__PACKAGE_PREFIX}/ HEAD > ${CMAKE_BINARY_DIR}/.${package_name_lower_case}.tar
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
            string(TOUPPER "${PACKAGE_NAME}" package_name_upper_case)
            foreach(dependency ${${package_name_upper_case}_DEPENDENCIES})
                string(TOLOWER "${dependency}" dependency_lower_case)
                #if(NOT TARGET dist_${dependency_lower_case}) # FIXME check if build in this directory
                    message(STATUS "${PACKAGE_NAME}: Adding to target dist_${package_name_lower_case} dependent target dist_${dependency_lower_case}")
                    add_custom_command(
                        TARGET dist_${package_name_lower_case}
                        COMMAND [ ! -f ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar ] || (tar --concatenate --file=${CMAKE_BINARY_DIR}/.${package_name_lower_case}.tar ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar && rm ${CMAKE_BINARY_DIR}/.${dependency_lower_case}.tar)
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
                #endif()
                add_dependencies("dist_${package_name_lower_case}" "dist_${dependency_lower_case}")
            endforeach()
        endif()
    endif()
endfunction()
