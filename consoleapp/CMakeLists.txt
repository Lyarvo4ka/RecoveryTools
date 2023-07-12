cmake_minimum_required(VERSION 3.5)

project("ConsolesApp" VERSION 0.0.1)


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
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

add_executable(XorAnalyzer src/main_xoranalyzer.cpp include/xoranalyzer.h)

add_executable(ext4_raw 
                src/ext4_raw_main.cpp  
                include/consolerawparser.h
                src/consolerawparser.cpp)

add_executable(fix_dbf src/fix_dbf_main.cpp)

add_executable(FileOffsetsWriter src/fileoffetswrite_main.cpp include/fileoffsetwriter.h)
add_executable(ReplaceBads src/replaceBads_main.cpp)

add_executable(FleshCollector src/fleshcollector_main.cpp
                include/fleshcollector.h)

# target_include_directories(XorAnalyzer
#     PUBLIC 
#         $<INSTALL_INTERFACE:include>    
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
#         ${CMAKE_CURRENT_BINARY_DIR}
# )

# target_include_directories(ext4_raw
#     PUBLIC 
#         $<INSTALL_INTERFACE:include>    
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
#         ${CMAKE_CURRENT_BINARY_DIR}
# )

 #target_include_directories(
 #       ${APP_NAME}
 #       PRIVATE
 #       ${CMAKE_CURRENT_SOURCE_DIR}/../lib
#)

target_include_directories(
       ext4_raw
       PRIVATE
       ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)
#find_package(libio REQUIRED)
target_link_libraries(ext4_raw libio)
target_link_libraries(ReplaceBads libio)
target_link_libraries(XorAnalyzer libio)
target_link_libraries(fix_dbf libio)
target_link_libraries(fix_dbf libraw)
target_link_libraries(FleshCollector libio)
# target_link_libraries(FileOffsetsWriter IO::libio)

#find_package(libraw REQUIRED)
target_link_libraries(ext4_raw libraw)
# target_link_libraries(XorAnalyzer RAW::libraw)
# target_link_libraries(fix_dbf RAW::libraw)


