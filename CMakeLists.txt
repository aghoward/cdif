cmake_minimum_required(VERSION 3.12)

project(Cdif)

add_library(cdif INTERFACE)
target_include_directories(cdif INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(cdif INTERFACE Threads::Threads)
target_compile_features(cdif INTERFACE cxx_std_17)

add_subdirectory(tests)
