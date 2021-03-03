
cmake_minimum_required(VERSION 3.5)

project("libio" VERSION 0.0.1)

configure_file(
        src/config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.h   
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

message("cmake source dir =" ${CMAKE_SOURCE_DIR})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)


find_package(Boost )
if(Boost_FOUND)
        message("BOOST FOUND = " ${Boost_INCLUDE_DIRS})
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()



set(HEADERS 
        include/io/constants.h 
        include/io/dataarray.h
        include/io/dbf.h
        include/io/diskdevice.h
        include/io/error.h
        include/io/file.h
        include/io/finder.h
        include/io/fs.h
        include/io/functions.h
        include/io/iodevice.h
        include/io/onec.h
        include/io/physicaldrive.h 
        include/io/sparsebundle.h
        include/io/uniqueid.h 
        include/io/utility.h
        include/io/xoranalyzer.h    
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

target_sources(libio PRIVATE 
        src/dataarray.cpp
        src/diskdevice.cpp
        src/error.cpp
        src/ioengine.cpp 
        src/file.cpp
        src/ioengine.cpp
        src/uniqueid.cpp
        src/utility.cpp
)

install(TARGETS libio EXPORT libioTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)
foreach ( file ${INCLUDE_FILES} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION include/${dir} )
    message(${dir})
endforeach()
install(FILES include/io/XorAnalyzer.h DESTINATION include/io/)

install(EXPORT libioTargets
        FILE libioConfig.cmake
        NAMESPACE IO::
        DESTINATION lib/cmake/libio
)