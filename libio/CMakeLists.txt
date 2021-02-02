
cmake_minimum_required(VERSION 3.5)

project("libio")


set(HEADERS 
        libio/constants.h 
        libio/dataarray.h
)

add_library(libio STATIC ${SOURCES} ${HEADERS})

target_include_directories(libio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)



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







