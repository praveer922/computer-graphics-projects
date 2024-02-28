cmake_minimum_required(VERSION 3.13)
project(project6)


set(SOURCES main.cpp includes/lodepng.cpp)
add_executable(project6 ${SOURCES})

target_link_libraries(project6 GL)
target_link_libraries(project6 glut)
target_link_libraries(project6 GLEW)