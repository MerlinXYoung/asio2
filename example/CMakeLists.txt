cmake_minimum_required(VERSION 3.15)
project(ASIO2-EXAMPLE)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
file(GLOB_RECURSE SRCS "*.cpp")

foreach(SRC ${SRCS})
    string(REGEX REPLACE ".*/" "" var ${SRC})
    string(REGEX REPLACE "\.cpp$" "" tgt ${var})
    add_executable(${tgt} ${SRC})
    target_link_libraries(${tgt} OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
endforeach(SRC ${SRCS})
