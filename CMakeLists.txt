# cmake_minimum_required(VERSION 3.24)
project(filehelp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
# include_directories(.)
# include_directories(src)
link_libraries(iconv)
add_executable(${PROJECT_NAME} src/filehelp.cpp src/handle.cpp logo.rc)