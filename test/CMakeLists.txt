set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/deps/boost_1_85_0")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.85.0 REQUIRED COMPONENTS filesystem system thread fiber)
include_directories(${Boost_INCLUDE_DIRS})
# 强制使用 Boost 静态库

set(Boost_USE_MULTITHREADED ON)
find_package(OpenSSL REQUIRED)

set(Boost_USE_STATIC_RUNTIME OFF) # 禁用静态运行时库，如果你不需要静态链接 C++ 标准库

include_directories(${Boost_INCLUDE_DIRS})

link_directories(${BOOST_LIBRARY_DIRS})

find_package(absl REQUIRED)
add_subdirectory(fiber_test)
add_subdirectory(rpc_test)
