cmake_minimum_required(VERSION 3.15)
project(ASIO2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})



# SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=0" )

add_subdirectory(example)
# add_subdirectory(example1)