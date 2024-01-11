cmake_minimum_required(VERSION 3.13)
project(project1)


set(SOURCES main.cpp)
add_executable(project1 ${SOURCES})

target_link_libraries(project1 GL)
target_link_libraries(project1 glut)