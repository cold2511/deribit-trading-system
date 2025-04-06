cmake_minimum_required(VERSION 3.10)
project(deribit_client)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

add_executable(main main.cpp)

target_include_directories(main PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${CURL_LIBRARIES})

