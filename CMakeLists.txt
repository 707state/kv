cmake_minimum_required(VERSION 3.10)

project(kv)

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.86.0 REQUIRED COMPONENTS filesystem system thread fiber
                                              serialization)
set(Boost_USE_STATIC_LIBS ON)
include_directories(${Boost_INCLUDE_DIRS})
# 强制使用 Boost 静态库

set(Boost_USE_MULTITHREADED ON)

find_package(OpenSSL REQUIRED)

set(Boost_USE_STATIC_RUNTIME OFF) # 禁用静态运行时库，如果你不需要静态链接 C++ 标准库

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Boost_INCLUDE_DIRS})

link_directories(${BOOST_LIBRARY_DIRS})

include_directories(include)

add_subdirectory(src)

add_subdirectory(test)

add_compile_options(-w -fcoroutines)
# 设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置共享库的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置静态库的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
