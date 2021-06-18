cmake_minimum_required(VERSION 3.5)

set(LIB_NAME libraw)
project(${LIB_NAME} VERSION 0.0.1)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)


find_package(Boost )
if(Boost_FOUND)
 #       message("BOOST FOUND = " ${Boost_INCLUDE_DIRS})
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

file(GLOB_RECURSE HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

add_library(${LIB_NAME} STATIC ${SOURCES} ${HEADERS})

file(GLOB_RECURSE FOUND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
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
)

find_package(libio REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC libio)

install(TARGETS ${LIB_NAME} EXPORT librawTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)

install(FILES ${HEADERS} DESTINATION include/raw/)

install(EXPORT librawTargets
        FILE librawConfig.cmake
        NAMESPACE RAW::
        DESTINATION lib/cmake/libraw
)