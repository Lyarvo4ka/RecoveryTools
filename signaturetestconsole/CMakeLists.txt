cmake_minimum_required(VERSION 3.14)

set(APP_NAME SignatureTestConsole)

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


