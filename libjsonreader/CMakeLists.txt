
cmake_minimum_required(VERSION 3.14)

set(LIB_NAME libjsonreader)

if (WIN32)
    project(${LIB_NAME} LANGUAGES CXX)
elseif(UNIX)
    project(${LIB_NAME})
endif()

set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

find_package(Boost )
if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()



#set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})

find_package(
  QT NAMES Qt6 Qt5
  COMPONENTS Core 
  REQUIRED) 

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Core 
  REQUIRED) 

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

file(GLOB_RECURSE FOUND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${LIB_NAME} STATIC ${FOUND_SOURCES} ${HEADERS})

target_sources(${LIB_NAME} PRIVATE 
        ${FOUND_SOURCES}
)

target_include_directories(${LIB_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FORMS_DIR}
        ${INCLUDE_DIR}
        ${SOURCE_DIR}
)
# Add the target includes for MY_PROJECT 
# target_include_directories(${LIB_NAME} PRIVATE ${FORMS_DIR})
# target_include_directories(${LIB_NAME} PRIVATE ${INCLUDE_DIR})
# target_include_directories(${LIB_NAME} PRIVATE ${SOURCE_DIR})

message("QTDIR " $ENV{QTDIR})



#===================== LINKING LIBRARIES =======================#
target_link_libraries(${LIB_NAME} Qt${QT_VERSION_MAJOR}::Core)

# find_package(libio CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} libio)

# find_package(libraw CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} libraw)


#===================== INSTALL TARGETS =======================#
install(TARGETS ${LIB_NAME} EXPORT libjsonreaderTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)

install(FILES ${HEADERS} DESTINATION include/json/)

install(EXPORT libjsonreaderTargets
        FILE libjsonreaderConfig.cmake
        NAMESPACE JSON::
        DESTINATION lib/cmake/libjsonreader
)


