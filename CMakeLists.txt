cmake_minimum_required(VERSION 3.16.3)

project(TEST)

add_executable(${PROJECT_NAME} Example.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
