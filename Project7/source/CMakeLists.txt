cmake_minimum_required(VERSION 3.13)
project(project6)


set(SOURCES main.cpp includes/lodepng.cpp)
add_executable(project7 ${SOURCES})

target_link_libraries(project7 GL)
target_link_libraries(project7 glut)
target_link_libraries(project7 GLEW)