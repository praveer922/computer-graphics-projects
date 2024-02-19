cmake_minimum_required(VERSION 3.13)
project(project5)


set(SOURCES main.cpp includes/lodepng.cpp)
add_executable(project5 ${SOURCES})

target_link_libraries(project5 GL)
target_link_libraries(project5 glut)
target_link_libraries(project5 GLEW)