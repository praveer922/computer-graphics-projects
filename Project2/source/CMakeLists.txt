cmake_minimum_required(VERSION 3.13)
project(project2)


set(SOURCES main.cpp)
add_executable(project2 ${SOURCES})

target_link_libraries(project2 GL)
target_link_libraries(project2 glut)
target_link_libraries(project2 GLEW)