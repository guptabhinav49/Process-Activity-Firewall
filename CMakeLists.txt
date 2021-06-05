cmake_minimum_required(VERSION 3.20)

# project title and version
project(Process-Activity-Firewall VERSION 1.0)

# add library
add_library(
    intercept SHARED
    intercept.c
)

# the utility executables
add_executable(listener listener.c)

target_link_libraries(intercept PRIVATE -ldl)