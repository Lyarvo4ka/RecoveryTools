cmake_minimum_required(VERSION 3.5)

project("libraw" VERSION 0.0.1)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)


find_package(Boost )
if(Boost_FOUND)
 #       message("BOOST FOUND = " ${Boost_INCLUDE_DIRS})
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

set(HEADERS 
        include/raw/abstractraw.h
        include/raw/djvuraw.h
        include/raw/ext4_raw.h
        include/raw/factories.h
        include/raw/filestruct.h
        include/raw/gopro.h
        include/raw/oleraw.h
        include/raw/onec.h
        include/raw/prproj.h
        include/raw/quicktime.h
        include/raw/rawpln.h
        include/raw/rawriff.h
        include/raw/rawzip.h
        include/raw/standartraw.h
        include/raw/zoomh6raw.h
)

add_library(libraw STATIC ${SOURCES} ${HEADERS})

target_sources(libraw PRIVATE 
        src/factories.cpp 
)


target_include_directories(libraw
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(libio CONFIG REQUIRED)
target_link_libraries(libraw PUBLIC libio)

install(TARGETS libraw EXPORT librawTargets
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