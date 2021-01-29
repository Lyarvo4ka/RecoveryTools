
cmake_minimum_required(VERSION 3.0)

project("libio")



add_library(libio STATIC ${SOURCES} )

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
