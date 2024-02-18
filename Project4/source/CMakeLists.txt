cmake_minimum_required(VERSION 3.13)
project(project4)


set(SOURCES main.cpp includes/lodepng.cpp)
add_executable(project4 ${SOURCES})

target_link_libraries(project4 GL)
target_link_libraries(project4 glut)
target_link_libraries(project4 GLEW)