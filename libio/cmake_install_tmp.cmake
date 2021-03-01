
##############################################
# Installation instructions

set_target_properties(libio PROPERTIES EXPORT_NAME LibIO)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#Export the targets to a script
install(EXPORT libio-targets
    FILE
         FindLibIO.cmake
    NAMESPACE
        LibIO::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LibIOConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)


configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/LibIOConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LibIOConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindLibIO.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LibIOConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LibIOConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Exporting from the build tree
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindLibIO.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FindLibIO.cmake
    COPYONLY)

export(EXPORT libio-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/LibIOTargets.cmake
    NAMESPACE IO::)

#Register package in user's package registry
export(PACKAGE IO)

##############################################
## Add test
#enable_testing()
#add_subdirectory(test)