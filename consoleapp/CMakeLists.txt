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
        include_directories(${Boost_INCLUDE_DIRS}) 
endif()

set(HEADERS 
        include/xoranalyzer.h 
)
#add_executable(${APP_NAME} ${SOURCES} ${HEADERS}) 
add_executable(ext4_raw src/ext4_raw_main.cpp)



# target_include_directories(${APP_NAME}
#     PUBLIC 
#         $<INSTALL_INTERFACE:include>    
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
#         ${CMAKE_CURRENT_BINARY_DIR}
# )

# target_sources(${APP_NAME} PRIVATE
#                    src/main_xoranalyzer.cpp
#                    src/xoranalyzer.cpp  
#  )

#  #target_include_directories(
#  #       ${APP_NAME}
#  #       PRIVATE
#  #       ${CMAKE_CURRENT_SOURCE_DIR}/../lib
# #)
# find_package(libio CONFIG REQUIRED)
# target_link_libraries(${APP_NAME} IO::libio)

# find_package(libraw CONFIG REQUIRED)
# target_link_libraries(${APP_NAME} RAW::libraw)

##########################################################

target_include_directories(ext4_raw
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(ext4_raw PRIVATE
                   src/main_xoranalyzer.cpp
                   src/xoranalyzer.cpp  
 )

 #target_include_directories(
 #       ${APP_NAME}
 #       PRIVATE
 #       ${CMAKE_CURRENT_SOURCE_DIR}/../lib
#)
find_package(libio CONFIG REQUIRED)
target_link_libraries(ext4_raw IO::libio)

find_package(libraw CONFIG REQUIRED)
target_link_libraries(ext4_raw RAW::libraw)