
cmake_minimum_required(VERSION 3.5)

project("libio" VERSION 0.0.1)

configure_file(
        src/config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.h   
)

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


add_library(libio STATIC ${SOURCES} ${HEADERS})
add_library(IO::libio ALIAS libio)

target_include_directories(libio
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE FOUND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
target_sources(libio PRIVATE 
        ${FOUND_SOURCES}
)

message( "CMAKE_INSTALL_INCLUDEDIR = "${CMAKE_INSTALL_INCLUDEDIR} )
message( "CMAKE_INSTALL_LIBDIR = "${CMAKE_INSTALL_LIBDIR} )

install(TARGETS libio EXPORT libioTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(FILES ${HEADERS} DESTINATION include/io/)

install(EXPORT libioTargets
        FILE libioConfig.cmake
        NAMESPACE IO::
        
        DESTINATION lib/cmake/libio
)