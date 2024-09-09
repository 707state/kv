cmake_minimum_required(VERSION 3.10)

project(kv)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/deps/boost_1_85_0")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.85.0 REQUIRED COMPONENTS filesystem system thread fiber
                                              serialization)
include_directories(${Boost_INCLUDE_DIRS})
# 强制使用 Boost 静态库

set(Boost_USE_MULTITHREADED ON)
find_package(OpenSSL REQUIRED)

set(Boost_USE_STATIC_RUNTIME OFF) # 禁用静态运行时库，如果你不需要静态链接 C++ 标准库

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Boost_INCLUDE_DIRS})
add_compile_options(-w)
link_directories(${BOOST_LIBRARY_DIRS})
include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
