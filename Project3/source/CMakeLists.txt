cmake_minimum_required(VERSION 3.13)
project(project3)


set(SOURCES main.cpp)
add_executable(project3 ${SOURCES})

target_link_libraries(project3 GL)
target_link_libraries(project3 glut)
target_link_libraries(project3 GLEW)