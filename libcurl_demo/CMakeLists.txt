cmake_minimum_required(VERSION 2.8)
project(RTSP_SERVER)
set(CMAKE_C_COMPILER g++)
add_definitions(-g -fpermissive -std=c++0x)
add_definitions(-DWITH_OPENSSL -DWITH_DOM -D__STDC_CONSTANT_MACROS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#ffmpeg
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/curl7.69.1/include)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/FFmpegPusher)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/StreamSource)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/curl7.69.1/lib)
set(CURL_LIBS curl)

aux_source_directory(./ SRC_MAIN)

add_executable(ftp_client ${SRC_MAIN}  )

target_link_libraries(ftp_client ${CURL_LIBS} ssl crypto m pthread)




