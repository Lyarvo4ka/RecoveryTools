cmake_minimum_required(VERSION 3.14)

set(APP_NAME RawRecoveryConsole)

set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

find_package(Boost )
if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

if (WIN32)
    project(${APP_NAME} LANGUAGES CXX)
elseif(UNIX)
    project(${APP_NAME})
endif()


#set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
find_package(Qt5Core REQUIRED)
#find_package(Qt5Widgets REQUIRED)

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories($ENV{QTDIR}/include)
include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

#  file(GLOB_RECURSE SOURCES
#     "${FORMS_DIR}/*.ui"
#     "${FORMS_DIR}/*.qrc"
#     "${INCLUDE_DIR}/*.h"
#     "${SOURCE_DIR}/*.cpp"
# )


#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS} )

target_sources(${APP_NAME}
                PRIVATE 
                ${SOURCES}
)

# Add the target includes for MY_PROJECT 
target_include_directories(${APP_NAME}
    # PUBLIC 
    #     $<INSTALL_INTERFACE:include>    
    #     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FORMS_DIR}
        ${INCLUDE_DIR}
        ${SOURCE_DIR}
)

#===================== LINKING LIBRARIES =======================#
target_link_libraries(${APP_NAME} Qt5::Core)

#find_package(libio CONFIG REQUIRED)
target_link_libraries(${APP_NAME} libio)

#find_package(libraw CONFIG REQUIRED)
target_link_libraries(${APP_NAME} libraw)

#find_package(libjsonreader CONFIG REQUIRED)
target_link_libraries(${APP_NAME} libjsonreader)


install(TARGETS ${APP_NAME}
#EXPORT ${PROJECT_NAME}Targets            # for downstream dependencies
ARCHIVE DESTINATION lib COMPONENT libs   # static lib
RUNTIME DESTINATION bin COMPONENT libs   # binaries
LIBRARY DESTINATION lib COMPONENT libs   # shared lib
FRAMEWORK DESTINATION bin COMPONENT libs # for mac
#PUBLIC_HEADER DESTINATION include/${PROJECT_NAME} COMPONENT devel   # headers for mac (note the different component -> different package)
#INCLUDES DESTINATION include     
)



# set(CPACK_PACKAGE_NAME "RawRecoveryConsole")

# if(APPLE)
#   set(CPACK_PACKAGE_VENDOR "Organisation") # PackageMaker doesn't like http://
# else()
#   set(CPACK_PACKAGE_VENDOR "http://example.com") # deb lintian insists on URL
# endif()

# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Example project")
# set(CPACK_PACKAGE_CONTACT "Person name <name@example.com>")
# set(CPACK_PACKAGE_VERSION ${VERSION})
# set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
# set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
# set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
# set(CPACK_RPM_PACKAGE_DEBUG 0)

# # Select package generator
# if(MSVC)
#   set(CPACK_GENERATOR "NSIS")
# endif(MSVC)

# if(APPLE)
#   set(CPACK_GENERATOR "PackageMaker")
#   set(CPACK_SET_DESTDIR ON)
# endif(APPLE)

# if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
#   find_program(RPM_EXE rpmbuild)
#   if(${RPM_EXE} MATCHES RPM_EXE-NOTFOUND)
#     set(CPACK_GENERATOR "TGZ;DEB")
#   else()
#     set(CPACK_GENERATOR "TGZ;DEB;RPM")
#   endif()
# endif(${CMAKE_SYSTEM_NAME} MATCHES Linux)

# # Components
# # See www.cmake.org/Wiki/CMake:Component_Install_With_CPack#Controlling_Differents_Ways_of_packaging_components
# # _____________________________________________________________________________
# set(CPACK_COMPONENT_INSTALL ON)
# set(CPACK_COMPONENTS_ALL libs devel doc)

# set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Libraries")
# #set(CPACK_COMPONENT_LIBS_DESCRIPTION "Runtime libraries.")
# set(CPACK_COMPONENT_LIBS_GROUP "Runtime")
# # express component dependencies this way, it will translate into package dependencies where applicable
# set(CPACK_COMPONENT_LIBS_DEPENDS doc)

# set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documents")
# set(CPACK_COMPONENT_DOCS_DESCRIPTION "User Documentation")
# set(CPACK_COMPONENT_DOCS_GROUP "Documentation")

# set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Development files")
# set(CPACK_COMPONENT_DEV_DESCRIPTION "Development header files and libraries, as well as cmake config files.")
# set(CPACK_COMPONENT_DEV_GROUP "Development")
# set(CPACK_COMPONENT_DEV_DEPENDS libs)

# # Debian specific configuration (minimum)
# # _____________________________________________________________________________

# # set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
# # SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libboost-test-dev")
# # SET(CPACK_DEBIAN_PACKAGE_CONFLICTS "Hello0-apps")

# # RPM specific configuration (minimum)
# # _____________________________________________________________________________
# # set(CPACK_RPM_PACKAGE_LICENSE "Proprietary")
# # set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
# # set(CPACK_RPM_PACKAGE_VERSION ${VERSION})
# # set(CPACK_RPM_COMPONENT_INSTALL ON)   # necessary even if CPACK_COMPONENT_INSTALL set to ON. A bug in my opinion.

# # OS X PackageMaker
# # _____________________________________________________________________________
# set(CPACK_OSX_PACKAGE_VERSION "10.5")

# include (InstallRequiredSystemLibraries)
# include (CPack)







set(CPACK_GENERATOR ZIP)
set(CPACK_SOURCE_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME "RawRecoveryConsole")
set(CPACK_PACKAGE_VENDOR "CMake.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CPack Component Installation Example")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CPack Component Example")
SET(CPACK_NSIS_MODIFY_PATH ON)


include (InstallRequiredSystemLibraries)
INCLUDE(CPack)


