cmake_minimum_required(VERSION 3.14)
project(ezpwd-vcpkg-consumer CXX)

# Consume the header-only Reed-Solomon library as installed by vcpkg.
find_package(ezpwd REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE ezpwd::ezpwd)
