cmake_minimum_required(VERSION 3.5)

project("XorAnalyzer" VERSION 0.0.1)

set(APP_NAME XorAnalyzer)

#configure_file(
#        src/config.h.in
#        ${CMAKE_CURRENT_BINARY_DIR}/config.h   
#)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)


find_package(Boost )
if(Boost_FOUND)
        message("BOOST FOUND = " ${Boost_INCLUDE_DIRS})
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

add_executable(${APP_NAME} ${SOURCES})

target_include_directories(${APP_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(${APP_NAME} PRIVATE
                   src/xoranalyzer.cpp 
 )

 target_include_directories(
        ${APP_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

target_link_libraries(${APP_NAME} libio)