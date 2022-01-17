cmake_minimum_required(VERSION 3.14)

set(APP_NAME RawRecovery)

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


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})

find_package(
  QT NAMES Qt6 Qt5
  COMPONENTS Core 
  REQUIRED) 

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Core Widgets 
  REQUIRED) 

  set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")
  set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories($ENV{QTDIR}/include)
include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE HEADERS
    ${INCLUDE_DIR}/*.h
)

file(GLOB_RECURSE FORMS
    ${FORMS_DIR}/*.ui
    ${SOURCE_DIR}/*.qrc
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

file(GLOB_RECURSE SOURCES
    ${SOURCE_DIR}/*.cpp
)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS} ${FORMS} )

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FORMS_DIR}
        ${INCLUDE_DIR}
        ${SOURCE_DIR}
)

target_link_libraries(${APP_NAME} 
                        Qt${QT_VERSION_MAJOR}::Core 
                        Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${APP_NAME} libio)
target_link_libraries(${APP_NAME} libraw)
target_link_libraries(${APP_NAME} libjsonreader)


set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)