
cmake_minimum_required(VERSION 3.14)

set(LIB_NAME libjsonreader)



 if (WIN32)
    project(${LIB_NAME} LANGUAGES CXX)
elseif(UNIX)
    project(${LIB_NAME})
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

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

# set(HEADERS 
#         include/jsonreader.h
#         include/signaturereader.h
# )

 file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

add_library(${LIB_NAME} STATIC  ${SOURCES} )

# target_sources(${LIB_NAME} PRIVATE 
#         src/jsonreader.cpp
# )
message("!!!SOURSES " ${SOURCES})

# Add the target includes for MY_PROJECT 
target_include_directories(${LIB_NAME} PRIVATE ${FORMS_DIR})
target_include_directories(${LIB_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${LIB_NAME} PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#
#target_link_libraries(${LIB_NAME} Qt5::Widgets)

find_package(libio CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} IO::libio)

find_package(libraw CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} RAW::libraw)


