cmake_minimum_required(VERSION 3.13)
project(project6)


set(SOURCES main.cpp includes/lodepng.cpp)
add_executable(project8 ${SOURCES})

target_link_libraries(project8 GL)
target_link_libraries(project8 glut)
target_link_libraries(project8 GLEW)